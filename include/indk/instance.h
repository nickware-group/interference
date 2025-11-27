/////////////////////////////////////////////////////////////////////////////
// Name:        indk/instance.h
// Purpose:     Compute Instance Manager class header
// Author:      Nickolay Babich
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
#include <indk/types.h>
#include <map>

namespace indk {
    /// Compute Instance Manager class.
    class ComputeInstanceManager {
    public:
        typedef enum {InstanceReady, InstanceBusy} InstanceStatus;
        typedef enum {InstanceRunAuto = -1} InstanceRunMode;

        typedef struct {
            void *model_data;
            std::atomic<InstanceStatus> status;
            std::shared_ptr<indk::ComputeBackend> backend;
            std::string hash;
        } InstanceData;
    private:
        std::vector<InstanceData*> Instances;
    public:
        ComputeInstanceManager() = default;
        void doCreateInstance(int backend = indk::System::ComputeBackends::NativeCPU);
        void doCreateInstances(int count, int backend = indk::System::ComputeBackends::NativeCPU);
        void doTranslateToInstance(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap &sync, const std::string& prepareid, int iid = 0);
        void doRunInstance(const std::vector<std::vector<float>>& x, const std::vector<std::string>& inputs, int iid = 0);
        void doResetInstance(int iid = 0);
        void doClearInstance(int iid = 0);
        void doClearInstances();
        void setMode(bool learning, int iid = 0);
        std::map<std::string, std::vector<indk::Position>> getReceptorPositions(int iid = 0);
        std::vector<indk::OutputValue> getOutputValues(int iid = 0);
        int getInstanceCount();
        ~ComputeInstanceManager() = default;
    };
}

#endif //INTERFERENCE_INSTANCE_H
