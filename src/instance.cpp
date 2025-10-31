/////////////////////////////////////////////////////////////////////////////
// Name:        instance.cpp
// Purpose:     Compute Instance Manager class
// Author:      Nickolay Babbysh
// Created:     21.10.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/instance.h>
#include <cstring>
#include <algorithm>
#include "indk/neuron.h"

void indk::ComputeInstanceManager::doCreateInstance(int backend) {
    auto instance = new InstanceData();
    instance -> backend = backend;
    instance -> status.store(InstanceReady);

    Instances.push_back(instance);
}

indk::ComputeInstanceManager::NeuronParams* indk::ComputeInstanceManager::doTranslateNeuronToInstance(indk::Neuron *neuron, indk::Neuron *nfrom, NeuronParams *pfrom, std::map<void*, NeuronParams*>& objects) {
    auto found = objects.find(neuron);

    if (found != objects.end()) {
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
        params -> receptors[j].fi = r -> getFi();
        params -> receptors[j].d_fi = r -> getdFi();
        params -> receptors[j].l = r -> getL();
        params -> receptors[j].position = new float[params->dimension_count];

        for (int p = 0; p < params->dimension_count; p++) {
            params -> receptors[j].position[p] = r -> getPos() -> getPositionValue(p);
        }
    }

    objects.emplace(neuron, params);
    return params;
}

void indk::ComputeInstanceManager::doTranslateToInstance(const indk::LinkList& links, const std::vector<std::string>& outputs, int id) {
    if (id >= Instances.size()) {
        if (!id) doCreateInstance();
        else return; // exception
    }

    if (links.empty()) {
        std::cout << "err no links" << std::endl;
        return;
    }

    auto instance = Instances[id];
    instance -> batch_size = 0;
    instance -> outputs.clear();
    instance -> max_latency = std::abs(std::get<4>(links[0]));

    std::cout << "Parse neurons" << std::endl;

    for (const auto &l: links) {
        auto from = std::get<0>(l);
        auto to = std::get<1>(l);
        auto nfrom = (indk::Neuron*)std::get<2>(l);
        auto nto = (indk::Neuron*)std::get<3>(l);

        // translating neurons
        auto pfrom = doTranslateNeuronToInstance(nfrom, nullptr, nullptr, instance->objects);
        auto pto = doTranslateNeuronToInstance(nto, nfrom, pfrom, instance->objects);

        // linking outputs

        for (const auto &output: outputs) {
            auto f = std::find_if(instance->outputs.begin(), instance->outputs.end(), [output](NeuronParams*p) { return  p->name == output; });
            if (f == instance->outputs.end()) {
                if (from == output) instance->outputs.push_back(pfrom);
                else if (to == output) instance->outputs.push_back(pto);
            }
        }
    }
}

void indk::ComputeInstanceManager::doTranslateFromInstance(int id) {
    auto instance = Instances[id];
}

void indk::ComputeInstanceManager::doRunInstance(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, int iid) {
    if (x.size() != inputs.size() || x.empty()) {
        std::cout << "input err" << std::endl;
        return;
    }

    auto instance = Instances[iid];

    float fisum, d, p;
    uint64_t csize = x[0].size();
    uint64_t done = 0;

    std::cout << csize << std::endl;

    if (!csize) {
        std::cout << "err no signal" << std::endl;
        return;
    }

    // linking necessary neural network inputs to neurons
    for (int i = 0; i < inputs.size(); i++) {
        for (const auto &o: instance->objects) {
            for (int j = 0; j < o.second->entry_count; j++) {
                if (!o.second->entries[j].entry_type) {
                    if (o.second->entries[j].input_from == inputs[i]) o.second->entries[j].input = (void*)x[i].data();
                }
            }
        }
        if (i && csize != x[i].size()) {
            // signals consistency is broken
            std::cout << "signals cons err" << std::endl;
            return;
        }
    }

    if (instance->batch_size < csize) {
        instance -> batch_size = csize;
        for (const auto &o: instance->objects) {
            delete [] o.second->output;
            o.second -> output = new float[instance->batch_size];
            memset(o.second->output, 0, sizeof(float)*instance->batch_size);
        }
    }

    // main loop
    while (done < instance->objects.size()) {
        bool processed = false;
        for (const auto &o: instance->objects) {
            if (o.second->t == csize) continue;

            bool ready = true;

            if (o.second->t < o.second->latency) {
                o.second->output[o.second->t] = 0;
                o.second->t++;
                processed = true;
                std::cout << "[" << o.second->name << "]" << " latency worked t " << o.second->t << std::endl;

                continue;
            }

            // checking if all signals are ready
            for (int j = 0; j < o.second->entry_count; j++) {
                auto e = o.second->entries[j];
//                std::cout << "entry type " << o.second->entries[j].entry_type << std::endl;

                if (o.second->entries[j].entry_type) {
                    auto source = ((NeuronParams*)o.second->entries[j].input);
//                    std::cout << "[" << o.second->name << "] " << source->name <<
//                                 " t " << source->t << " <= " << o.second->t << " l " << o.second->latency << std::endl;

                    if ((o.second->t < o.second->latency && source->t <= o.second->t) || (o.second->t >= o.second->latency && source->t <= o.second->t-o.second->latency)) {
                        bool latency = false;
                        for (int k = 0; k < e.synapse_count; k++) {
                            if (e.synapses[k].tl > o.second->t) {
                                latency = true;
                            }
                        }
                        if (!latency) {
                            ready = false;
                            break;
                        }
                    }
                }
            }

            // go next if not ready
            if (!ready) {
                std::cout << "[" << o.second->name << "] " << "not ready" << std::endl;
                continue;
            }

            // computing synapse parameters
            for (int j = 0; j < o.second->entry_count; j++) {
                auto e = o.second->entries[j];
                float *input;

                NeuronParams* source = nullptr;

                if (!o.second->entries[j].entry_type) input = (float*)o.second->entries[j].input;   // using neural network input signal
                else {
                    source = ((NeuronParams*)o.second->entries[j].input);
                    input = source -> output;                                                       // using signal from linked neuron output
                }

                // compute synapse parameters
                for (int k = 0; k < e.synapse_count; k++) {
                    float value;
                    if (source) {
                        if (o.second->latency > o.second->t) value = 0;
                        else value = input[o.second->t-o.second->latency];
                    } else value = e.synapses[k].tl > o.second->t ? 0 : input[o.second->t-e.synapses[k].tl];

                    float gamma = indk::Computer::getGammaFunctionValue(e.synapses[k].gamma,
                                                                        e.synapses[k].k1,
                                                                        e.synapses[k].k2,
                                                                        value);

                    e.synapses[k].l_gamma = e.synapses[k].gamma;
                    e.synapses[k].l_d_gamma = e.synapses[k].d_gamma;
                    e.synapses[k].d_gamma = gamma - e.synapses[k].gamma;
                    e.synapses[k].gamma = gamma;

//                    std::cout << "[input value/gamma] " << value << " " << e.synapses[k].gamma << std::endl;
                }
            }

            // compute new receptors positions
            for (int i = 0; i < o.second->receptor_count; i++) {
                fisum = 0;
                p = 0;
                auto r = o.second -> receptors[i];
                indk::Computer::doClearPosition(o.second->position_buffer, o.second->dimension_count);

                for (int j = 0; j < o.second->entry_count; j++) {
                    auto e = o.second->entries[j];
                    for (int k = 0; k < e.synapse_count; k++) {
                        d = indk::Computer::getDistance(e.synapses[k].position, r.position, o.second->dimension_count);
                        auto fivalues = indk::Computer::getFiFunctionValue(e.synapses[k].lambda, e.synapses[k].gamma, e.synapses[k].d_gamma, d);
                        if (fivalues.second > 0) {
                            indk::Computer::getNewReceptorPosition(o.second->position_buffer,
                                                                   r.position,
                                                                   e.synapses[k].position,
                                                                   indk::Computer::getFiVectorLength(fivalues.second),
                                                                   d,
                                                                   o.second->dimension_count);
                        }
                        fisum += fivalues.first;
                    }
                }

                r.d_fi = fisum - r.fi;
                r.fi = fisum;

                indk::Computer::doAddPosition(r.position, o.second->position_buffer, o.second->dimension_count);
                p += indk::Computer::getReceptorInfluenceValue(indk::Computer::isReceptorActive(r.fi, r.rs),
                                                               r.d_fi,
                                                               o.second->position_buffer,
                                                               o.second->dimension_count);
                r.rs = indk::Computer::getRcValue(r.k3, r.rs, r.fi, r.d_fi);

//                std::cout << "[d_fi/rs] "  << r.d_fi << " " << r.rs << std::endl;
            }

            std::cout << p << std::endl;

            p /= (float)o.second -> receptor_count;
            o.second->output[o.second->t] = p;
            o.second->t++;

            if (o.second->t == csize) done++;

            processed = true;

            std::cout << "[" << o.second->name << "] " << "[compute done] t " << o.second->t << " / " << csize << std::endl;
        }

        if (!processed) {
            std::cout << "[processing error] no signal passing" << std::endl;
            break;
        }

        std::cout << std::endl;
    }
}

std::vector<std::pair<float, std::string>> indk::ComputeInstanceManager::getOutputValues(int iid) {
    auto instance = Instances[iid];

    std::vector<std::pair<float, std::string> > outputs;
    outputs.reserve(instance->outputs.size());
    for (const auto &o: instance->outputs) {
        auto output = o->t == 0 ? 0 : o->output[o->t-1];
        outputs.emplace_back(output, o->name);
    }

    return outputs;
}
