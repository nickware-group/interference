/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_BACKENDS_NATIVE_H
#define INTERFERENCE_BACKENDS_NATIVE_H

#include <indk/backend.h>
#include <indk/position.h>

namespace indk {
    namespace ComputeBackends {
        class NativeCPU  : public indk::ComputeBackend {
        public:
            NativeCPU();
            void* doTranslate(const indk::LinkList& links, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync) override;
            void doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_instance) override;
            void doReset(void*) override;
            void setMode(void *model, bool learning) override;
            void setParameters(indk::ComputeBackend::Parameters*) override;
            std::vector<indk::OutputValue> getOutputValues(void *_model) override;
            std::map<std::string, std::vector<indk::Position>> getReceptorPositions(void *_model) override;
        };
    }
}

#endif //INTERFERENCE_BACKENDS_NATIVE_H
