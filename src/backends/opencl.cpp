/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <indk/backends/opencl.h>
#include <indk/math.h>
#include <indk/instance.h>
#include <algorithm>

#define KERNEL(name, ...) std::string name = #__VA_ARGS__

indk::ComputeBackends::OpenCL *handler = nullptr;

indk::ComputeBackends::OpenCL::OpenCL() {
    handler = this;
    BackendName = "OpenCL";
    TranslatorName = indk::Translators::CL::getTranslatorName();
    Ready = false;

#ifdef INDK_OPENCL_SUPPORT
    std::vector<cl::Platform> platforms;

    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No platforms found. Check OpenCL installation!" << std::endl;
        return;
    }

    for (auto &p: platforms) {
        std::vector<cl::Device> devices;
        p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for (auto &d: devices) {
            auto name = d.getInfo<CL_DEVICE_NAME>();
            if (CurrentDeviceName.empty()) CurrentDeviceName = name;
            auto device = new DeviceContext;
            device->ready = false;
            device->device = d;
            device->platform_name = p.getInfo<CL_PLATFORM_NAME>();
            device->device_name = name;
            device->compute_units = d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
            device->workgroup_size = d.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
            DeviceList.insert(std::make_pair(name, device));
        }
    }

    Context = cl::Context(CL_DEVICE_TYPE_ALL);

    if (!DeviceList.empty()) Ready = true;
#endif
}

indk::ComputeBackends::OpenCL::DeviceContext* indk::ComputeBackends::OpenCL::doInitCurrentDevice() {
    if (!Ready) return {};

#ifdef INDK_OPENCL_SUPPORT
    auto found = DeviceList.find(CurrentDeviceName);

    if (found == DeviceList.end()) {
        throw indk::Error(indk::Error::EX_BACKEND_CL_DEVICE_NOT_FOUND, "selected device name "+CurrentDeviceName);
    }

    auto dcontext = found->second;

    if (dcontext->ready) return dcontext;

    KERNEL(kernel_code_pairs, __kernel void indk_kernel_pairs(__global float16 *pairs,
                                                              __global float2 *inputs,
                                                              __global float *outputs) {
        int id = get_global_id(0);
        // pairs.s0 - receptor x
        // pairs.s1 - receptor y
        // pairs.s2 - synapse x              (const)
        // pairs.s3 - synapse y              (const)

        // pairs.s4 - synapse gamma value
        // pairs.s5 - input index            (const)

        // pairs.s6 - lambda                 (const)
        // pairs.s7 - k1                     (const)
        // pairs.s8 - k2                     (const)

        // pairs.s9 - reserved
        // pairs.sA - reserved
        // pairs.sB - reserved

        // check if we need to compute this pair by flag
        int run = inputs[(int)pairs[id].s5].s0;
        if (run) {
            float in = inputs[(int)pairs[id].s5].s1;

            // vector length
            float d = 0;
            d += (pairs[id].s0-pairs[id].s2)*(pairs[id].s0-pairs[id].s2);
            d += (pairs[id].s1-pairs[id].s3)*(pairs[id].s1-pairs[id].s3);
            d = sqrt(d);

            float ngamma = pairs[id].s4 + (pairs[id].s7*in-pairs[id].s4/pairs[id].s8);

            float e = pairs[id].s6 * exp(-pairs[id].s6*d);
            float fi = ngamma * e;
            float dfi = (ngamma-pairs[id].s4) * e;
            float nx = 0, ny = 0;

            if (dfi > 0 && d > 0) {
                float nposd = sqrt(dfi) / d;
                nx = (pairs[id].s0-pairs[id].s2) * nposd;
                ny = (pairs[id].s1-pairs[id].s3) * nposd;
            }

            // update gamma value
            pairs[id].s4 = ngamma;

            pairs[id].s9 = nx;
            pairs[id].sA = ny;
            pairs[id].sB = fi;
        };
    });

    KERNEL(kernel_code_receptors, __kernel void indk_kernel_receptors(__global float8 *receptors,
                                                                      __global float16 *pairs,
                                                                      __global float2 *inputs) {
        int id = get_global_id(0);
        // receptors.s0 - left pairs range edge           (const)
        // receptors.s1 - right pairs range edge          (const)
        // receptors.s2 - input index                     (const)

        // receptors.s3 - receptor sensitivity
        // receptors.s4 - neurotransmitter level value

        // receptors.s5 - k3                              (const)

        // receptors.s6 - reserved

        int run = inputs[(int)receptors[id].s2].s0;
        if (run) {
            float drx = 0, dry = 0, fisum = 0;

            for (int i = receptors[id].s0; i < receptors[id].s1; i++) {
                drx += pairs[i].s9;
                dry += pairs[i].sA;
                fisum += pairs[i].sB;
            }

            for (int i = receptors[id].s0; i < receptors[id].s1; i++) {
                pairs[i].s0 += drx;
                pairs[i].s1 += dry;
            }

            float dfisum = fisum - receptors[id].s4;
            receptors[id].s4 = fisum;

            float d = drx*drx + dry*dry;
            d = sqrt(d);

            float p = 0;
            if (d > 0 && fisum > receptors[id].s3) p = d;

            if (dfisum > 0 && fisum >= receptors[id].s3) receptors[id].s3 += dfisum;
            else receptors[id].s3 = receptors[id].s3 / (receptors[id].s5*receptors[id].s3+1);

            receptors[id].s6 = p;
        };
    });

    KERNEL(kernel_code_neurons, __kernel void indk_kernel_neurons(__global float4 *neurons,
                                                                  __global float8 *receptors,
                                                                  __global float2 *inputs,
                                                                  __global float *outputs,
                                                                  __global int *times) {
        int id = get_global_id(0);
        // neurons.s0 - left receptors range edge           (const)
        // neurons.s1 - right receptors range edge          (const)
        // neurons.s2 - input index                         (const)
        // neurons.s3 - latency

        int run = inputs[(int)neurons[id].s2].s0;
        if (run) {
            float p = 0;
            int rcount = neurons[id].s1 - neurons[id].s0;

            for (int i = neurons[id].s0; i < neurons[id].s1; i++) {
                p += receptors[i].s6;
            }
            p /= (float)rcount;

            outputs[id] = p;
            times[id]++;
        } else {
           outputs[id] = -1;
        };
    });

    int status;

    auto pairs = cl::Program(Context, {kernel_code_pairs.c_str(),kernel_code_pairs.length()+1});
    if ((status = pairs.build({dcontext->device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << pairs.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dcontext->device) << std::endl;

    }

    auto receptors = cl::Program(Context, {kernel_code_receptors.c_str(),kernel_code_receptors.length()+1});
    if ((status = receptors.build({dcontext->device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << receptors.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dcontext->device) << std::endl;

    }

    auto neurons = cl::Program(Context, {kernel_code_neurons.c_str(),kernel_code_neurons.length()+1});
    if ((status = neurons.build({dcontext->device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << neurons.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dcontext->device) << std::endl;

    }

    dcontext->pairs = cl::Kernel(pairs, "indk_kernel_pairs");
    dcontext->receptors = cl::Kernel(receptors, "indk_kernel_receptors");
    dcontext->neurons = cl::Kernel(neurons, "indk_kernel_neurons");

    dcontext->ready = true;

    return dcontext;
#endif
}

void* indk::ComputeBackends::OpenCL::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap &sync) {
    return indk::Translators::CL::doTranslate(neurons, outputs, sync);
}

void indk::ComputeBackends::OpenCL::doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_model) {
    auto model = (indk::Translators::CL::ModelData*)_model;

#ifdef INDK_OPENCL_SUPPORT
    auto dcontext = doInitCurrentDevice();

    cl::Buffer pairs_buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float16)*model->pair_pool_size);
    cl::Buffer receptors_buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float8)*model->receptor_pool_size);
    cl::Buffer neurons_buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float3)*model->neuron_pool_size);
    cl::Buffer inputs_buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float2)*model->input_pool_size);
    cl::Buffer outputs_buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float)*model->neuron_pool_size);
    cl::Buffer times_buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_int)*model->neuron_pool_size);

    dcontext->pairs.setArg(0, pairs_buffer);
    dcontext->pairs.setArg(1, inputs_buffer);
    dcontext->pairs.setArg(2, outputs_buffer);

    dcontext->receptors.setArg(0, receptors_buffer);
    dcontext->receptors.setArg(1, pairs_buffer);
    dcontext->receptors.setArg(2, inputs_buffer);

    dcontext->neurons.setArg(0, neurons_buffer);
    dcontext->neurons.setArg(1, receptors_buffer);
    dcontext->neurons.setArg(2, inputs_buffer);
    dcontext->neurons.setArg(3, outputs_buffer);
    dcontext->neurons.setArg(4, times_buffer);

    auto queue = cl::CommandQueue(Context, dcontext->device);
    queue.enqueueWriteBuffer(pairs_buffer, CL_TRUE, 0, sizeof(cl_float16)*model->pair_pool_size, model->PairsInfo);
    queue.enqueueWriteBuffer(receptors_buffer, CL_TRUE, 0, sizeof(cl_float8)*model->receptor_pool_size, model->ReceptorsInfo);
    queue.enqueueWriteBuffer(neurons_buffer, CL_TRUE, 0, sizeof(cl_float3)*model->neuron_pool_size, model->NeuronsInfo);
    queue.enqueueWriteBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size,  model->Outputs);

    memset(model->Times, 0, sizeof(cl_int)*model->neuron_pool_size);

    model -> batch_size = x[0].size();

    bool done = false;

    std::vector<std::pair<uint64_t, bool>> incache;
    uint64_t i = 0;
    for (const auto &n: model->objects) {
        auto elist = n -> getEntries();

        for (const auto &e: elist) {
            auto ifound = std::find(inputs.begin(), inputs.end(), e);
            if (ifound != inputs.end()) {
                auto index = std::distance(inputs.begin(), ifound);
                incache.emplace_back(index, false);
            } else {
                auto nfound = std::find_if(model->objects.begin(), model->objects.end(), [e](indk::Neuron *n) { return n->getName()==e; });
                if (nfound != model->objects.end()) {
                    auto index = std::distance(model->objects.begin(), nfound);
                    incache.emplace_back(index, true);
                }
            }
        }
        model -> output_sequence.emplace_back();

        while (model->Times[i] < n->getLatency()) {
            model->Times[i]++;
            model -> output_sequence.back().emplace_back(0);
        }

        i++;
    }

    queue.enqueueWriteBuffer(times_buffer, CL_TRUE, 0, sizeof(cl_int)*model->neuron_pool_size,  model->Times);

    while (!done) {
        done = true;

        uint64_t xi = 0, xistart;
        i = 0;
        for (const auto &n: model->objects) {
            auto elist = n -> getEntries();
            bool ready = true;

            if (model->Times[i] == model->batch_size) {
                ready = false;
            } else {
                xistart = xi;
                for (const auto &e: elist) {
                    if (std::get<1>(incache[xi])) {
                        auto source = std::get<0>(incache[xi]);
                        auto st = model->Times[source];
                        if (st <= model->Times[i]-n->getLatency()) {
                            ready = false;
                            break;
                        }
                    }
                    xi++;
                }
                xi = xistart;
            }

            for (const auto &e: elist) {
                if (ready) {
                    if (!std::get<1>(incache[xi])) {
                        model->Inputs[xi] = {
                                static_cast<cl_float>(1),
                                static_cast<cl_float>(x[std::get<0>(incache[xi])][model->Times[i]]),
                        };
                    } else {
                        auto source = std::get<0>(incache[xi]);
                        if (model->Times[i]-n->getLatency() < 0) std::cout << model->Times[i]-n->getLatency() << " err" << std::endl;
                        model->Inputs[xi] = {
                                static_cast<cl_float>(1),
                                static_cast<cl_float>(model->output_sequence[source][model->Times[i]-n->getLatency()]),
                        };
                    }
                } else {
                    model->Inputs[xi] = {
                        static_cast<cl_float>(0),
                        static_cast<cl_float>(0),
                    };
                }

//                std::cout << n->getName() << " " << e << " " << model->Inputs[xi].s0 << " " << model->Inputs[xi].s1 << std::endl;
                xi++;
            }
            i++;
        }

        queue.enqueueWriteBuffer(inputs_buffer, CL_TRUE, 0, sizeof(cl_float2)*model->input_pool_size, model->Inputs);

        queue.enqueueNDRangeKernel(dcontext->pairs, cl::NullRange, cl::NDRange(model->pair_pool_size), cl::NullRange);
        queue.finish();
        queue.enqueueNDRangeKernel(dcontext->receptors, cl::NullRange, cl::NDRange(model->receptor_pool_size), cl::NullRange);
        queue.finish();
        queue.enqueueNDRangeKernel(dcontext->neurons, cl::NullRange, cl::NDRange(model->neuron_pool_size), cl::NullRange);
        queue.finish();

        queue.enqueueReadBuffer(times_buffer, CL_TRUE, 0, sizeof(cl_int)*model->neuron_pool_size, model->Times);

        for (i = 0; i < model->neuron_pool_size; i++) {
            if (model->Times[i] != model->batch_size) {
                done = false;
                break;
            }
//            std::cout << "[" << model->t << "] " << model->objects[i]->getName() << " " << model->Times[i] << std::endl;
        }

        queue.enqueueReadBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size, model->Outputs);
        for (i = 0; i < model->neuron_pool_size; i++) {
//            std::cout << "[" << model->Times[i] << "] " << model->objects[i]->getName() << " " << model->Outputs[i] << std::endl;
            if (model->Outputs[i] >= 0) model -> output_sequence[i].emplace_back(model->Outputs[i]);
        }
    }

//    queue.enqueueReadBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size, model->Outputs);
    queue.enqueueReadBuffer(pairs_buffer, CL_TRUE, 0, sizeof(cl_float16)*model->pair_pool_size, model->PairsInfo);
    queue.enqueueReadBuffer(receptors_buffer, CL_TRUE, 0, sizeof(cl_float8)*model->receptor_pool_size, model->ReceptorsInfo);
#endif
}

void indk::ComputeBackends::OpenCL::doReset(void *model) {
    indk::Translators::CL::doReset((indk::Translators::CL::ModelData*)model);
}

void indk::ComputeBackends::OpenCL::doClear(void *model) {
    indk::Translators::CL::doClear((indk::Translators::CL::ModelData*)model);
}

void indk::ComputeBackends::OpenCL::setMode(void *_model, bool learning) {
    auto model = (indk::Translators::CL::ModelData*)_model;
    model -> learning_mode = learning;
}

void indk::ComputeBackends::OpenCL::setParameters(indk::ComputeBackend::Parameters *parameters) {
    CurrentDeviceName = ((Parameters*)parameters) -> device_name;
}

std::vector<indk::OutputValue> indk::ComputeBackends::OpenCL::getOutputValues(void *_model) {
    auto model = (indk::Translators::CL::ModelData*)_model;

    std::vector<indk::OutputValue> outputs;
    outputs.reserve(model->outputs.size());
#ifdef INDK_OPENCL_SUPPORT
    for (const auto &o: model->outputs) {
        auto found = std::find_if(model->objects.begin(), model->objects.end(), [o](indk::Neuron *n){ return n->getName() == o; });
        if (found != model->objects.end()) {
            auto index = std::distance(model->objects.begin(), found);
            auto value = model->output_sequence[index][model->batch_size-1];
            indk::OutputValue output = {.value=value, .source=o, .time=model->batch_size};
            outputs.emplace_back(output);
        }
    }
#endif
    return outputs;
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeBackends::OpenCL::getReceptorPositions(void *_model) {
    auto model = (indk::Translators::CL::ModelData*)_model;

    std::map<std::string, std::vector<indk::Position>> list;
#ifdef INDK_OPENCL_SUPPORT
    uint64_t ri = 0;
    for (uint64_t i = 0; i < model->objects.size(); i++) {
        auto n = model->objects[i];
        std::vector<indk::Position> positions;

        for (uint64_t r = 0; r < n->getReceptorsCount(); r++) {
            std::vector<float> coords;

            coords.emplace_back(model->PairsInfo[(int)model->ReceptorsInfo[ri].s0].s0);
            coords.emplace_back(model->PairsInfo[(int)model->ReceptorsInfo[ri].s0].s1);

            while (coords.size() < n->getDimensionsCount()) coords.emplace_back(0);

            positions.emplace_back(n->getXm(), coords);
            ri++;
        }
        list.insert(std::make_pair(n->getName(), positions));
    }
#endif
    return list;
}

std::vector<indk::ComputeBackends::OpenCL::DeviceInfo> indk::ComputeBackends::OpenCL::getDeviceInfoList() {
    std::vector<DeviceInfo> info;
    for (auto &d: DeviceList) {
        DeviceInfo di;
        di.platform_name = d.second->platform_name;
        di.device_name = d.second->device_name;
        di.compute_units = d.second->compute_units;
        di.workgroup_size = d.second->workgroup_size;
        info.emplace_back(di);
    }
    return info;
}

std::vector<indk::ComputeBackends::OpenCL::DeviceInfo> indk::ComputeBackends::OpenCL::getDevicesInfo() {
    if (handler) return handler->getDeviceInfoList();
    return {};
}

indk::ComputeBackends::OpenCL::~OpenCL() {
    for (auto &d: DeviceList) {
        delete d.second;
    }
    DeviceList.clear();
}
