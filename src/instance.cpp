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
#include <algorithm>

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
        return;
    }

    auto instance = Instances[iid];
//    auto bdata = indk::System::getComputeBackend(instance->backend_id);
    instance -> model_data = instance -> backend -> doTranslate(links, outputs, sync);
}

void indk::ComputeInstanceManager::doRunInstance(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, int iid) {
    if (iid >= Instances.size()) {
        return; // exception
    }

    if (!Instances[iid]->model_data) {
        std::cout << "model data err" << std::endl;
        return;
    }

    if (x.size() != inputs.size() || x.empty()) {
        std::cout << "input err" << std::endl;
        return;
    }

    Instances[iid] -> status = InstanceBusy;
    Instances[iid] -> backend -> doCompute(x, inputs, Instances[iid]->model_data);
    Instances[iid] -> status = InstanceReady;
}

void indk::ComputeInstanceManager::doResetInstance(int iid) {
    if (iid >= Instances.size()) {
        return; // exception
    }

    if (Instances[iid]->model_data) Instances[iid] -> backend -> doReset(Instances[iid]->model_data);
}

std::vector<std::pair<float, std::string>> indk::ComputeInstanceManager::getOutputValues(int iid) {
//    auto model = (indk::Translators::CPU::ModelData*)Instances[iid]->model_data;

    std::vector<std::pair<float, std::string> > outputs;
//    outputs.reserve(model->outputs.size());
//    for (const auto &o: model->outputs) {
//        auto output = o->t == 0 ? 0 : o->output[o->t-1];
//        outputs.emplace_back(output, o->name);
//    }

    return outputs;
}
