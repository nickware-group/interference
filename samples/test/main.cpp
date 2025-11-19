/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     30.01.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <fstream>
#include <indk/neuralnet.h>
#include <indk/profiler.h>
#include <iomanip>
#include "indk/backends/native_multithread.h"
#include "indk/backends/opencl.h"

indk::NeuralNet NN;
std::vector<std::vector<float>> X;

std::vector<indk::ComputeBackendsInfo> backends;

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
                                                                                time_since_epoch()).count();
}

int doCheckAvailableBackends() {
    int rcount = 0;
    std::cout << "=== AVAILABLE COMPUTE BACKENDS ===" << std::endl;
    std::cout << std::setw(10) << std::left << "ID" << std::setw(40) << "Backend name" << std::setw(20) << "Translator name" << std::setw(20) << "Ready" << std::endl;
    std::cout << "-----------------------------------------------------------------------------" << std::endl;
    for (auto &info: backends) {
        std::cout << std::setw(10) << info.backend_id << std::setw(40) << std::left << info.backend_name << std::setw(20) << info.translator_name << std::setw(20) << (info.ready?"Yes":"No") << std::endl;
        rcount += info.ready;
    }
    std::cout << "-----------------------------------------------------------------------------" << std::endl << std::endl;

    if (indk::System::isComputeBackendAvailable(indk::System::ComputeBackends::OpenCL)) {
        auto info = indk::ComputeBackends::OpenCL::getDevicesInfo();
        std::cout << "=== OPENCL DEVICES ===" << std::endl;
        std::cout << std::setw(40) << std::left << "Platform name" << std::setw(80) << "Device name" << std::setw(20) << "Compute units" << std::setw(20) << "Workgroup size" << std::endl;
        std::cout << "------------------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
        for (auto &i: info) {
            std::cout << std::setw(40) << i.platform_name << std::setw(80) << std::left << i.device_name << std::setw(20) << i.compute_units << std::setw(20) << i.workgroup_size << std::endl;
        }
        std::cout << "------------------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
    }

    return rcount;
}

void doCreateInstances() {
    for (auto &info: backends) {
        if (info.ready) NN.doCreateInstance(info.backend_id);
    }
}

void doLoadModel(const std::string& path, int size) {
    std::ifstream structure(path);
    NN.setStructure(structure);

    for (int i = 2; i < size; i++) {
        NN.doReplicateEnsemble("A1", "A"+std::to_string(i));
    }
    NN.doStructurePrepare();

    std::cout << "Model name            : " << NN.getName() << std::endl;
    std::cout << "Model desc            : " << NN.getDescription() << std::endl;
    std::cout << "Model ver             : " << NN.getVersion() << std::endl;
    std::cout << "Neuron count          : " << NN.getNeuronCount() << std::endl;
    std::cout << "Total parameter count : " << NN.getTotalParameterCount() << std::endl;
    std::cout << "Compute Instance count: " << NN.getInstanceCount() << std::endl;
    std::cout << std::endl;
}

int doTest(float ref, int instance) {
    auto T = getTimestampMS();
    auto Y = NN.doLearn(X, true, {}, instance);
    T = getTimestampMS() - T;
    std::cout << std::setw(20) << std::left << "done ["+std::to_string(T)+" ms] ";

    bool passed = false;
    for (auto &y: Y) {
        if (std::fabs(y.value-ref) > 1e-3) {
            std::cout << "[FAILED]" << std::endl;
            std::cout << "Output value " << y.value << " is not " << ref << std::endl;
            std::cout << std::endl;
            passed = false;
            break;
        } else passed = true;
    }
    if (passed) {
        std::cout << "[PASSED]" << std::endl;
    }

    return passed;
}

int doTests(const std::string& name, float ref) {
    int count = 0;

    for (auto &info: backends) {
        if (info.ready) {
            std::cout << std::setw(60) << std::left << name+" ("+info.backend_name+"): ";
            count += doTest(ref, info.backend_id); // using backend id as instance id
        }
    }
    std::cout << std::endl << std::endl;

    return count;
}

int main() {
    backends = indk::System::getComputeBackendsInfo();
    auto rbackends = doCheckAvailableBackends();

    constexpr unsigned STRUCTURE_COUNT                      = 2;
    constexpr float SUPERSTRUCTURE_TEST_REFERENCE_OUTPUT    = 0.0291;
    constexpr float BENCHMARK_TEST_REFERENCE_OUTPUT         = 2.7622;
    const unsigned TOTAL_TEST_COUNT                         = STRUCTURE_COUNT*rbackends;

    // setting up parameters
    indk::ComputeBackends::NativeCPUMultithread::Parameters parameters_mt;
    parameters_mt.worker_count = 4;
    indk::System::setComputeBackendParameters(indk::System::ComputeBackends::NativeCPUMultithread, &parameters_mt);

//    indk::ComputeBackends::OpenCL::Parameters parameters_cl;
//    parameters_cl.device_name = "NVIDIA GeForce RTX 5070";
//    indk::System::setComputeBackendParameters(indk::System::ComputeBackends::OpenCL, &parameters_cl);

    indk::System::setVerbosityLevel(1);

    int count = 0;

    doCreateInstances();

    // creating data array
    X.emplace_back();
    X.emplace_back();
    for (int i = 0; i < 170; i++) {
        X[0].push_back(50);
        X[1].push_back(50);
    }

    // running tests
    std::cout << "=== SUPERSTRUCTURE TEST ===" << std::endl;
    doLoadModel("structures/structure_general.json", 101);
    count += doTests("Superstructure test", SUPERSTRUCTURE_TEST_REFERENCE_OUTPUT);

    std::cout << "=== BENCHMARK ===" << std::endl;
    doLoadModel("structures/structure_bench.json", 10001);
    count += doTests("Benchmark", BENCHMARK_TEST_REFERENCE_OUTPUT);

    std::cout << std::endl;
    std::cout << "Tests passed: [" << count << "/" << TOTAL_TEST_COUNT << "]" << std::endl;

    if (count != TOTAL_TEST_COUNT) return 1;
    return 0;
}
