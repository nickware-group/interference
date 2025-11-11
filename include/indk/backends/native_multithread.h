/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     04.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_BACKENDS_NATIVE_MULTITHREAD_H
#define INTERFERENCE_BACKENDS_NATIVE_MULTITHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <indk/backend.h>
#include <indk/system.h>
#include <indk/translators/cpu.h>

namespace indk {
    namespace ComputeBackends {
        class NativeCPUMultithread  : public indk::ComputeBackend {
        private:
            /// \private

            typedef struct worker_task {
                std::vector<std::vector<indk::Translators::CPU::NeuronParams*>> neurons;
                std::vector<indk::Event*> events;
                std::atomic<uint64_t> task_elements_done;
                std::atomic<uint64_t> task_size;
                std::atomic<uint64_t> compute_size;
                std::atomic<uint64_t> workers_done;
            } Task;

            typedef struct worker_context {
                uint64_t thread_id;
                std::vector<Task*> tasks;
                std::thread thread;
                std::mutex m;
                std::condition_variable cv;
                std::mutex task_lock;
            } Context;

            std::vector<std::shared_ptr<Task>> Tasks;
            std::vector<std::shared_ptr<Context>> Workers;
//            std::map<void*, unsigned int> ObjectTable;
            int WorkerCount;
            std::mutex WorkersLock;

            [[noreturn]] static void tCompute(Context *context);
        public:
            typedef struct Parameters : public indk::ComputeBackend::Parameters {
                int worker_count;
            } Parameters;

            NativeCPUMultithread();
            void* doTranslate(const indk::LinkList& links, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync) override;
            void doCompute(const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_model) override;
            void doReset(void*) override;
//            std::vector<indk::PatternDefinition> doComparePatterns(void *model, const std::vector<std::string>& objects, int method) override;
            void setParameters(indk::ComputeBackend::Parameters*) override;
            std::vector<indk::OutputValue> getOutputValues(void *_model) override;
            std::map<std::string, indk::Position> getReceptorPositions(void *_model) override;
        };
    }
}

#endif //INTERFERENCE_BACKENDS_NATIVE_MULTITHREAD_H
