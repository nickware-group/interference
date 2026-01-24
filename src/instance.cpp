/////////////////////////////////////////////////////////////////////////////
// Name:        instance.cpp
// Purpose:     Compute Instance Manager class
// Author:      Nickolay Babich
// Created:     21.10.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/instance.h>
#include <indk/types.h>
#include <indk/neuron.h>
#include <indk/math.h>

void indk::ComputeInstanceManager::doCreateInstance(int backend) {
    auto instance = new InstanceData();
    instance -> backend = indk::System::getComputeBackend(backend);
    instance -> model_data = nullptr;
    instance -> status = InstanceReady;

    Instances.push_back(instance);
}

void indk::ComputeInstanceManager::doCreateInstances(int count, int backend) {
    for (int i = 0; i < count; i++) doCreateInstance(backend);
}

void indk::ComputeInstanceManager::doTranslateToInstance(const std::vector<indk::Neuron*> &neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync, const std::string& prepareid, int iid) {
    if (iid >= Instances.size()) {
        if (!iid) doCreateInstance();
        else throw indk::Error(indk::Error::EX_INSTANCE_OUT_OF_RANGE, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (neurons.empty()) {
        throw indk::Error(indk::Error::EX_INSTANCE_TRANSLATION_NO_NEURONS, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    auto instance = Instances[iid];

    if (prepareid == instance->hash) return;

    doClearInstance(iid);
    instance -> hash = prepareid;
    instance -> model_data = instance -> backend -> doTranslate(neurons, outputs, sync);
}

void indk::ComputeInstanceManager::doRunInstance(const std::vector<indk::Neuron*> &neurons, const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, int iid) {
    if (iid >= Instances.size()) {
        throw indk::Error(indk::Error::EX_INSTANCE_OUT_OF_RANGE, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (!Instances[iid]->model_data) {
        throw indk::Error(indk::Error::EX_INSTANCE_MODEL_DATA_ERROR, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (x.size() != inputs.size() || x.empty()) {
        throw indk::Error(indk::Error::EX_INSTANCE_RUN_INPUT_ERROR, std::string(__FUNCTION__)+", instance "+std::to_string(iid)+", total inputs "+std::to_string(inputs.size())+", total signals "+std::to_string(x.size()));
    }

    Instances[iid] -> status = InstanceBusy;
    Instances[iid] -> backend -> doCompute(neurons, x, inputs, Instances[iid]->model_data);
    Instances[iid] -> status = InstanceReady;
}

void indk::ComputeInstanceManager::doResetInstance(const std::vector<std::string> &neurons, int iid) {
    if (iid >= Instances.size()) {
        return;
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->model_data) Instances[iid] -> backend -> doReset(neurons, Instances[iid]->model_data);
}

void indk::ComputeInstanceManager::doClearInstance(int iid) {
    if (iid >= Instances.size()) {
        return;
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->model_data) Instances[iid] -> backend -> doClear(Instances[iid]->model_data);
    Instances[iid] -> model_data = nullptr;
    Instances[iid] -> hash = "";
}

void indk::ComputeInstanceManager::doClearInstances() {
    for (int i = 0; i < Instances.size(); i++) {
        doClearInstance(i);
    }
}

void indk::ComputeInstanceManager::setMode(bool learning, int iid) {
    if (iid >= Instances.size()) {
        throw indk::Error(indk::Error::EX_INSTANCE_OUT_OF_RANGE, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (!Instances[iid]->model_data) {
        throw indk::Error(indk::Error::EX_INSTANCE_MODEL_DATA_ERROR, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->model_data) Instances[iid]->backend->setMode(Instances[iid]->model_data, learning);
}

std::vector<indk::OutputValue> indk::ComputeInstanceManager::getOutputValues(const std::vector<std::string> &neurons, int iid) {
    if (iid >= Instances.size()) {
        throw indk::Error(indk::Error::EX_INSTANCE_OUT_OF_RANGE, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (!Instances[iid]->model_data) {
        throw indk::Error(indk::Error::EX_INSTANCE_MODEL_DATA_ERROR, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    return Instances[iid]->backend->getOutputValues(neurons, Instances[iid]->model_data);
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeInstanceManager::getReceptorPositions(const std::vector<std::string> &neurons, int iid) {
    if (iid >= Instances.size()) {
        throw indk::Error(indk::Error::EX_INSTANCE_OUT_OF_RANGE, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (Instances[iid]->status != InstanceReady) {
        throw indk::Error(indk::Error::EX_INSTANCE_BUSY, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    if (!Instances[iid]->model_data) {
        throw indk::Error(indk::Error::EX_INSTANCE_MODEL_DATA_ERROR, std::string(__FUNCTION__)+", instance "+std::to_string(iid));
    }

    return Instances[iid]->backend->getReceptorPositions(neurons, Instances[iid]->model_data);
}

int indk::ComputeInstanceManager::getInstanceCount() const {
    return Instances.size();
}

indk::ComputeInstanceManager::~ComputeInstanceManager() {
    for (int i = 0; i < Instances.size(); i++) {
        doClearInstance(i);
        delete Instances[i];
    }
}
