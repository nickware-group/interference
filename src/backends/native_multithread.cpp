/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <algorithm>
#include <indk/backends/native_multithread.h>
#include <indk/math.h>
#include <indk/instance.h>

#define INDK_THREAD_DEFAULT_NUM 2

indk::ComputeBackends::NativeCPUMultithread::NativeCPUMultithread() {
    BackendName = "Native CPU multithread";
    TranslatorName = indk::Translators::CPU::getTranslatorName();

    WorkerCount = INDK_THREAD_DEFAULT_NUM;
    while (Workers.size() < WorkerCount) {
        auto context = new Context;
        context -> thread_id = Workers.size();
        context -> thread = std::thread(tCompute, context);
        Workers.emplace_back(context);
    }

    Ready = true;
}

void* indk::ComputeBackends::NativeCPUMultithread::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync) {
    return indk::Translators::CPU::doTranslate(neurons, outputs, sync);
}

void indk::ComputeBackends::NativeCPUMultithread::doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_model) {
    auto model = (indk::Translators::CPU::ModelData*)_model;

    uint64_t csize = x[0].size();

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

    Task task;
    task.sync_map = model -> sync_map;
    task.learning_mode = model -> learning_mode;
    task.task_elements_done = 0;
    task.workers_done = 0;
    task.compute_size = csize;
    task.task_size = model->objects.size();
    while (task.neurons.size() < WorkerCount) {
        task.neurons.emplace_back();
        task.events.emplace_back(new indk::Event());
    }

    int last = 0;
    for (const auto &o: model->objects) {
        if (last >= WorkerCount) last = 0;
        task.neurons[last].emplace_back(o.second);
        last++;
    }

    for (const auto &w: Workers) {
        w -> task_lock.lock();
        w -> tasks.emplace_back(&task);
        w -> task_lock.unlock();
        w -> cv.notify_one();
    }

    while (task.workers_done != task.events.size()) {
        for (const auto &e: task.events) {
            e -> doWaitTimed(100);
        }
    }

    for (auto e: task.events) {
        delete e;
    }
}

[[noreturn]] void indk::ComputeBackends::NativeCPUMultithread::tCompute(indk::ComputeBackends::NativeCPUMultithread::Context *context) {
    float fisum, d, p;
    int ti;

    while (true) {
        std::unique_lock<std::mutex> lk(context->m);
        context -> cv.wait(lk);

        ti = 0;

        // main loop
        while (true) {
            context->task_lock.lock();
            if (context->tasks.empty()) {
                context->task_lock.unlock();
                break;
            }
            if (ti >= context->tasks.size()) ti = 0;
            auto task = context -> tasks[ti];
            context->task_lock.unlock();

            if (task->task_elements_done != task->task_size) {
                auto neurons = task->neurons[context->thread_id];
                for (const auto &n: neurons) {
                    if (n->t == task->compute_size) continue;
                    if (!task->learning_mode) {
                        auto f = task->sync_map.find(n->name);
                        if (f != task->sync_map.end()) continue;
                    }

                    p = 0;

                    bool ready = true;

                    if (n->t < n->latency) {
                        n->output[n->t] = 0;
                        n->t++;
//                    std::cout << "[" << n->name << "]" << " latency worked t " << n->t << std::endl;

                        continue;
                    }

                    // checking if all signals are ready
                    for (int j = 0; j < n->entry_count; j++) {
                        auto e = n->entries[j];
//                std::cout << "entry type " << n->entries[j].entry_type << std::endl;

                        if (n->entries[j].entry_type) {
                            auto source = ((indk::Translators::CPU::NeuronParams*)n->entries[j].input);
//                    std::cout << "[" << n->name << "] " << source->name <<
//                                 " t " << source->t << " <= " << n->t << " l " << n->latency << std::endl;

                            if ((n->t < n->latency && source->t <= n->t) || (n->t >= n->latency && source->t <= n->t-n->latency)) {
                                bool latency = false;
                                for (int k = 0; k < e.synapse_count; k++) {
                                    if (e.synapses[k].tl > n->t) {
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
//                std::cout << "[" << n->name << "] " << "not ready" << std::endl;
                        continue;
                    }

                    // computing synapse parameters
                    for (int j = 0; j < n->entry_count; j++) {
                        auto e = n->entries[j];
                        float *input;

                        indk::Translators::CPU::NeuronParams* source = nullptr;

                        if (!n->entries[j].entry_type) input = (float*)n->entries[j].input;   // using neural network input signal
                        else {
                            source = ((indk::Translators::CPU::NeuronParams*)n->entries[j].input);
                            input = source -> output;                                                       // using signal from linked neuron output
                        }

                        // compute synapse parameters
                        for (int k = 0; k < e.synapse_count; k++) {
                            float value;
                            if (source) {
//                        std::cout << "[" << n->name << "] " << source->name << " " << n->latency <<
//                                  " t " << n->t << std::endl;
                                if (n->latency > n->t) value = 0;
                                else value = input[n->t-n->latency];
                            } else value = e.synapses[k].tl > n->t ? 0 : input[n->t-e.synapses[k].tl];

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
                    for (int i = 0; i < n->receptor_count; i++) {
                        fisum = 0;

                        auto r = n -> receptors[i];
                        indk::Math::doClearPosition(n->position_buffer, n->dimension_count);

                        for (int j = 0; j < n->entry_count; j++) {
                            auto e = n->entries[j];
                            for (int k = 0; k < e.synapse_count; k++) {
                                d = indk::Math::getDistance(e.synapses[k].position, r.position, n->dimension_count);
                                auto fivalues = indk::Math::getFiFunctionValue(e.synapses[k].lambda, e.synapses[k].gamma, e.synapses[k].d_gamma, d);
                                if (fivalues.second > 0) {
                                    indk::Math::getNewReceptorPosition(n->position_buffer,
                                                                       r.position,
                                                                       e.synapses[k].position,
                                                                       indk::Math::getFiVectorLength(fivalues.second),
                                                                       d,
                                                                       n->dimension_count);
//                            std::cout << d << " " << r.position[0] << " " << r.position[1] << " " << e.synapses[k].position[0] << " " << e.synapses[k].position[1] << " "  << n->position_buffer[0] << " " << n->position_buffer[1] << std::endl;
                                }
                                fisum += fivalues.first;
                            }
                        }

                        r.d_fi = fisum - r.fi;
                        r.fi = fisum;

                        indk::Math::doAddPosition(r.position, n->position_buffer, n->dimension_count);
                        p += indk::Math::getReceptorInfluenceValue(indk::Math::isReceptorActive(r.fi, r.rs),
                                                                   n->position_buffer,
                                                                   n->dimension_count);
                        r.rs = indk::Math::getRcValue(r.k3, r.rs, r.fi, r.d_fi);

//                std::cout << "[fi/d_fi/rs/p/pos0/pos1] "  << r.fi << " " << r.d_fi << " " << r.rs << " " << p << " " << n->position_buffer[0] << " " << n->position_buffer[1] << std::endl;
                    }

                    p /= (float)n -> receptor_count;
//                std::cout << p << std::endl;

                    n->output[n->t] = p;
                    n->t++;

                    if (n->t == task->compute_size) task->task_elements_done++;

                    if (task->task_elements_done == task->task_size) {
                        break;
                    }
                }
            }

            ti++;

            if (task->task_elements_done == task->task_size) {
//                std::cout << "[" << context->thread_id << "]" << " task done" << std::endl;

                context->task_lock.lock();
                context->tasks.erase(std::remove(context->tasks.begin(), context->tasks.end(), task), context->tasks.end());
                context->task_lock.unlock();

                task -> workers_done++;
                task -> events[context->thread_id] -> doNotifyOne();

                ti = 0;
            }
        }
    }
}

void indk::ComputeBackends::NativeCPUMultithread::doReset(void *model) {
    indk::Translators::CPU::doReset((indk::Translators::CPU::ModelData*)model);
}

void indk::ComputeBackends::NativeCPUMultithread::setMode(void *_model, bool learning) {
    auto model = (indk::Translators::CPU::ModelData*)_model;
    model -> learning_mode = learning;
}

void indk::ComputeBackends::NativeCPUMultithread::setParameters(indk::ComputeBackend::Parameters *parameters) {
    WorkerCount = ((Parameters*)parameters) -> worker_count;
    while (Workers.size() < WorkerCount) {
        auto context = new Context;
        context -> thread_id = Workers.size();
        context -> thread = std::thread(tCompute, context);
        Workers.emplace_back(context);
    }
}

std::vector<indk::OutputValue> indk::ComputeBackends::NativeCPUMultithread::getOutputValues(void *model) {
    return indk::Translators::CPU::getOutputValues((indk::Translators::CPU::ModelData*)model);
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeBackends::NativeCPUMultithread::getReceptorPositions(void *model) {
    return indk::Translators::CPU::getReceptorPositions((indk::Translators::CPU::ModelData*)model);
}
