/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/system.h>
#include <indk/backends/native.h>
#include <indk/backends/native_multithread.h>

int VerbosityLevel = 1;

std::vector<std::shared_ptr<indk::ComputeBackend>> ComputeBackendList = {
        std::make_shared<indk::ComputeBackends::NativeCPU>(),
        std::make_shared<indk::ComputeBackends::NativeCPUMultithread>()
};

void indk::System::doAddComputeBackend(const std::shared_ptr<indk::ComputeBackend>& backend) {
    ComputeBackendList.push_back(backend);
}

void indk::System::setComputeBackendParameters(int id, indk::ComputeBackend::Parameters *parameters) {
    if (id >= ComputeBackendList.size()) return; // error
    ComputeBackendList[id] -> setParameters(parameters);
}

std::shared_ptr<indk::ComputeBackend> indk::System::getComputeBackend(int id) {
    if (id >= ComputeBackendList.size()) return{}; // error
    return ComputeBackendList[id];
}

void indk::System::setVerbosityLevel(int level) {
    VerbosityLevel = level;
}

int indk::System::getVerbosityLevel() {
    return VerbosityLevel;
}

std::vector<indk::ComputeBackendsInfo> indk::System::getComputeBackendsInfo() {
    std::vector<indk::ComputeBackendsInfo> info;
    info.reserve(ComputeBackendList.size());
    for (int i = 0; i < ComputeBackendList.size(); i++) {
        info.push_back({.id=i, .backend_name=ComputeBackendList[i]->getBackendName(), .translator_name=ComputeBackendList[i]->getTranslatorName(), .ready=ComputeBackendList[i]->isReady()});
    }
    return info;
}

bool indk::Event::doWaitTimed(int T) {
    auto rTimeout = std::chrono::milliseconds(T);
    bool bTimeout = false;
    bool bRet;
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    while (!m_bEvent && !bTimeout) {
        bTimeout = std::cv_status::timeout == m_oConditionVariable.wait_for(oNotifierLock, rTimeout);
    }
    bRet = m_bEvent;
    m_bEvent = false;
    return bRet;
}

bool indk::Event::doWait() {
    bool bRet;
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    m_oConditionVariable.wait(oNotifierLock);
    bRet = m_bEvent;
    m_bEvent = false;
    return bRet;
}

void indk::Event::doNotifyOne() {
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    m_bEvent = true;
    m_oConditionVariable.notify_one();
}
