/////////////////////////////////////////////////////////////////////////////
// Name:        indk/system.h
// Purpose:     General-purpose auxiliary methods header
// Author:      Nickolay Babich
// Created:     18.10.22
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_SYSTEM_H
#define INTERFERENCE_SYSTEM_H

#include <mutex>
#include <vector>
#include <condition_variable>
#include <indk/backend.h>

namespace indk {
    class System {
    public:
        static void doAddComputeBackend(const std::shared_ptr<indk::ComputeBackend>& backend);

        static void setComputeBackendParameters(int id, indk::ComputeBackend::Parameters*);

        static std::shared_ptr<indk::ComputeBackend> getComputeBackend(int id);

        static std::vector<ComputeBackendsInfo> getComputeBackendsInfo();

        static bool isComputeBackendAvailable(int id);

        /**
         * Set library verbosity level.
         * @param level New verbosity level value.
         */
        static void setVerbosityLevel(int level);

        /**
         * Get current verbosity level.
         * @return Verbosity level value.
         */
        static int getVerbosityLevel();

        /**
         * Compute backends enum.
         */
        typedef enum {
            /// Native CPU compute backend.
            NativeCPU,
            /// Native CPU multithread compute backend. You can set the number of threads by `parameter` argument of setComputeBackend method.
            NativeCPUMultithread,
            /// OpenCL compute backend.
            OpenCL,
            /// Vulkan compute backend.
            Vulkan
        } ComputeBackends;
    };

    class Event {
    public:
        Event(): m_bEvent(false) {}
        ~Event() = default;
        bool doWaitTimed(int);
        bool doWait();
        void doNotifyOne();
    private:
        std::mutex m_oMutex;
        std::condition_variable m_oConditionVariable;
        bool m_bEvent;
    };
}

#endif //INTERFERENCE_SYSTEM_H
