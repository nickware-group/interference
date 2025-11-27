/////////////////////////////////////////////////////////////////////////////
// Name:        indk/backend.h
// Purpose:     Compute Backend abstract class header
// Author:      Nickolay Babich
// Created:     05.11.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_BACKEND_H
#define INTERFERENCE_BACKEND_H

#include <vector>
#include <string>
#include <atomic>
#include <indk/types.h>
#include <indk/position.h>
#include <indk/neuron.h>

namespace indk {
    class ComputeBackend {
    protected:
        std::string BackendName;
        std::string TranslatorName;
        std::atomic<bool> Ready{false};
    public:
        struct Parameters {};

        virtual void* doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync) = 0;
        virtual void doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *instance) = 0;
        virtual void doReset(void*) = 0;
        virtual void doClear(void*) = 0;

        virtual void setMode(void *model, bool learning) = 0;
        virtual void setParameters(indk::ComputeBackend::Parameters*) = 0;

        virtual std::vector<indk::OutputValue> getOutputValues(void*) = 0;
        virtual std::map<std::string, std::vector<indk::Position>> getReceptorPositions(void *_model) = 0;

        std::string getBackendName();
        std::string getTranslatorName();
        bool isReady();
    };
}

#endif //INTERFERENCE_BACKEND_H
