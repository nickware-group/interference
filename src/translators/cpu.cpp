/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <indk/translators/cpu.h>
#include <cstring>
#include <algorithm>

indk::Translators::CPU::NeuronParams* indk::Translators::CPU::doTranslateNeuronToInstance(indk::Neuron *neuron, indk::Neuron *nfrom,
                                                                                          NeuronParams *pfrom,
                                                                                          std::map<void*, NeuronParams*> &nobjects,
                                                                                          std::map<std::string, NeuronParams*> &objects) {
    if (!neuron) return nullptr;
    auto found = nobjects.find(neuron);

    if (found != nobjects.end()) {
        if (nfrom != nullptr) {
            for (int j = 0; j < neuron->getEntriesCount(); j++) {
                if (found->second->entries[j].input_from == nfrom->getName()) {
                    found->second->entries[j].input = pfrom;
                    found->second->entries[j].entry_type = 1;
                }
            }
        }

        return found->second;
    }

    auto params = new NeuronParams();
    params -> name = neuron -> getName();
    params -> size = neuron -> getXm();
    params -> dimension_count = neuron -> getDimensionsCount();
    params -> entry_count = neuron->getEntriesCount();
    params -> receptor_count = neuron->getReceptorsCount();
    params -> entries = new EntryParams[neuron->getEntriesCount()];
    params -> receptors = new ReceptorParams[neuron->getReceptorsCount()];
    params -> output = nullptr;
    params -> position_buffer = new float[params->dimension_count];
    params -> t = 0;
    params -> latency = neuron->getLatency();

    auto elist = neuron -> getEntries();
    for (int j = 0; j < neuron->getEntriesCount(); j++) {
        auto e = neuron -> getEntry(j);

        params -> entries[j].entry_type = 0;
        params -> entries[j].input_from = elist[j];
        params -> entries[j].synapses = new SynapseParams[e->getSynapsesCount()];
        params -> entries[j].synapse_count = e -> getSynapsesCount();

        for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
            auto s = e -> getSynapse(k);

            params -> entries[j].synapses[k].tl = s -> getTl();
            params -> entries[j].synapses[k].k1 = s -> getk1();
            params -> entries[j].synapses[k].k2 = s -> getk2();
            params -> entries[j].synapses[k].gamma = s -> getGamma();
            params -> entries[j].synapses[k].d_gamma = s -> getdGamma();
            params -> entries[j].synapses[k].lambda = s -> getLambda();
            params -> entries[j].synapses[k].neurotransmitter_type = s -> getNeurotransmitterType();
            params -> entries[j].synapses[k].position = new float[params->dimension_count];

            for (int p = 0; p < params->dimension_count; p++) {
                params -> entries[j].synapses[k].position[p] = s -> getPos() -> getPositionValue(p);
            }
        }

        params -> entries[j].input = nullptr;
        if (nfrom != nullptr) {
            if (params->entries[j].input_from == nfrom->getName()) {
                params -> entries[j].input = pfrom;
                params -> entries[j].entry_type = 1;
            }
        }
    }

    for (int j = 0; j < neuron->getReceptorsCount(); j++) {
        auto r = neuron -> getReceptor(j);
        params -> receptors[j].k3 = r -> getk3();
        params -> receptors[j].rs = r -> getRs();
        params -> receptors[j].rs_default = params -> receptors[j].rs;
        params -> receptors[j].fi = r -> getFi();
        params -> receptors[j].d_fi = r -> getdFi();
        params -> receptors[j].l = r -> getL();
        params -> receptors[j].position = new float[params->dimension_count];
        params -> receptors[j].position_default = new float[params->dimension_count];

        for (int p = 0; p < params->dimension_count; p++) {
            params -> receptors[j].position[p] = r -> getPos() -> getPositionValue(p);
            params -> receptors[j].position_default[p] = r -> getPos() -> getPositionValue(p);
        }
    }

    nobjects.emplace(neuron, params);
    objects.emplace(params->name, params);
    return params;
}

void* indk::Translators::CPU::doTranslate(const indk::LinkList &links, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync) {
    auto model = new ModelData;

    model -> outputs.clear();
    model -> batch_size = 0;

    std::map<void*, NeuronParams*> nobjects;

    for (const auto &l: links) {
        auto from = std::get<0>(l);
        auto to = std::get<1>(l);
        auto nfrom = (indk::Neuron*)std::get<2>(l);
        auto nto = (indk::Neuron*)std::get<3>(l);

        // translating neurons
        auto pfrom = doTranslateNeuronToInstance(nfrom, nullptr, nullptr, nobjects, model->objects);
        doTranslateNeuronToInstance(nto, nfrom, pfrom, nobjects, model->objects);
    }

    // linking outputs
    for (const auto &output: outputs) {
        auto found = model->objects.find(output);
        if (found != model->objects.end()) model->outputs.push_back(found->second);
    }

    return model;
}

void indk::Translators::CPU::doReset(indk::Translators::CPU::ModelData *model) {
    for (const auto &o: model->objects) {
        auto neuron = o.second;
        neuron -> t = 0;

        for (int i = 0; i < neuron->receptor_count; i++) {
            auto r = o.second -> receptors[i];
            r.fi = 0;
            r.l = 0;
            r.rs = r.rs_default;
            memcpy(r.position, r.position_default, o.second->dimension_count);
        }
        for (int j = 0; j < neuron->entry_count; j++) {
            auto e = o.second->entries[j];
            for (unsigned int k = 0; k < e.synapse_count; k++) {
                e.synapses[k].gamma = 0;
                e.synapses[k].d_gamma = 0;
                e.synapses[k].l_gamma = 0;
                e.synapses[k].l_d_gamma = 0;
            }
        }
    }
}

std::string indk::Translators::CPU::getTranslatorName() {
    return "CPU";
}
