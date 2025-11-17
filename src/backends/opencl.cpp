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

indk::ComputeBackends::OpenCL::OpenCL() {
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
        std::cout << "Platform: " << p.getInfo<CL_PLATFORM_NAME>() << std::endl;

        std::vector<cl::Device> devices;
        p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for (auto &d: devices) {
            auto name = d.getInfo<CL_DEVICE_NAME>();
            std::cout << "Device: " << name << std::endl;
            if (CurrentDeviceName.empty()) CurrentDeviceName = name;
            DeviceContext device;
            device.ready = false;
            device.device = d;
            DeviceList.insert(std::make_pair(name, device));
//            DeviceList.back().device = d;
//            DeviceList.back().ready = false;
        }
    }

    Context = cl::Context(CL_DEVICE_TYPE_ALL);

    if (!DeviceList.empty()) Ready = true;
#endif
}

indk::ComputeBackends::OpenCL::DeviceContext indk::ComputeBackends::OpenCL::doInitCurrentDevice() {
    if (!Ready) return {};

#ifdef INDK_OPENCL_SUPPORT
    auto found = DeviceList.find(CurrentDeviceName);

    if (found == DeviceList.end()) {
        return {}; // TODO: must be exception
    }

    auto dcontext = found->second;

    if (dcontext.ready) return dcontext;

    KERNEL(kernel_code_pairs,
           __kernel void indk_kernel_pairs(__global float16 *pairs, __global float2 *inputs) {
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
                   bool run = inputs[(int)pairs[id].s5].s0;
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
           }
    );

    KERNEL(kernel_code_receptors,
           __kernel void indk_kernel_receptors(__global float8 *receptors,  __global float16 *pairs, __global float2 *inputs) {
                   int id = get_global_id(0);
                   // receptors.s0 - left pairs range edge           (const)
                   // receptors.s1 - right pairs range edge          (const)
                   // receptors.s2 - input index                     (const)

                   // receptors.s3 - receptor sensitivity
                   // receptors.s4 - neurotransmitter level value

                   // receptors.s5 - k3                              (const)

                   // receptors.s6 - reserved

                   bool run = inputs[(int)receptors[id].s2].s0;
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
           }
    );

    KERNEL(kernel_code_neurons,
           __kernel void indk_kernel_neurons(__global float3 *neurons,  __global float8 *receptors, __global float2 *inputs, __global float *outputs) {
                   int id = get_global_id(0);
                   // neurons.s0 - left receptors range edge           (const)
                   // neurons.s1 - right receptors range edge          (const)
                   // neurons.s2 - input index                         (const)

                   bool run = inputs[(int)neurons[id].s2].s0;
                   if (run) {
                       float p = 0;
                       int rcount = neurons[id].s1 - neurons[id].s0;

                       for (int i = neurons[id].s0; i < neurons[id].s1; i++) {
                           p += receptors[i].s6;
                       }
                       p /= (float)rcount;

                       outputs[id] = p;
                   };
           }
    );

    int status;

    auto pairs = cl::Program(Context, {kernel_code_pairs.c_str(),kernel_code_pairs.length()+1});
    if ((status = pairs.build({dcontext.device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << pairs.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dcontext.device) << std::endl;

    }

    auto receptors = cl::Program(Context, {kernel_code_receptors.c_str(),kernel_code_receptors.length()+1});
    if ((status = receptors.build({dcontext.device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << receptors.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dcontext.device) << std::endl;

    }

    auto neurons = cl::Program(Context, {kernel_code_neurons.c_str(),kernel_code_neurons.length()+1});
    if ((status = neurons.build({dcontext.device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << neurons.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dcontext.device) << std::endl;

    }

    dcontext.pairs = cl::Kernel(pairs, "indk_kernel_pairs");
    dcontext.receptors = cl::Kernel(receptors, "indk_kernel_receptors");
    dcontext.neurons = cl::Kernel(neurons, "indk_kernel_neurons");

    dcontext.ready = true;

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

    dcontext.pairs.setArg(0, pairs_buffer);
    dcontext.pairs.setArg(1, inputs_buffer);

    dcontext.receptors.setArg(0, receptors_buffer);
    dcontext.receptors.setArg(1, pairs_buffer);
    dcontext.receptors.setArg(2, inputs_buffer);

    dcontext.neurons.setArg(0, neurons_buffer);
    dcontext.neurons.setArg(1, receptors_buffer);
    dcontext.neurons.setArg(2, inputs_buffer);
    dcontext.neurons.setArg(3, outputs_buffer);

    auto queue = cl::CommandQueue(Context, dcontext.device);
    queue.enqueueWriteBuffer(pairs_buffer, CL_TRUE, 0, sizeof(cl_float16)*model->pair_pool_size, model->PairsInfo);
    queue.enqueueWriteBuffer(receptors_buffer, CL_TRUE, 0, sizeof(cl_float8)*model->receptor_pool_size, model->ReceptorsInfo);
    queue.enqueueWriteBuffer(neurons_buffer, CL_TRUE, 0, sizeof(cl_float3)*model->neuron_pool_size, model->NeuronsInfo);
    queue.enqueueWriteBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size,  model->Outputs);

    model->t = 0;

    while (model->t < x[0].size()) {
        uint64_t xi = 0;
        for (const auto &o: model->objects) {
            auto n = (indk::Neuron*)o;
            auto elist = n -> getEntries();

            for (const auto &e: elist) {
                auto found = std::find(inputs.begin(), inputs.end(), e);
                if (found != inputs.end()) {
                    auto index = std::distance(inputs.begin(), found);
                    model->Inputs[xi] = {
                            static_cast<cl_float>(1),
                            static_cast<cl_float>(x[index][model->t]),
                    };
                } else {
                    model->Inputs[xi] = {
                            static_cast<cl_float>(0),
                            static_cast<cl_float>(0),
                    };
                }
                xi++;
            }
        }

        queue.enqueueWriteBuffer(inputs_buffer, CL_TRUE, 0, sizeof(cl_float2)*model->input_pool_size, model->Inputs);

        queue.enqueueNDRangeKernel(dcontext.pairs, cl::NullRange, cl::NDRange(model->pair_pool_size), cl::NullRange);
        queue.finish();
        queue.enqueueNDRangeKernel(dcontext.receptors, cl::NullRange, cl::NDRange(model->receptor_pool_size), cl::NullRange);
        queue.finish();
        queue.enqueueNDRangeKernel(dcontext.neurons, cl::NullRange, cl::NDRange(model->neuron_pool_size), cl::NullRange);
        queue.finish();

        model->t++;
    }

    queue.enqueueReadBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size, model->Outputs);

    queue.enqueueReadBuffer(pairs_buffer, CL_TRUE, 0, sizeof(cl_float16)*model->pair_pool_size, model->PairsInfo);
    queue.enqueueReadBuffer(receptors_buffer, CL_TRUE, 0, sizeof(cl_float8)*model->receptor_pool_size, model->ReceptorsInfo);
#endif
}

void indk::ComputeBackends::OpenCL::doReset(void *model) {
    indk::Translators::CL::doReset((indk::Translators::CL::ModelData*)model);
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

    for (const auto &o: model->outputs) {
        auto found = std::find_if(model->objects.begin(), model->objects.end(), [o](indk::Neuron *n){ return n->getName() == o; });
        if (found != model->objects.end()) {
            auto index = std::distance(model->objects.begin(), found);
//            std::cout << o << " " << model->Outputs[index] << std::endl;
            auto value = model->t == 0 ? 0 : model->Outputs[index];
            indk::OutputValue output = {.value=value, .source=o, .time=model->t};
            outputs.emplace_back(output);
        }
    }

    return outputs;
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeBackends::OpenCL::getReceptorPositions(void *_model) {
    auto model = (indk::Translators::CL::ModelData*)_model;

    std::map<std::string, std::vector<indk::Position>> list;

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

    return list;
}
