/////////////////////////////////////////////////////////////////////////////
// Name:        indk/translators/vk.h
// Purpose:     Vulkan translator class header
// Author:      Nickolay Babich
// Created:     01.02.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_TRANSLATOR_VK_H
#define INTERFERENCE_TRANSLATOR_VK_H

#include <cstdint>
#include <string>
#include <atomic>
#include <vector>
#include <indk/neuron.h>
#include <indk/types.h>

namespace indk {
    namespace Translators {
        class VK {
        public:
            typedef struct vk_model_data {
                // Data arrays for Vulkan buffers
                // PairsInfo: 16 floats per pair (receptor-synapse)
                // [0] receptor x, [1] receptor y, [2] synapse x, [3] synapse y
                // [4] gamma, [5] input index, [6] lambda, [7] k1, [8] k2
                // [9-11] reserved for output (nx, ny, fi)
                std::vector<float> PairsInfo;

                // ReceptorsInfo: 8 floats per receptor
                // [0] left pairs range, [1] right pairs range, [2] input index
                // [3] sensitivity (rs), [4] neurotransmitter level (fi), [5] k3
                // [6] reserved for output (p), [7] reserved
                std::vector<float> ReceptorsInfo;

                // NeuronsInfo: 4 floats per neuron
                // [0] left receptors range, [1] right receptors range
                // [2] input index, [3] latency
                std::vector<float> NeuronsInfo;

                // Inputs: 2 floats per input
                // [0] run flag, [1] input value
                std::vector<float> Inputs;

                // Outputs: 1 float per neuron
                std::vector<float> Outputs;

                // Times: 1 int per neuron
                std::vector<int32_t> Times;

                // Pool sizes
                uint64_t pair_pool_size;
                uint64_t receptor_pool_size;
                uint64_t neuron_pool_size;
                uint64_t input_pool_size;
                uint64_t batch_size;

                // Objects and outputs
                std::vector<indk::Neuron*> objects;
                std::vector<std::string> outputs;
                std::vector<std::vector<float>> output_sequence;
                bool learning_mode;
            } ModelData;

            static void* doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync);
            static void doReset(ModelData *model);
            static void doClear(ModelData *model);

            static std::string getTranslatorName();
        };
    }
}

#endif //INTERFERENCE_TRANSLATOR_VK_H
