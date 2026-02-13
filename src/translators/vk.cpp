/////////////////////////////////////////////////////////////////////////////
// Name:        translators/vk.cpp
// Purpose:     Vulkan translator class
// Author:      Nickolay Babich
// Created:     01.02.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/translators/vk.h>
#include <cstring>
#include <algorithm>

void* indk::Translators::VK::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync) {
    auto model = new ModelData;

    model->objects = neurons;
    model->outputs = outputs;
    model->pair_pool_size = 0;
    model->receptor_pool_size = 0;
    model->input_pool_size = 0;
    model->neuron_pool_size = neurons.size();
    model->learning_mode = true;

    // Calculate pool sizes
    for (const auto &n: neurons) {
        model->pair_pool_size += n->getReceptorsCount() * n->getSynapsesCount();
        model->receptor_pool_size += n->getReceptorsCount();
        model->input_pool_size += n->getEntriesCount();
    }

    // Allocate arrays (16 floats per pair)
    model->PairsInfo.resize(model->pair_pool_size * 16, 0.0f);
    // 8 floats per receptor
    model->ReceptorsInfo.resize(model->receptor_pool_size * 8, 0.0f);
    // 4 floats per neuron
    model->NeuronsInfo.resize(model->neuron_pool_size * 4, 0.0f);
    // 2 floats per input
    model->Inputs.resize(model->input_pool_size * 2, 0.0f);
    // 1 float per neuron output
    model->Outputs.resize(model->neuron_pool_size, 0.0f);
    // 1 int per neuron time
    model->Times.resize(model->neuron_pool_size, 0);

    doReset(model);

    return model;
}

void indk::Translators::VK::doReset(indk::Translators::VK::ModelData *model) {
    std::fill(model->Outputs.begin(), model->Outputs.end(), 0.0f);

    indk::Position *rpos, *spos;
    uint64_t px = 0, pxstart;
    uint64_t rx = 0, rxstart;
    uint64_t ex = 0, exstart;

    for (uint64_t ni = 0; ni < model->objects.size(); ni++) {
        auto n = model->objects[ni];
        auto elist = n->getEntries();

        rxstart = rx;
        exstart = ex;

        for (int i = 0; i < n->getReceptorsCount(); i++) {
            pxstart = px;

            auto r = n->getReceptor(i);
            rpos = r->getPos0();

            ex = exstart;
            for (int j = 0; j < n->getEntriesCount(); j++) {
                auto e = n->getEntry(j);

                for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
                    auto s = e->getSynapse(k);
                    spos = s->getPos();

                    // PairsInfo layout: 16 floats per pair
                    uint64_t base = px * 16;
                    model->PairsInfo[base + 0] = rpos->getPositionValue(0);  // receptor x
                    model->PairsInfo[base + 1] = rpos->getPositionValue(1);  // receptor y
                    model->PairsInfo[base + 2] = spos->getPositionValue(0);  // synapse x
                    model->PairsInfo[base + 3] = spos->getPositionValue(1);  // synapse y
                    model->PairsInfo[base + 4] = s->getGamma();              // gamma
                    model->PairsInfo[base + 5] = static_cast<float>(ex);     // input index
                    model->PairsInfo[base + 6] = s->getLambda();             // lambda
                    model->PairsInfo[base + 7] = s->getk1();                 // k1
                    model->PairsInfo[base + 8] = s->getk2();                 // k2
                    // [9-15] reserved for output values

                    px++;
                }
                ex++;
            }

            // ReceptorsInfo layout: 8 floats per receptor
            uint64_t rbase = rx * 8;
            model->ReceptorsInfo[rbase + 0] = static_cast<float>(pxstart);  // left pairs range
            model->ReceptorsInfo[rbase + 1] = static_cast<float>(px);       // right pairs range
            model->ReceptorsInfo[rbase + 2] = static_cast<float>(exstart);  // input index
            model->ReceptorsInfo[rbase + 3] = r->getRs();                   // sensitivity
            model->ReceptorsInfo[rbase + 4] = r->getFi();                   // fi
            model->ReceptorsInfo[rbase + 5] = r->getk3();                   // k3
            // [6-7] reserved

            rx++;
        }

        // NeuronsInfo layout: 4 floats per neuron
        uint64_t nbase = ni * 4;
        model->NeuronsInfo[nbase + 0] = static_cast<float>(rxstart);  // left receptors range
        model->NeuronsInfo[nbase + 1] = static_cast<float>(rx);       // right receptors range
        model->NeuronsInfo[nbase + 2] = static_cast<float>(exstart);  // input index
        model->NeuronsInfo[nbase + 3] = static_cast<float>(n->getLatency()); // latency

        model->output_sequence.clear();
    }
}

void indk::Translators::VK::doClear(indk::Translators::VK::ModelData *model) {
    model->PairsInfo.clear();
    model->ReceptorsInfo.clear();
    model->NeuronsInfo.clear();
    model->Inputs.clear();
    model->Outputs.clear();
    model->Times.clear();
    model->output_sequence.clear();
    delete model;
}

std::string indk::Translators::VK::getTranslatorName() {
    return "VK";
}
