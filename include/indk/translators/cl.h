/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_TRANSLATOR_CL_H
#define INTERFERENCE_TRANSLATOR_CL_H

#include <cstdint>
#include <string>
#include <atomic>
#include <indk/neuron.h>
#include <indk/types.h>

#ifdef INDK_OPENCL_SUPPORT
    #include <CL/opencl.hpp>
#endif

namespace indk {
    namespace Translators {
        class CL {
        public:
            typedef struct cl_model_data {
#ifdef INDK_OPENCL_SUPPORT
                    cl::CommandQueue Queue;

                    cl_float16 *PairsInfo;
                    cl_float8 *ReceptorsInfo;
                    cl_float3 *NeuronsInfo;
                    cl_float2 *Inputs;
                    cl_float *Outputs;
                    cl_int *Times;
#endif
                uint64_t pair_pool_size;
                uint64_t receptor_pool_size;
                uint64_t neuron_pool_size;
                uint64_t input_pool_size;
                uint64_t batch_size;

                std::vector<indk::Neuron*> objects;
                std::vector<std::string> outputs;
                std::vector<std::vector<float>> output_sequence;
                bool learning_mode;
            } ModelData;

            static void* doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync);
            static void doReset(ModelData *model);

            static std::string getTranslatorName();
        };
    }
}

#endif //INTERFERENCE_TRANSLATOR_CL_H
