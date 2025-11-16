/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <indk/backends/native.h>
#include <indk/translators/cpu.h>
#include <indk/math.h>
#include <indk/instance.h>

indk::ComputeBackends::NativeCPU::NativeCPU() {
    BackendName = "Native CPU";
    TranslatorName = indk::Translators::CPU::getTranslatorName();
    Ready = true;
}

void* indk::ComputeBackends::NativeCPU::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap &sync) {
    return indk::Translators::CPU::doTranslate(neurons, outputs, sync);
}

void indk::ComputeBackends::NativeCPU::doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_model) {
    auto model = (indk::Translators::CPU::ModelData*)_model;

    float fisum, d, p;
    uint64_t csize = x[0].size();
    uint64_t done = 0;

//    std::cout << csize << std::endl;

    if (!csize) {
        std::cout << "err no signal" << std::endl;
        return;
    }

    // linking necessary neural network inputs to neurons
    for (int i = 0; i < inputs.size(); i++) {
        for (const auto &o: model->objects) {
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

    if (model->batch_size < csize) {
        model -> batch_size = csize;
        for (const auto &o: model->objects) {
            delete [] o.second->output;
            o.second -> output = new float[model->batch_size];
            memset(o.second->output, 0, sizeof(float)*model->batch_size);
        }
    }

    // main loop
    while (done < model->objects.size()) {
        bool processed = false;
        for (const auto &o: model->objects) {
            if (o.second->t == csize) continue;
            if (!model->learning_mode) {
                auto f = model->sync_map.find(o.second->name);
                if (f != model->sync_map.end()) {
                    o.second -> t = csize;
                    continue;
                }
            }
            p = 0;

            bool ready = true;

            if (o.second->t < o.second->latency) {
                o.second->output[o.second->t] = 0;
                o.second->t++;
                processed = true;
//                std::cout << "[" << o.second->name << "]" << " latency worked t " << o.second->t << std::endl;

                continue;
            }

            // checking if all signals are ready
            for (int j = 0; j < o.second->entry_count; j++) {
                auto e = o.second->entries[j];
//                std::cout << "entry type " << o.second->entries[j].entry_type << std::endl;

                if (o.second->entries[j].entry_type) {
                    auto source = ((indk::Translators::CPU::NeuronParams*)o.second->entries[j].input);
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
//                std::cout << "[" << o.second->name << "] " << "not ready" << std::endl;
                continue;
            }

            // computing synapse parameters
            for (int j = 0; j < o.second->entry_count; j++) {
                auto e = o.second->entries[j];
                float *input;

                indk::Translators::CPU::NeuronParams* source = nullptr;

                if (!o.second->entries[j].entry_type) input = (float*)o.second->entries[j].input;   // using neural network input signal
                else {
                    source = ((indk::Translators::CPU::NeuronParams*)o.second->entries[j].input);
                    input = source -> output;                                                       // using signal from linked neuron output
                }

                // compute synapse parameters
                for (int k = 0; k < e.synapse_count; k++) {
                    float value;
                    if (source) {
//                        std::cout << "[" << o.second->name << "] " << source->name << " " << o.second->latency <<
//                                  " t " << o.second->t << std::endl;
                        if (o.second->latency > o.second->t) value = 0;
                        else value = input[o.second->t-o.second->latency];
                    } else value = e.synapses[k].tl > o.second->t ? 0 : input[o.second->t-e.synapses[k].tl];

                    float gamma = indk::Math::getGammaFunctionValue(e.synapses[k].gamma,
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

                auto r = o.second -> receptors[i];
                indk::Math::doClearPosition(o.second->position_buffer, o.second->dimension_count);

                for (int j = 0; j < o.second->entry_count; j++) {
                    auto e = o.second->entries[j];
                    for (int k = 0; k < e.synapse_count; k++) {
                        d = indk::Math::getDistance(e.synapses[k].position, r.position, o.second->dimension_count);
                        auto fivalues = indk::Math::getFiFunctionValue(e.synapses[k].lambda, e.synapses[k].gamma, e.synapses[k].d_gamma, d);
                        if (fivalues.second > 0) {
                            indk::Math::getNewReceptorPosition(o.second->position_buffer,
                                                                   r.position,
                                                                   e.synapses[k].position,
                                                                   indk::Math::getFiVectorLength(fivalues.second),
                                                                   d,
                                                                   o.second->dimension_count);
//                            std::cout << d << " " << r.position[0] << " " << r.position[1] << " " << e.synapses[k].position[0] << " " << e.synapses[k].position[1] << " "  << o.second->position_buffer[0] << " " << o.second->position_buffer[1] << std::endl;
                        }
                        fisum += fivalues.first;
                    }
                }

                r.d_fi = fisum - r.fi;
                r.fi = fisum;

                indk::Math::doAddPosition(r.position, o.second->position_buffer, o.second->dimension_count);
                p += indk::Math::getReceptorInfluenceValue(indk::Math::isReceptorActive(r.fi, r.rs),
                                                               o.second->position_buffer,
                                                               o.second->dimension_count);
                r.rs = indk::Math::getRcValue(r.k3, r.rs, r.fi, r.d_fi);

//                std::cout << "[fi/d_fi/rs/p/pos0/pos1] "  << r.fi << " " << r.d_fi << " " << r.rs << " " << p << " " << o.second->position_buffer[0] << " " << o.second->position_buffer[1] << std::endl;
            }

            p /= (float)o.second -> receptor_count;
//            std::cout << p << std::endl;

            o.second->output[o.second->t] = p;
            o.second->t++;

            if (o.second->t == csize) done++;

            processed = true;

//            std::cout << "[" << o.second->name << "] " << "[compute done] t " << o.second->t << " / " << csize << std::endl;
        }

        if (!processed) {
            std::cout << "[processing error] no signal passing" << std::endl;
            break;
        }
//        std::cout << std::endl;
    }

    if (!model->learning_mode) {
        for (const auto &obj: model->sync_map) {
            auto to = model->objects.find(obj.first);
            auto from = model->objects.find(obj.second);
            if (to != model->objects.end() && from != model->objects.end()) {
                auto neuron1 = to->second;
                auto neuron2 = from->second;
                neuron1 -> t = neuron2 -> t.load();

                if (neuron1->receptor_count != neuron2->receptor_count) continue;

                for (int i = 0; i < neuron1->receptor_count; i++) {
                    auto r1 = neuron1 -> receptors[i];
                    auto r2 = neuron2 -> receptors[i];
                    r1.fi = r2.fi;
                    r1.l = r2.l;
                    r1.rs = r2.rs;
                    std::memcpy(r1.position, r2.position, sizeof(float)*neuron1->dimension_count);
                }

                if (neuron1->entry_count != neuron2->entry_count) continue;

                for (int j = 0; j < neuron1->entry_count; j++) {
                    auto e1 = neuron1->entries[j];
                    auto e2 = neuron2->entries[j];
                    if (e1.synapse_count != e2.synapse_count) continue;
                    for (unsigned int k = 0; k < e1.synapse_count; k++) {
                        e1.synapses[k].gamma = e2.synapses[k].gamma;
                        e1.synapses[k].d_gamma = e2.synapses[k].d_gamma;
                        e1.synapses[k].l_gamma = e2.synapses[k].l_gamma;
                        e1.synapses[k].l_d_gamma = e2.synapses[k].l_d_gamma;
                    }
                }
            }
        }
    }
}

void indk::ComputeBackends::NativeCPU::doReset(void *model) {
    indk::Translators::CPU::doReset((indk::Translators::CPU::ModelData*)model);
}

void indk::ComputeBackends::NativeCPU::setMode(void *_model, bool learning) {
    auto model = (indk::Translators::CPU::ModelData*)_model;
    model -> learning_mode = learning;
}

void indk::ComputeBackends::NativeCPU::setParameters(indk::ComputeBackend::Parameters*) {
}

std::vector<indk::OutputValue> indk::ComputeBackends::NativeCPU::getOutputValues(void *_model) {
    auto model = (indk::Translators::CPU::ModelData*)_model;

    std::vector<indk::OutputValue> outputs;
    outputs.reserve(model->outputs.size());

    for (const auto &o: model->outputs) {
        auto value = o->t == 0 ? 0 : o->output[o->t-1];
        indk::OutputValue output = {.value=value, .source=o->name, .time=o->t};
        outputs.emplace_back(output);
    }

    return outputs;
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeBackends::NativeCPU::getReceptorPositions(void *_model) {
    auto model = (indk::Translators::CPU::ModelData*)_model;

    std::map<std::string, std::vector<indk::Position>> list;

    for (auto &n: model->objects) {
        std::vector<indk::Position> positions;
        for (uint64_t r = 0; r < n.second->receptor_count; r++) {
            positions.emplace_back(n.second->size, std::vector<float>(n.second->receptors[r].position, n.second->receptors[r].position+n.second->dimension_count));
        }
        list.insert(std::make_pair(n.second->name, positions));
    }
    return list;
}
