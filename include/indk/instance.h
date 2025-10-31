/////////////////////////////////////////////////////////////////////////////
// Name:        instance.cpp
// Purpose:     Compute Instance Manager class
// Author:      Nickolay Babbysh
// Created:     21.10.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_INSTANCE_H
#define INTERFERENCE_INSTANCE_H

#include <vector>
#include <atomic>
#include <indk/system.h>
#include <indk/error.h>
#include <indk/system.h>
#include <map>
#include "neuron.h"

namespace indk {
    /// Compute Instance Manager class.
    class ComputeInstanceManager {
    public:
        typedef enum {InstanceReady, InstanceBusy} InstanceStatus;
        typedef enum {InstanceRunAuto = -1} InstanceRunMode;

    private:
        // synapse imprint [in, k1, k2, Lambda, Tl, Gamma, dGamma, NeurotransmitterType]
        typedef struct synapse_params {
            float k1, k2, lambda, gamma, d_gamma;
            float l_gamma, l_d_gamma;
            int64_t tl;
            int neurotransmitter_type;
            float *position;
        } SynapseParams;

        // receptor imprint [k3, Rs, L, Fi, dFi]
        typedef struct receptor_params {
            float k3, rs, l, fi, d_fi;
            float *position;
        } ReceptorParams;

        typedef struct entry_params {
            void *input;
            int entry_type;
            uint64_t synapse_count;
            SynapseParams *synapses;
            std::string input_from;
        } EntryParams;

        typedef struct neutron_params {
            std::atomic<uint64_t> t;
            std::string name;
            uint64_t latency;
            uint64_t dimension_count, receptor_count, entry_count;
            EntryParams *entries;
            ReceptorParams *receptors;
            float *position_buffer;
            float *output;
        } NeuronParams;

        typedef struct {
            std::map<void*, NeuronParams*> objects;
            std::vector<NeuronParams*> outputs;
            uint64_t batch_size;
            uint64_t max_latency;
            std::atomic<InstanceStatus> status;
            int backend;
        } InstanceData;

        std::vector<InstanceData*> Instances;

        static NeuronParams* doTranslateNeuronToInstance(indk::Neuron *neuron, indk::Neuron *nfrom, NeuronParams *pfrom,
                                                         std::map<void*, NeuronParams*>& objects);
    public:
        ComputeInstanceManager() = default;
        void doCreateInstance(int backend = indk::System::ComputeBackends::Default);
        void doTranslateToInstance(const indk::LinkList& links, const std::vector<std::string>& outputs, int id);
        void doTranslateFromInstance(int id);
        void doRunInstance(const std::vector<std::vector<float>>& x, const std::vector<std::string>& inputs, int iid = 0);
        std::vector<std::pair<float, std::string> > getOutputValues(int iid = 0);
        ~ComputeInstanceManager() = default;
    };
}

#endif //INTERFERENCE_INSTANCE_H
