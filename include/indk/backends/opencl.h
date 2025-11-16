/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_BACKENDS_OPENCL_H
#define INTERFERENCE_BACKENDS_OPENCL_H

#include <indk/backend.h>
#include <indk/position.h>
#include <indk/translators/cl.h>

namespace indk {
    namespace ComputeBackends {
        class OpenCL : public indk::ComputeBackend {
        private:
            std::string CurrentDeviceName;
#ifdef INDK_OPENCL_SUPPORT
            typedef struct device_context {
                cl::Device device;
                cl::Kernel pairs;
                cl::Kernel receptors;
                cl::Kernel neurons;
                bool ready;
            } DeviceContext;

            cl::Context Context;
            std::map<std::string, DeviceContext> DeviceList;
#endif
        public:
            typedef struct Parameters : public indk::ComputeBackend::Parameters {
                std::string device_name;
            } Parameters;

            OpenCL();
            indk::ComputeBackends::OpenCL::DeviceContext doInitCurrentDevice();
            void* doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync) override;
            void doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_instance) override;
            void doReset(void*) override;
            void setMode(void *model, bool learning) override;
            void setParameters(indk::ComputeBackend::Parameters*) override;
            std::vector<indk::OutputValue> getOutputValues(void *_model) override;
            std::map<std::string, std::vector<indk::Position>> getReceptorPositions(void *_model) override;
        };
    }
}

#endif //INTERFERENCE_BACKENDS_OPENCL_H
