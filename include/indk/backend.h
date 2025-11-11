/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     05.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_BACKEND_H
#define INTERFERENCE_BACKEND_H

#include <vector>
#include <string>
#include <atomic>
#include <indk/types.h>
#include <indk/position.h>

namespace indk {
    class ComputeBackend {
    protected:
        std::string BackendName;
        std::string TranslatorName;
        std::atomic<bool> Ready{false};
    public:
        struct Parameters {};

        virtual void* doTranslate(const indk::LinkList& links, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync) = 0;
        virtual void doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *instance) = 0;
        virtual void doReset(void*) = 0;
//        virtual std::vector<indk::PatternDefinition> doComparePatterns(void *model, const std::vector<std::string>& objects, int method) = 0;

        virtual void setParameters(indk::ComputeBackend::Parameters*) = 0;

        virtual std::vector<indk::OutputValue> getOutputValues(void*) = 0;
        virtual std::map<std::string, std::vector<indk::Position>> getReceptorPositions(void *_model) = 0;

        std::string getBackendName();
        std::string getTranslatorName();
        bool isReady();
    };
}

#endif //INTERFERENCE_BACKEND_H
