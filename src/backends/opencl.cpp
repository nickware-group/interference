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
            std::cout << "Device: " << d.getInfo<CL_DEVICE_NAME>() << std::endl;
            DeviceContext device;
            device.ready = false;

            DeviceList.emplace_back(device);
//            DeviceList.back().device = d;
//            DeviceList.back().ready = false;
        }
    }

    Context = cl::Context(CL_DEVICE_TYPE_ALL);

    if (!DeviceList.empty()) Ready = true;
#endif
}

void indk::ComputeBackends::OpenCL::doInitDevice() {
    auto dcontext = DeviceList[0];
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No platforms found. Check OpenCL installation!" << std::endl;
        return;
    }
    cl::Platform platform = platforms[2];
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty()) {
        std::cerr << "No devices found. Check OpenCL installation!" << std::endl;;
        return;
    }

//    for (auto &p: platforms) {
//        std::cout << "Platform: " << p.getInfo<CL_PLATFORM_NAME>() << std::endl;
//        bool found = false;
//
//        p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
//        for (int i = 0; i < devices.size(); i++) {
//            auto name = devices[i].getInfo<CL_DEVICE_NAME>();
//            std::cout << "Device: " << devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
//            if (CurrentDeviceName == name) {
////                device = devices[i];
//                found = true;
//                break;
//            }
//        }
//        if (found) break;
//    }

    cl::Device device = devices[0];

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

    std::cout << "Using device  : " << device.getInfo<CL_DEVICE_NAME>() << " (CU: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << ")" << std::endl;
    std::cout << "Driver version: " << device.getInfo<CL_DRIVER_VERSION>() << std::endl;

    auto pairs = cl::Program(Context, {kernel_code_pairs.c_str(),kernel_code_pairs.length()+1});
    if ((status = pairs.build({device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << pairs.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;

    }

    auto receptors = cl::Program(Context, {kernel_code_receptors.c_str(),kernel_code_receptors.length()+1});
    if ((status = receptors.build({device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << receptors.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;

    }

    auto neurons = cl::Program(Context, {kernel_code_neurons.c_str(),kernel_code_neurons.length()+1});
    if ((status = neurons.build({device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << neurons.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;

    }

    dcontext.pairs = cl::Kernel(pairs, "indk_kernel_pairs");
    dcontext.receptors = cl::Kernel(receptors, "indk_kernel_receptors");
    dcontext.neurons = cl::Kernel(neurons, "indk_kernel_neurons");

    dcontext.ready = true;
}

void* indk::ComputeBackends::OpenCL::doTranslate(const indk::LinkList &links, const std::vector<std::string> &outputs, const indk::StateSyncMap &sync) {
    return indk::Translators::CL::doTranslate(links, outputs, sync);
}

void indk::ComputeBackends::OpenCL::doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_model) {
    auto model = (indk::Translators::CL::ModelData*)_model;

#ifdef INDK_OPENCL_SUPPORT
    auto dcontext = DeviceList[0];

    if (!dcontext.ready) doInitDevice();

    std::cout << "Ready" << std::endl;

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

//    auto Queue = cl::CommandQueue(Context, dcontext.device);
//    Queue.enqueueWriteBuffer(pairs_buffer, CL_TRUE, 0, sizeof(cl_float16)*model->pair_pool_size, model->PairsInfo);
//    Queue.enqueueWriteBuffer(receptors_buffer, CL_TRUE, 0, sizeof(cl_float8)*model->receptor_pool_size, model->ReceptorsInfo);
//    Queue.enqueueWriteBuffer(neurons_buffer, CL_TRUE, 0, sizeof(cl_float3)*model->neuron_pool_size, model->NeuronsInfo);
//    Queue.enqueueWriteBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size,  model->Outputs);

    uint64_t xi = 0;
    for (const auto &o: model->objects) {
        auto n = (indk::Neuron*)o;
//        auto state = n->getState(n->getTime());
//        auto ec = n -> getEntriesCount();
//
//        for (int j = 0; j < ec; j++) {
//            auto e = n -> getEntry(j);
//            if (state == indk::Neuron::States::Pending) {
//                Inputs[xi] = {
//                        static_cast<cl_float>(1),
//                        static_cast<cl_float>(e->getIn()),
//                };
//            } else {
//                Inputs[xi] = {
//                        static_cast<cl_float>(0),
//                        static_cast<cl_float>(0),
//                };
//            }
//            xi++;
//        }
    }

//    Queue.enqueueWriteBuffer(inputs_buffer, CL_TRUE, 0, sizeof(cl_float2)*model->input_pool_size, model->Inputs);
//
//    Queue.enqueueNDRangeKernel(dcontext.pairs, cl::NullRange, cl::NDRange(model->pair_pool_size), cl::NullRange);
//    Queue.finish();
//    Queue.enqueueNDRangeKernel(dcontext.receptors, cl::NullRange, cl::NDRange(model->receptor_pool_size), cl::NullRange);
//    Queue.finish();
//    Queue.enqueueNDRangeKernel(dcontext.neurons, cl::NullRange, cl::NDRange(model->neuron_pool_size), cl::NullRange);
//    Queue.finish();
//
//    Queue.enqueueReadBuffer(outputs_buffer, CL_TRUE, 0, sizeof(cl_float)*model->neuron_pool_size, model->Outputs);

//    for (int ni = 0; ni < Objects.size(); ni++) {
//        if (Inputs[(int)NeuronsInfo[ni].s2].s0 == 0) {
//            continue;
//        }
//        auto n = (indk::Neuron*)Objects[ni];
//        n -> doFinalizeInput(Outputs[ni]);
//    }
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
//    outputs.reserve(model->outputs.size());
//
//    for (const auto &o: model->outputs) {
//        auto value = o->t == 0 ? 0 : o->output[o->t-1];
//        indk::OutputValue output = {.value=value, .source=o->name, .time=o->t};
//        outputs.emplace_back(output);
//    }

    return outputs;
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeBackends::OpenCL::getReceptorPositions(void *_model) {
    auto model = (indk::Translators::CL::ModelData*)_model;

    std::map<std::string, std::vector<indk::Position>> list;

//    for (auto &n: model->objects) {
//        std::vector<indk::Position> positions;
//        for (uint64_t r = 0; r < n.second->receptor_count; r++) {
//            positions.emplace_back(n.second->size, std::vector<float>(n.second->receptors[r].position, n.second->receptors[r].position+n.second->dimension_count));
//        }
//        list.insert(std::make_pair(n.second->name, positions));
//    }
    return list;
}
