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
            int hash;
        } InstanceData;
    private:
        std::vector<InstanceData*> Instances;
    public:
        ComputeInstanceManager() = default;
        void doCreateInstance(int backend = indk::System::ComputeBackends::NativeCPU);
        void doTranslateToInstance(const indk::LinkList& links, const std::vector<std::string>& outputs, const indk::StateSyncMap &sync, int iid = 0);
        void doRunInstance(const std::vector<std::vector<float>>& x, const std::vector<std::string>& inputs, int iid = 0);
        void doResetInstance(int iid = 0);
        std::vector<indk::PatternDefinition> doComparePatterns(const std::vector<std::string>& objects, int iid = 0, int method = indk::ScopeProcessingMethods::ProcessMin);
        std::vector<indk::OutputValue> getOutputValues(int iid = 0);
        ~ComputeInstanceManager() = default;
    };
}

#endif //INTERFERENCE_INSTANCE_H
