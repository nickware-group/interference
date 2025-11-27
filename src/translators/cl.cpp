/////////////////////////////////////////////////////////////////////////////
// Name:        translators/cl.cpp
// Purpose:     CL translator class
// Author:      Nickolay Babich
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/translators/cl.h>
#include <cstring>
#include <algorithm>

void* indk::Translators::CL::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync) {
    auto model = new ModelData;

#ifdef INDK_OPENCL_SUPPORT
    model -> objects = neurons;
    model -> outputs = outputs;
    model -> pair_pool_size = 0;
    model -> receptor_pool_size = 0;
    model -> input_pool_size = 0;
    model -> neuron_pool_size = neurons.size();

    for (const auto &n: neurons) {
        model -> pair_pool_size += n->getReceptorsCount() * n->getSynapsesCount();
        model -> receptor_pool_size += n->getReceptorsCount();
        model -> input_pool_size += n->getEntriesCount();
    }

    model -> PairsInfo = new cl_float16[model->pair_pool_size];
    model -> ReceptorsInfo = new cl_float8[model->receptor_pool_size];
    model -> NeuronsInfo = new cl_float3[model->neuron_pool_size];
    model -> Inputs = new cl_float2[model->input_pool_size];
    model -> Outputs = new cl_float[model->neuron_pool_size];
    model -> Times = new cl_int[model->neuron_pool_size];
    memset(model->Outputs, 0, sizeof(cl_float)*model->neuron_pool_size);

    doReset(model);
#endif

    return model;
}

void indk::Translators::CL::doReset(indk::Translators::CL::ModelData *model) {
    memset(model->Outputs, 0, sizeof(cl_float)*model->neuron_pool_size);

#ifdef INDK_OPENCL_SUPPORT
    indk::Position *rpos, *spos;
    uint64_t px = 0, pxstart;
    uint64_t rx = 0, rxstart;
    uint64_t ex = 0, exstart;

    for (uint64_t ni = 0; ni < model->objects.size(); ni++) {
        auto n = model->objects[ni];
        auto elist = n -> getEntries();

        rxstart = rx;
        exstart = ex;

        for (int i = 0; i < n->getReceptorsCount(); i++) {
            pxstart = px;

            auto r = n -> getReceptor(i);
            rpos = r -> getPos0();

            ex = exstart;
            for (int j = 0; j < n->getEntriesCount(); j++) {
                auto e = n -> getEntry(j);

                for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
                    auto s = e -> getSynapse(k);
                    spos = s -> getPos();

                    model -> PairsInfo[px] = {
                            static_cast<cl_float>(rpos->getPositionValue(0)),
                            static_cast<cl_float>(rpos->getPositionValue(1)),
                            static_cast<cl_float>(spos->getPositionValue(0)),
                            static_cast<cl_float>(spos->getPositionValue(1)),
                            static_cast<cl_float>(s->getGamma()),
                            static_cast<cl_float>(ex),

                            static_cast<cl_float>(s->getLambda()),
                            static_cast<cl_float>(s->getk1()),
                            static_cast<cl_float>(s->getk2()),
                    };
                    px++;
                }
                ex++;
            }
            model -> ReceptorsInfo[rx] = {
                    static_cast<cl_float>(pxstart),
                    static_cast<cl_float>(px),
                    static_cast<cl_float>(exstart),
                    static_cast<cl_float>(r->getRs()),
                    static_cast<cl_float>(r->getFi()),
                    static_cast<cl_float>(r->getk3()),
            };
            rx++;
        }
        model -> NeuronsInfo[ni] = {
                static_cast<cl_float>(rxstart),
                static_cast<cl_float>(rx),
                static_cast<cl_float>(exstart)
        };

        model -> output_sequence.clear();
    }
#endif
}

void indk::Translators::CL::doClear(indk::Translators::CL::ModelData *model) {
    delete [] model->PairsInfo;
    delete [] model->ReceptorsInfo;
    delete [] model->NeuronsInfo;
    delete [] model->Inputs;
    delete [] model->Outputs;
    delete [] model->Times;
    delete model;
}

std::string indk::Translators::CL::getTranslatorName() {
    return "CL";
}
