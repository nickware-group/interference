/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include <indk/translators/cl.h>
#include <cstring>
#include <algorithm>

void* indk::Translators::CL::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap& sync) {
    auto model = new ModelData;

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

#ifdef INDK_OPENCL_SUPPORT
    model -> PairsInfo = new cl_float16[model->pair_pool_size];
    model -> ReceptorsInfo = new cl_float8[model->receptor_pool_size];
    model -> NeuronsInfo = new cl_float3[model->neuron_pool_size];
    model -> Inputs = new cl_float2[model->input_pool_size];
    model -> Outputs = new cl_float[model->neuron_pool_size];
    memset(model->Outputs, 0, sizeof(cl_float)*model->neuron_pool_size);

    indk::Position *rpos, *spos;
    uint64_t px = 0, pxstart;
    uint64_t rx = 0, rxstart;
    uint64_t ex = 0, exstart;

    for (uint64_t ni = 0; ni < neurons.size(); ni++) {
        auto n = (indk::Neuron*)neurons[ni];
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

                if (!i) {
                    auto found = std::find_if(neurons.begin(), neurons.end(), [elist, j](indk::Neuron *n){ return n->getName()==elist[j]; });
                    if (found != neurons.end()) {
                        auto nei = std::distance(neurons.begin(), found);
                        model -> Inputs[ex] = {
                            static_cast<cl_float>(2),
                            static_cast<cl_float>(nei),
                        };
                    } else {
                        model -> Inputs[ex] = {
                            static_cast<cl_float>(0),
                            static_cast<cl_float>(0),
                        };
                    }
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
            static_cast<cl_float>(exstart),
        };
    }
#endif

    return model;
}

void indk::Translators::CL::doReset(indk::Translators::CL::ModelData *model) {
//    for (const auto &o: model->objects) {
//        auto neuron = o.second;
//        neuron -> t = 0;
//
//        for (int i = 0; i < neuron->receptor_count; i++) {
//            auto r = o.second -> receptors[i];
//            r.fi = 0;
//            r.l = 0;
//            r.rs = r.rs_default;
//            std::memcpy(r.position, r.position_default, sizeof(float)*o.second->dimension_count);
//        }
//        for (int j = 0; j < neuron->entry_count; j++) {
//            auto e = o.second->entries[j];
//            for (unsigned int k = 0; k < e.synapse_count; k++) {
//                e.synapses[k].gamma = 0;
//                e.synapses[k].d_gamma = 0;
//                e.synapses[k].l_gamma = 0;
//                e.synapses[k].l_d_gamma = 0;
//            }
//        }
//    }
}

std::string indk::Translators::CL::getTranslatorName() {
    return "CL";
}
