/////////////////////////////////////////////////////////////////////////////
// Name:        instance.cpp
// Purpose:     Compute Instance Manager class
// Author:      Nickolay Babbysh
// Created:     21.10.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/instance.h>
#include <indk/types.h>
#include <indk/neuron.h>
#include <algorithm>
#include "indk/math.h"

void indk::ComputeInstanceManager::doCreateInstance(int backend) {
    auto instance = new InstanceData();
    instance -> backend = indk::System::getComputeBackend(backend);
    instance -> status = InstanceReady;

    Instances.push_back(instance);
}

void indk::ComputeInstanceManager::doTranslateToInstance(const indk::LinkList& links, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync, int iid) {
    if (iid >= Instances.size()) {
        if (!iid) doCreateInstance();
        else return; // exception
    }

    if (links.empty()) {
        std::cout << "err no links" << std::endl;
        return; // TODO: exception
    }

    auto instance = Instances[iid];
    instance -> model_data = instance -> backend -> doTranslate(links, outputs, sync);
}

void indk::ComputeInstanceManager::doRunInstance(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, int iid) {
    if (iid >= Instances.size()) {
        return; // TODO: exception
    }

    if (!Instances[iid]->model_data) {
        std::cout << "model data err" << std::endl;
        return; // TODO: exception
    }

    if (x.size() != inputs.size() || x.empty()) {
        std::cout << "input err" << std::endl;
        return; // TODO: exception
    }

    Instances[iid] -> status = InstanceBusy;
    Instances[iid] -> backend -> doCompute(x, inputs, Instances[iid]->model_data);
    Instances[iid] -> status = InstanceReady;
}

void indk::ComputeInstanceManager::doResetInstance(int iid) {
    if (iid >= Instances.size()) {
        return; // TODO: exception
    }

    if (Instances[iid]->model_data) Instances[iid] -> backend -> doReset(Instances[iid]->model_data);
}

std::vector<indk::OutputValue> indk::ComputeInstanceManager::getOutputValues(int iid) {
    if (iid >= Instances.size()) {
        return {}; // TODO: exception
    }

    return Instances[iid]->backend->getOutputValues(Instances[iid]->model_data);
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeInstanceManager::getReceptorValues(int iid) {
    if (iid >= Instances.size()) {
        return {}; // TODO: exception
    }

    return Instances[iid]->backend->getReceptorPositions(Instances[iid]->model_data);
}
