/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_TRANSLATOR_CPU_H
#define INTERFERENCE_TRANSLATOR_CPU_H

#include <cstdint>
#include <string>
#include <atomic>
#include <indk/neuron.h>
#include <indk/types.h>

namespace indk {
    namespace Translators {
        class CPU {
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

            typedef struct cpu_model_data {
                std::map<std::string, NeuronParams*> objects;
                std::vector<NeuronParams*> outputs;
                indk::StateSyncMap sync_map;
                uint64_t batch_size;
                bool learning_mode;
            } ModelData;

            static NeuronParams* doTranslateNeuronToInstance(indk::Neuron *neuron, std::map<std::string, NeuronParams*>& objects);

            static void* doTranslate(const std::vector<indk::Neuron*> &neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync);
            static void doReset(ModelData *model);

            static std::string getTranslatorName();
        };
    }
}

#endif //INTERFERENCE_TRANSLATOR_CPU_H
