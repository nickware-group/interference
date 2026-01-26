/////////////////////////////////////////////////////////////////////////////
// Name:        translators/cpu.cpp
// Purpose:     CPU translator class
// Author:      Nickolay Babich
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
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
    params -> batch_size = 0;
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

void indk::Translators::CPU::doReset(const std::vector<std::string> &neurons, indk::Translators::CPU::ModelData *model) {
    std::map<std::string, NeuronParams*> local;

    for (auto &n: neurons) {
        auto f = model->objects.find(n);
        if (f != model->objects.end()) {
            local.emplace(*f);
        }
    }

    if (local.empty()) local = model->objects;

    for (const auto &o: local) {
        auto neuron = o.second;
        neuron -> t = 0;
        neuron -> batch_size = 0;

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

void indk::Translators::CPU::doClear(indk::Translators::CPU::ModelData *model) {
    for (const auto &o: model->objects) {
        auto neuron = o.second;

        for (int i = 0; i < neuron->receptor_count; i++) {
            auto r = neuron -> receptors[i];
            delete [] r.position;
            delete [] r.position_default;
        }
        delete [] neuron->receptors;

        for (int j = 0; j < neuron->entry_count; j++) {
            auto e = neuron -> entries[j];
            for (unsigned int k = 0; k < e.synapse_count; k++) {
                delete [] e.synapses[k].position;
            }
            delete [] e.synapses;
        }
        delete [] neuron->entries;
    }

    delete model;
}

std::vector<indk::OutputValue> indk::Translators::CPU::getOutputValues(const std::vector<std::string> &neurons, indk::Translators::CPU::ModelData *model) {
    std::vector<indk::OutputValue> outputs;

    std::vector<NeuronParams*> objects;
    for (const auto &n: neurons) {
        auto nfound = model->objects.find(n);
        if (nfound == model->objects.end()) continue;

        objects.push_back(nfound->second);
    }

    if (objects.empty()) objects = model -> outputs;

    outputs.reserve(objects.size());
    for (const auto &o: objects) {
        auto value = o->t == 0 ? 0 : o->output[o->t-1];
        indk::OutputValue output = {.value=value, .source=o->name, .time=o->t};
        outputs.emplace_back(output);
    }

    return outputs;
}

std::map<std::string, std::vector<indk::Position>> indk::Translators::CPU::getReceptorPositions(const std::vector<std::string> &neurons, indk::Translators::CPU::ModelData *model) {
    std::map<std::string, std::vector<indk::Position>> list;
    std::map<std::string, NeuronParams*> local;

    for (auto &n: neurons) {
        auto f = model->objects.find(n);
        if (f != model->objects.end()) {
            local.emplace(*f);
        }
    }

    if (local.empty()) local = model->objects;

    for (auto &n: local) {
        std::vector<indk::Position> positions;
        for (uint64_t r = 0; r < n.second->receptor_count; r++) {
            positions.emplace_back(n.second->size, std::vector<float>(n.second->receptors[r].position, n.second->receptors[r].position+n.second->dimension_count));
        }
        list.insert(std::make_pair(n.second->name, positions));
    }

    return list;
}

std::string indk::Translators::CPU::getTranslatorName() {
    return "CPU";
}
