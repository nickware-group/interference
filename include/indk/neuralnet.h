/////////////////////////////////////////////////////////////////////////////
// Name:        indk/neuralnet.h
// Purpose:     Neural net classes header
// Author:      Nickolay Babich
// Created:     12.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_NEURALNET_H
#define INTERFERENCE_NEURALNET_H

#include <map>
#include <algorithm>
#include <tuple>
#include <functional>
#include <queue>
#include <indk/neuron.h>
#include <indk/system.h>
#include <indk/interlink.h>
#include <indk/instance.h>

namespace indk {
    typedef enum {
        CompareDefault,
        CompareNormalized
    } PatternCompareFlags;

    typedef std::queue<std::string> NQueue;
    typedef std::vector<std::pair<std::string, std::vector<std::string>>> EntryList;

    /**
     * Main neural net class.
     */
    class NeuralNet {
    private:
        std::string Name, Description, Version;

        EntryList Entries;
        std::map<std::string, std::vector<std::string>> Ensembles;
        std::map<std::string, indk::Neuron*> Neurons;
        std::vector<std::string> Outputs;

        StateSyncMap StateSyncList;

        int64_t doFindEntry(const std::string&);
        std::vector<indk::Neuron*> doParseActiveNeurons(const std::vector<std::string>& inputs, int mode);

        std::vector<indk::Neuron*> LastActiveNeurons;
        std::string PrepareID;
        bool StateSyncEnabled;

        indk::Interlink *InterlinkService;
        std::vector<std::vector<std::string>> InterlinkDataBuffer;

        indk::ComputeInstanceManager InstanceManager;
    public:
        NeuralNet();
        explicit NeuralNet(const std::string &path);
        void doInterlinkInit(int port = 4408, int timeout = 5);
        void doInterlinkWebInit(const std::string& path, int port = 8044);
        void doInterlinkSyncStructure(const std::string &data = "");
        void doInterlinkSyncData(int mode, int instance);
        std::vector<float> doComparePatterns(int CompareFlag = indk::PatternCompareFlags::CompareDefault,
                                             int ProcessingMethod = indk::ScopeProcessingMethods::ProcessMin,
                                             int instance = 0);
        std::vector<float> doComparePatterns(const std::string& ename,
                                             int CompareFlag = indk::PatternCompareFlags::CompareDefault,
                                             int ProcessingMethod = indk::ScopeProcessingMethods::ProcessMin,
                                             int instance = 0);
        std::vector<float> doComparePatterns(std::vector<std::string> nnames,
                                             int CompareFlag = indk::PatternCompareFlags::CompareDefault,
                                             int ProcessingMethod = indk::ScopeProcessingMethods::ProcessMin,
                                             int instance = 0);
        void doCreateNewScope();
        void doChangeScope(uint64_t);
        void doAddNewOutput(const std::string&);
        void doIncludeNeuronToEnsemble(const std::string&, const std::string&);

        void doReset(int instance);
        void doStructurePrepare(int mode = 0, int instance = 0);

        void doCreateInstance(int backend = indk::System::ComputeBackends::NativeCPU);
        void doCreateInstances(int count, int backend = indk::System::ComputeBackends::NativeCPU);

        std::vector<indk::OutputValue> doLearn(const std::vector<std::vector<float>>&, bool prepare = true, const std::vector<std::string>& inputs = {}, int instance = 0);
        std::vector<indk::OutputValue> doRecognise(const std::vector<std::vector<float>>&, bool prepare = true, const std::vector<std::string>& inputs = {}, int instance = 0);
        void doLearnAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<indk::OutputValue>)>& Callback = nullptr, bool prepare = true,
                          const std::vector<std::string>& inputs = {}, int instance = 0);
        void doRecogniseAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<indk::OutputValue>)>& Callback = nullptr, bool prepare = true,
                              const std::vector<std::string>& inputs = {}, int instance = 0);

        std::vector<indk::OutputValue> doSignalReceive(const std::string& ensemble = "");
        indk::Neuron* doReplicateNeuron(const std::string& from, const std::string& to, bool integrate);
        void doDeleteNeuron(const std::string& name);
        void doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries = false);
        void doClearCache();


        std::vector<indk::OutputValue> doSignalProcess(const std::vector<std::vector<float>>& x, const std::vector<std::string>& inputs, bool mode, int instance = 0);

        std::string setStructure(std::ifstream&);
        void setStructure(const std::string &Str);
        void setLearned(bool);
        void setStateSyncEnabled(bool enabled = true);
        bool isLearned();
        std::string getStructure(bool minimized = true);
        std::string getName();
        std::string getDescription();
        std::string getVersion();
        std::vector<indk::Neuron*> getEnsemble(const std::string&);
        indk::Neuron* getNeuron(const std::string&);
        std::vector<indk::Neuron*> getNeurons();
        uint64_t getNeuronCount();
        uint64_t getTotalParameterCount();
        int getInstanceCount();
        ~NeuralNet();
    };
}

#endif //INTERFERENCE_NEURALNET_H
