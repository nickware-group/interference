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

indk::Translators::CPU::NeuronParams* indk::Translators::CPU::doTranslateNeuronToInstance(indk::Neuron *neuron, std::map<std::string, NeuronParams*> &objects) {
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
        auto found = objects.find(elist[j]);
        if (found != objects.end()) {
//            std::cout << params->name << " <- " << elist[j] << std::endl;
            params -> entries[j].input = found->second;
            params -> entries[j].entry_type = 1;
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
            params -> receptors[j].position[p] = r -> getPos0() -> getPositionValue(p);
            params -> receptors[j].position_default[p] = r -> getPos0() -> getPositionValue(p);
        }
    }

    auto outputs = neuron -> getLinkOutput();
    for (const auto &oname: outputs) {
        auto found = objects.find(oname);
        if (found != objects.end()) {
            for (int i = 0; i < found->second->entry_count; i++) {
                if (found->second->entries[i].input_from == params->name) {
//                    std::cout << params->name << " -> " << oname << std::endl;
                    found->second->entries[i].input = params;
                    found->second->entries[i].entry_type = 1;
                }
            }
        }
    }

    objects.emplace(params->name, params);
    return params;
}

void* indk::Translators::CPU::doTranslate(const std::vector<indk::Neuron*> &neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync) {
    auto model = new ModelData;

    model -> outputs.clear();
    model -> sync_map = sync;
    model -> batch_size = 0;

    // translating neurons
    for (const auto &n: neurons) {
        doTranslateNeuronToInstance(n, model->objects);
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
            std::memcpy(r.position, r.position_default, sizeof(float)*o.second->dimension_count);
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
