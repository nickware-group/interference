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
    #include <CL/cl.hpp>
#endif

namespace indk {
    namespace Translators {
        class CL {
        public:
            // synapse imprint
            typedef struct synapse_params {
                float k1, k2, lambda, gamma, d_gamma;
                float l_gamma, l_d_gamma;
                int64_t tl;
                int neurotransmitter_type;
                float *position;
            } SynapseParams;

            // receptor imprint
            typedef struct receptor_params {
                float k3;
                float rs, l, fi, d_fi;
                float *position;
                float rs_default;
                float *position_default;
            } ReceptorParams;

            // entry imprint
            typedef struct entry_params {
                void *input;
                int entry_type;
                uint64_t synapse_count;
                SynapseParams *synapses;
                std::string input_from;
            } EntryParams;

            // neuron imprint
            typedef struct neutron_params {
                std::atomic<uint64_t> t;
                std::string name;
                uint32_t size;
                uint64_t latency;
                uint64_t dimension_count, receptor_count, entry_count;
                EntryParams *entries;
                ReceptorParams *receptors;
                float *position_buffer;
                float *output;
            } NeuronParams;

            typedef struct cl_model_data {
#ifdef INDK_OPENCL_SUPPORT
                    cl::CommandQueue Queue;

                    cl_float16 *PairsInfo;
                    cl_float8 *ReceptorsInfo;
                    cl_float3 *NeuronsInfo;
                    cl_float2 *Inputs;
                    cl_float *Outputs;

//                    cl::Buffer PairsBuffer;
//                    cl::Buffer ReceptorsBuffer;
//                    cl::Buffer NeuronsBuffer;
//                    cl::Buffer InputsBuffer;
//                    cl::Buffer OutputsBuffer;
#endif
                uint64_t pair_pool_size;
                uint64_t receptor_pool_size;
                uint64_t neuron_pool_size;
                uint64_t input_pool_size;
                uint64_t t;
                std::vector<indk::Neuron*> objects;
                std::vector<std::string> outputs;
                bool learning_mode;
            } ModelData;

            static void* doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync);
            static void doReset(ModelData *model);

            static std::string getTranslatorName();
        };
    }
}

#endif //INTERFERENCE_TRANSLATOR_CL_H
