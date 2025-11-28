/////////////////////////////////////////////////////////////////////////////
// Name:        neuralnet/neuralnet.cpp
// Purpose:     Neural net main class
// Author:      Nickolay Babich
// Created:     12.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <queue>
#include <thread>
#include <json.hpp>
#include <indk/neuralnet.h>
#include <indk/profiler.h>

typedef nlohmann::json json;

indk::NeuralNet::NeuralNet() {
    StateSyncEnabled = false;
    InterlinkService = new indk::Interlink();
}

indk::NeuralNet::NeuralNet(const std::string &path) {
    StateSyncEnabled = false;
    InterlinkService = new indk::Interlink();
    std::ifstream filestream(path);
    setStructure(filestream);
}

void indk::NeuralNet::doInterlinkInit(int port, int timeout) {
    InterlinkService -> doInitInput(port, timeout);

    indk::Profiler::doAttachCallback(this, indk::Profiler::EventFlags::EventTick, [this](indk::NeuralNet *nn) {
        auto neurons = getNeurons();
        for (uint64_t i = 0; i < neurons.size(); i++) {
            if (i >= InterlinkDataBuffer.size()) {
                InterlinkDataBuffer.emplace_back();
            }
//            InterlinkDataBuffer[i].push_back(std::to_string(neurons[i]->doSignalReceive().second));
        }
    });

    indk::Profiler::doAttachCallback(this, indk::Profiler::EventFlags::EventProcessed, [this](indk::NeuralNet *nn) {
        doInterlinkSyncData();
    });

    if (InterlinkService->isInterlinked()) {
        if (!InterlinkService->getStructure().empty()) setStructure(InterlinkService->getStructure());
    }
}

void indk::NeuralNet::doInterlinkWebInit(const std::string& path, int port) {
    InterlinkService -> doInitWebServer(path, port);
}

void indk::NeuralNet::doInterlinkSyncStructure() {
    if (!InterlinkService || InterlinkService && !InterlinkService->isInterlinked()) return;
    InterlinkService -> doUpdateStructure(getStructure());
}

void indk::NeuralNet::doInterlinkSyncData() {
    if (!InterlinkService || InterlinkService && !InterlinkService->isInterlinked()) return;
    json j, jm;
    uint64_t in = 0;

    for (const auto &n: Neurons) {
        json jn, jnm;

        jn["name"] = n.second->getName();

        jnm["name"] = n.second->getName();
        jnm["output_signal"] = json::parse("[]");

        if (in < InterlinkDataBuffer.size()) {
            for (const auto& o: InterlinkDataBuffer[in]) {
                jnm["output_signal"].push_back(o);
            }
        }
        jm.push_back(jnm);

        for (int i = 0; i < n.second->getReceptorsCount(); i++) {
            json jr;
            auto r = n.second -> getReceptor(i);
            jr["sensitivity"] = r -> getSensitivityValue();

            auto scopes = r -> getReferencePosScopes();
            for (const auto &s: scopes) {
                json js;
                for (int p = 0; p < n.second->getDimensionsCount(); p++) {
                    js.push_back(s->getPositionValue(p));
                }
                jr["scopes"].push_back(js);
            }

            for (int p = 0; p < n.second->getDimensionsCount(); p++) {
                jr["phantom"].push_back(r->getPosf()->getPositionValue(p));
            }

            jn["receptors"].push_back(jr);
        }

        j["neurons"].push_back(jn);
        in++;
    }

    InterlinkDataBuffer.clear();
    InterlinkService -> doUpdateModelData(j.dump());
    InterlinkService -> doUpdateMetrics(jm.dump());
}

int64_t indk::NeuralNet::doFindEntry(const std::string& ename) {
    auto ne = std::find_if(Entries.begin(), Entries.end(), [ename](const std::pair<std::string, std::vector<std::string>>& e){
        return e.first == ename;
    });
    if (ne == Entries.end()) return -1;
    return std::distance(Entries.begin(), ne);
}

std::vector<float> indk::NeuralNet::doComparePatterns(int CompareFlag, int ProcessingMethod, int instance) {
    return doComparePatterns(std::vector<std::string>(), CompareFlag, ProcessingMethod, instance);
}

std::vector<float> indk::NeuralNet::doComparePatterns(const std::string& ename, int CompareFlag, int ProcessingMethod, int instance) {
    auto en = Ensembles.find(ename);
    if (en != Ensembles.end()) {
        return doComparePatterns(en->second, CompareFlag, ProcessingMethod, instance);
    }
    return {};
}

/**
 * Compare neuron patterns (learning and recognition patterns) for all output neurons.
 * @return Vector of pattern difference values for each output neuron.
 */
std::vector<float> indk::NeuralNet::doComparePatterns(std::vector<std::string> nnames, int CompareFlag, int ProcessingMethod, int instance) {
    std::vector<float> PDiffR, PDiff;

    auto result = InstanceManager.getReceptorPositions(instance);

    if (nnames.empty()) nnames = Outputs;
    for (const auto& O: nnames) {
        auto n = Neurons.find(O);
        if (n == Neurons.end()) break;
        auto r = result.find(O);
        if (r == result.end()) break;

        auto P = n -> second -> doComparePattern(r->second, ProcessingMethod);

        PDiffR.push_back(std::get<0>(P));
    }

    switch (CompareFlag) {
        default:
        case indk::PatternCompareFlags::CompareDefault:
            return PDiffR;

        case indk::PatternCompareFlags::CompareNormalized:
            float PDRMin = PDiffR[std::distance(PDiffR.begin(), std::min_element(PDiffR.begin(), PDiffR.end()))];
            float PDRMax = PDiffR[std::distance(PDiffR.begin(), std::max_element(PDiffR.begin(), PDiffR.end()))] - PDRMin;
            for (auto &PDR: PDiffR) {
                if (PDRMax != 0) PDiff.push_back(1 - (PDR-PDRMin) / PDRMax);
                else PDiff.push_back(1);
            }
            return PDiff;
    }
}

void indk::NeuralNet::doCreateNewScope() {
    for (const auto& N: Neurons) N.second -> doCreateNewScope();
}

void indk::NeuralNet::doChangeScope(uint64_t scope) {
    for (const auto& N: Neurons) N.second -> doChangeScope(scope);
}

void indk::NeuralNet::doAddNewOutput(const std::string& name) {
    Outputs.push_back(name);
}

void indk::NeuralNet::doIncludeNeuronToEnsemble(const std::string& name, const std::string& ensemble) {
    auto en = Ensembles.find(ensemble);
    if (en != Ensembles.end()) {
        en -> second.push_back(name);
    } else {
        Ensembles.insert(std::make_pair(ensemble, std::vector<std::string>({name})));
    }
}

/**
 * Resets all neurons in the network.
 * See indk::Neuron::doReset() method for details.
 */
void indk::NeuralNet::doReset(int instance) {
    InstanceManager.doResetInstance(instance);
}

std::vector<indk::Neuron*> indk::NeuralNet::doParseActiveNeurons(const std::vector<std::string>& inputs, int mode) {
    std::string id = std::to_string(mode);
    for (auto &i: inputs) id += i;
    if (PrepareID == id) return LastActiveNeurons;

    InstanceManager.doClearInstances();

    std::vector<indk::Neuron*> aneurons;

    NQueue nqueue;

    for (auto &e: Entries) {
        auto found = std::find(inputs.begin(), inputs.end(), e.first);
        if (found != inputs.end()) {
            for (auto &en: e.second) {
                nqueue.emplace(en);
            }
        }
    }

    while (!nqueue.empty()) {
        auto nname = nqueue.front();
        nqueue.pop();

        auto n = Neurons.find(nname);

        if (n != Neurons.end()) {
            auto found = std::find(aneurons.begin(), aneurons.end(), n->second);
            if (found != aneurons.end()) continue;

            aneurons.push_back(n->second);

            auto nlinks = n -> second -> getLinkOutput();
            for (auto &nl: nlinks) {
                nqueue.emplace(nl);
            }
        }
    }

    if (!mode && StateSyncEnabled) { // state sync working if mode is not learning
        for (const auto &s: StateSyncList) {
            auto found = std::find_if(aneurons.begin(), aneurons.end(), [s](indk::Neuron *n) { return n->getName() == s.second; });
            if (found != aneurons.end()) {
                auto n = Neurons.find(s.first);
                if (n != Neurons.end()) aneurons.push_back(n->second);
            }
        }
    }

    LastActiveNeurons = aneurons;
    PrepareID = id;

    return LastActiveNeurons;
}

void indk::NeuralNet::doStructurePrepare(int mode, int instance) {
    std::vector<std::string> entries;
    for (const auto &e: Entries) entries.push_back(e.first);
    auto aneurons = doParseActiveNeurons(entries, mode);

    if (StateSyncEnabled) InstanceManager.doTranslateToInstance(aneurons, Outputs, StateSyncList, PrepareID, instance);
    else InstanceManager.doTranslateToInstance(aneurons, Outputs, {}, PrepareID, instance);
}

void indk::NeuralNet::doCreateInstance(int backend) {
    InstanceManager.doCreateInstance(backend);
}

void indk::NeuralNet::doCreateInstances(int count, int backend) {
    InstanceManager.doCreateInstances(count, backend);
}

std::vector<indk::OutputValue> indk::NeuralNet::doSignalProcess(const std::vector<std::vector<float>>& x, const std::vector<std::string>& inputs, bool mode, int instance) {
    std::vector<std::string> entries = inputs;

    if (entries.empty()) {
        for (const auto &e: Entries) {
            entries.push_back(e.first);
        }
    }

    auto aneurons = doParseActiveNeurons(entries, mode);

    if (StateSyncEnabled) InstanceManager.doTranslateToInstance(aneurons, Outputs, StateSyncList, PrepareID, instance);
    else InstanceManager.doTranslateToInstance(aneurons, Outputs, {}, PrepareID, instance);

    InstanceManager.setMode(mode, instance);
    InstanceManager.doRunInstance(x, entries, instance);

    // if mode == learning, save receptor positions in current scope
    if (mode) {
        auto positions = InstanceManager.getReceptorPositions(instance);
        for (const auto &p: positions) {
            auto found = Neurons.find(p.first);
            if (found != Neurons.end()) {
                for (int i = 0; i< p.second.size(); i++) {
                    auto pos = p.second[i];
                    found -> second -> getReceptor(i) -> setPos(&pos);
                }
            }
        }
    }

    auto output = InstanceManager.getOutputValues(instance);

    return output;
}

/**
 * Start neural network learning process.
 * @param Xx Input data vector that contain signals for learning.
 * @return Output signals.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doLearn(const std::vector<std::vector<float>>& Xx, bool prepare, const std::vector<std::string>& inputs, int instance) {
    if (InterlinkService && InterlinkService->isInterlinked()) {
        InterlinkService -> doUpdateStructure(getStructure());
    }
    if (prepare) doReset(instance);
    return doSignalProcess(Xx, inputs, true, instance);
}

/**
 * Recognize data by neural network.
 * @param Xx Input data vector that contain signals for recognizing.
 * @return Output signals.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doRecognise(const std::vector<std::vector<float>>& Xx, bool prepare, const std::vector<std::string>& inputs, int instance) {
    if (prepare) doReset(instance);
    return doSignalProcess(Xx, inputs, false, instance);
}

/**
 * Start neural network learning process asynchronously.
 * @param Xx Input data vector that contain signals for learning.
 * @param callback Callback function for output signals.
 */
void indk::NeuralNet::doLearnAsync(const std::vector<std::vector<float>>& Xx, const std::function<void(std::vector<indk::OutputValue>)>& callback, bool prepare,
                                   const std::vector<std::string>& inputs, int instance) {
    InstanceManager.setMode(true, instance);
    if (prepare) doReset(instance);
//    doSignalTransferAsync(Xx, callback, inputs);
}

/**
 * Recognize data by neural network asynchronously.
 * @param Xx Input data vector that contain signals for recognizing.
 * @param callback Callback function for output signals.
 */
void indk::NeuralNet::doRecogniseAsync(const std::vector<std::vector<float>>& Xx, const std::function<void(std::vector<indk::OutputValue>)>& callback, bool prepare,
                                       const std::vector<std::string>& inputs, int instance) {
    InstanceManager.setMode(false, instance);
    if (prepare) doReset(instance);
//    doSignalTransferAsync(Xx, callback, inputs);
}

/**
 * Get output signals.
 * @return Output signals vector.
 */
std::vector<indk::OutputValue> indk::NeuralNet::doSignalReceive(const std::string& ensemble) {
    std::vector<indk::OutputValue> ny;
    std::vector<std::string> elist;

    if (!ensemble.empty()) {
        auto en = Ensembles.find(ensemble);
        if (en == Ensembles.end()) return {};
        elist = en -> second;
        if (elist.empty()) return {};
    }

    for (const auto& oname: Outputs) {
        auto n = Neurons.find(oname);
        if (n != Neurons.end()) {
            if (!elist.empty()) {
                auto item = std::find_if(elist.begin(), elist.end(), [oname](const std::string &value) {
                    return oname == value;
                });
                if (item == elist.end()) continue;
            }

//            ny.emplace_back(n->second->doSignalReceive().second, oname);
        }
    }
    return ny;
}

/**
 * Creates full copy of neuron.
 * @param from Source neuron name.
 * @param to Name of new neuron.
 * @param integrate Link neuron to the same elements as the source neuron.
 */
indk::Neuron* indk::NeuralNet::doReplicateNeuron(const std::string& from, const std::string& to, bool integrate) {
    PrepareID = "";

    auto n = Neurons.find(from);
    if (n == Neurons.end()) {
        if (indk::System::getVerbosityLevel() > 0)
            std::cout << "Neuron replication error: element " << from << " not found" << std::endl;
        return nullptr;
    }
    if (Neurons.find(to) != Neurons.end()) {
        if (indk::System::getVerbosityLevel() > 0)
            std::cout << "Neuron replication error: element " << to << " already exists" << std::endl;
        return nullptr;
    }

    auto nnew = new indk::Neuron(*n->second);
    nnew -> setName(to);
    Neurons.insert(std::make_pair(to, nnew));

    if (integrate) {
        auto entries = nnew -> getEntries();
        for (auto &e: entries) {
            auto ne = doFindEntry(e);
            if (ne != -1) {
                Entries[ne].second.push_back(to);
            } else {
                auto nfrom = Neurons.find(e);
                if (nfrom != Neurons.end()) {
                    nfrom -> second -> doLinkOutput(to);
                }
            }
        }

//        bool found = false;
//        for (auto& e: Ensembles) {
//            for (const auto &en: e.second) {
//                if (en == from) {
//                    e.second.push_back(to);
//                    found = true;
//                    break;
//                }
//            }
//            if (found) break;
//        }
    } else {
//        nnew ->  doClearEntries();
    }
    return nnew;
}

/**
 * Delete the neuron.
 * @param name Name of the neuron.
 */
void indk::NeuralNet::doDeleteNeuron(const std::string& name) {
    auto n = Neurons.find(name);
    if (n == Neurons.end()) return;
    delete n->second;
    Neurons.erase(n);
}

/**
 * Creates full copy of group of neurons.
 * @param from Source ensemble name.
 * @param to Name of new ensemble.
 * @param entries Copy entries during replication. So, if neuron `A1N1` (ensemble `A1`) has an entry `A1E1`
 * and you replicating to ensemble `A2`, a new entry `A2E1` will be added.
 */
void indk::NeuralNet::doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries) {
    json j;

    PrepareID = "";
    std::vector<std::string> enew;
    auto efrom = Ensembles.find(From);

    if (efrom != Ensembles.end()) {
        auto eto = Ensembles.find(To);
        auto lastname = efrom->second.back();

        std::map<std::string, std::string> newnames;
        for (auto &nn: efrom->second) {
            std::string nname;
            if (nn.substr(0, From.size()) == From) {
                nname = nn;
                nname.replace(0, From.size(), To);
            } else nname = To + nn;
            newnames.insert(std::make_pair(nn, nname));
        }

        for (auto &en: efrom->second) {
            json ji;

            auto n = Neurons.find(en);
            if (n != Neurons.end()) {
                auto nnew = new indk::Neuron(*n->second);
                std::string nname = newnames.find(en)->second;
                nnew -> setName(nname);

                auto entries = nnew -> getEntries();
                for (auto &e: entries) {
                    std::string ename = e;
                    auto r = newnames.find(e);
                    if (r != newnames.end()) {
                        nnew -> doReplaceEntryName(e, r->second);
                    } else if (CopyEntries) {
                        if (e.substr(0, From.size()) == From) {
                            ename = e;
                            ename.replace(0, From.size(), To);
                        } else ename = To + e;
                        nnew -> doReplaceEntryName(e, ename);
                    }

                    auto ne = doFindEntry(ename);
                    if (ne != -1) {
                        Entries[ne].second.push_back(nname);
                    } else if (CopyEntries && r == newnames.end()) {
                        std::vector<std::string> elinks;
                        elinks.push_back(nname);
                        j["entries"].push_back(ename);
                        Entries.emplace_back(ename, elinks);
                    }
                }

                auto outputlinks = nnew -> getLinkOutput();
                nnew -> doClearOutputLinks();
                for (auto &o: outputlinks) {
                    auto r = newnames.find(o);
                    if (r != newnames.end()) {
                        nnew -> doLinkOutput(r->second);
                    } else {
                        nnew -> doLinkOutput(o);
                    }
                }

                auto no = std::find(Outputs.begin(), Outputs.end(), en);
                if (no != Outputs.end()) {
                    Outputs.push_back(nname);
                    j["outputs"].push_back(nname);
                }

                Neurons.insert(std::make_pair(nname, nnew));

                entries = nnew -> getEntries();
                for (auto &e: entries) {
                    ji["input_signals"].push_back(e);
                }
                ji["old_name"] = en;
                ji["new_name"] = nname;
                j["neurons"].push_back(ji);

                auto sobject = StateSyncList.find(en);
                if (sobject == StateSyncList.end()) {
                    StateSyncList.insert(std::make_pair(nname, en));
                }

                if (eto == Ensembles.end()) {
                    enew.push_back(nname);
                } else {
                    eto->second.push_back(nname);
                }
            }
        }

        if (eto == Ensembles.end()) Ensembles.insert(std::make_pair(To, enew));
    }

    if (indk::System::getVerbosityLevel() > 1) {
        auto e = Ensembles.find(To);
        std::cout << "Entries: ";
        for (const auto& ne: Entries) {
            std::cout << ne.first << " ";
        }
        std::cout << std::endl;
        std::cout << e->first << " -";
        for (const auto& en: e->second) {
            std::cout << " " << en;
        }
        std::cout << std::endl;
        std::cout << "Outputs: ";
        for (const auto& o: Outputs) {
            std::cout << o << " ";
        }
        std::cout << std::endl;
    }
}

void indk::NeuralNet::doClearCache() {
    PrepareID = "";
}

/**
 * Load neural network structure.
 * @param Stream Input stream of file that contains neural network structure in JSON format.
 */
void indk::NeuralNet::setStructure(std::ifstream &Stream) {
    if (!Stream.is_open()) {
        if (indk::System::getVerbosityLevel() > 0) std::cerr << "Error opening file" << std::endl;
        return;
    }
    std::string jstr;
    while (!Stream.eof()) {
        std::string rstr;
        getline(Stream, rstr);
        jstr.append(rstr);
    }
    setStructure(jstr);
}

/** \example samples/test/structure.json
 * Example of interference neural net structure. It can be used by NeuralNet class and indk::NeuralNet::setStructure method.
 */

/**
 * Load neural network structure.
 * @param Str JSON string that contains neural network structure.
 *
 * Format of neural network structure:
 * \code
 * {
 *      "entries": [<list of neural network entries>],
 *      "neurons": [{
 *          "name": <name of neuron>,
            "size": <size of neuron>,
            "dimensions": 3,
            "input_signals": [<list of input signals, it can be network entries or other neurons>],
            "ensemble": <the name of the ensemble to which the neuron will be connected>,
            "synapses": [{
                "entry": 0,
                "position": [100, 100, 100],
                "neurotransmitter": "activation",
                "k1": 1
            }],
            "receptors": [{
                "type": "cluster",
                "position": [100, 210, 100],
                "count": 15,
                "radius": 10
            }]
 *      }],
 *      "output_signals": [<list of output sources>],
 *      "name": "neural network structure name",
 *      "desc": "neural network structure description",
 *      "version": "neural network structure version"
 * }
 * \endcode
 *
 *
 * @note
 * Example of neural network structure can be found in the samples:
 * <a href="samples_2test_2structure_8json-example.html">test sample</a>,
 * <a href="samples_2test_2structure_8json-example.html">vision sample</a>
 *
 */
void indk::NeuralNet::setStructure(const std::string &Str) {
    for (const auto& N: Neurons) delete N.second;
    PrepareID = "";
    Entries.clear();
    Outputs.clear();
    Neurons.clear();
    Ensembles.clear();
    StateSyncList.clear();

    try {
        auto j = json::parse(Str);
        //std::cout << j.dump(4) << std::endl;
        Name = j["name"].get<std::string>();
        Description = j["desc"].get<std::string>();
        Version = j["version"].get<std::string>();

        std::multimap<std::string, std::string> links;
        for (auto &jneuron: j["neurons"].items()) {
            auto nname = jneuron.value()["name"].get<std::string>();
            for (auto &jinputs: jneuron.value()["input_signals"].items()) {
                auto iname = jinputs.value().get<std::string>();
                links.insert(std::make_pair(iname, nname));
            }
        }

        for (auto &jentry: j["entries"].items()) {
            auto ename = jentry.value().get<std::string>();
            std::vector<std::string> elinks;
            auto l = links.equal_range(ename);
            for (auto it = l.first; it != l.second; it++) {
                elinks.push_back(it->second);
                if (indk::System::getVerbosityLevel() > 1) std::cout << ename << " -> " << it->second << std::endl;
            }
            Entries.emplace_back(ename, elinks);
        }

        for (auto &joutput: j["output_signals"].items()) {
            auto oname = joutput.value().get<std::string>();
            if (indk::System::getVerbosityLevel() > 1) std::cout <<  "Output " << oname << std::endl;
            Outputs.push_back(oname);
        }

//        for (auto &l: links) {
//            std::cout << l.first << " - " << l.second << std::endl;
//        }

        for (auto &jneuron: j["neurons"].items()) {
            auto nname = jneuron.value()["name"].get<std::string>();
            auto nsize = jneuron.value()["size"].get<unsigned int>();
            auto ndimensions = jneuron.value()["dimensions"].get<unsigned int>();
            uint64_t nlatency = 0;
            if (jneuron.value()["latency"] != nullptr) {
                nlatency = jneuron.value()["latency"].get<uint64_t>();
                if (indk::System::getVerbosityLevel() > 1) std::cout << nname << " with latency " << nlatency << std::endl;
            }
            std::vector<std::string> nentries;
            for (auto &jent: jneuron.value()["input_signals"].items()) {
                nentries.push_back(jent.value().get<std::string>());
            }
            auto *N = new indk::Neuron(nsize, ndimensions, nlatency, nentries);

            if (jneuron.value()["ensemble"] != nullptr) {
                auto ename = jneuron.value()["ensemble"].get<std::string>();
                auto e = Ensembles.find(ename);
                if (e == Ensembles.end()) {
                    std::vector<std::string> en;
                    en.push_back(nname);
                    Ensembles.insert(std::make_pair(ename, en));
                } else {
                    e->second.push_back(nname);
                }
            }

            for (auto &jsynapse: jneuron.value()["synapses"].items()) {
                std::vector<float> pos;
                if (ndimensions != jsynapse.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jsynapse.value()["position"].items()) {
                    pos.push_back(jposition.value().get<float>());
                }
                float k1 = 1.2;
                if (jsynapse.value()["k1"] != nullptr) k1 = jsynapse.value()["k1"].get<float>();
                unsigned int tl = 0;
                if (jsynapse.value()["tl"] != nullptr) tl = jsynapse.value()["tl"].get<unsigned int>();
                int nt = 0;
                if (jsynapse.value()["neurotransmitter"] != nullptr) {
                    if (jsynapse.value()["neurotransmitter"].get<std::string>() == "deactivation")
                        nt = 1;
                }
                if (jsynapse.value()["type"] != nullptr && jsynapse.value()["type"].get<std::string>() == "cluster") {
                    auto sradius = jsynapse.value()["radius"].get<unsigned int>();
                    N -> doCreateNewSynapseCluster(pos, sradius, k1, tl, nt);
                } else {
                    auto sentryid = -1;
                    if (jsynapse.value()["entry"] != nullptr) {
                        sentryid = jsynapse.value()["entry"].get<unsigned int>();
                    } else {
                        std::cout << "Error: entry number must be set" << std::endl;
                        return;
                    }
                    auto sentry = jneuron.value()["input_signals"][sentryid];
                    N -> doCreateNewSynapse(sentry, pos, k1, tl, nt);
                }


            }
            for (auto &jreceptor: jneuron.value()["receptors"].items()) {
                std::vector<float> pos;
                if (ndimensions != jreceptor.value()["position"].size()) {
                    std::cout << "Error: position vector size not equal dimension count" << std::endl;
                    return;
                }
                for (auto &jposition: jreceptor.value()["position"].items()) {
                    pos.push_back(jposition.value().get<float>());
                }
                if (jreceptor.value()["type"] != nullptr && jreceptor.value()["type"].get<std::string>() == "cluster") {
                    auto rcount = jreceptor.value()["count"].get<unsigned int>();
                    auto rradius = jreceptor.value()["radius"].get<unsigned int>();
                    N -> doCreateNewReceptorCluster(pos, rradius, rcount);

                    for (auto i = N->getReceptorsCount()-rcount; i < N ->getReceptorsCount(); i++) {
                        auto r = N -> getReceptor(i);
                        for (auto &jscope: jreceptor.value()["scopes"].items()) {
                            pos.clear();
                            for (auto &jposition: jscope.value().items()) {
                                pos.push_back(jposition.value().get<float>());
                            }
                            N -> doCreateNewScope();
                            r -> setPos(new indk::Position(nsize, pos));
                        }
                    }
                } else {
                    N -> doCreateNewReceptor(pos);
                    auto r = N -> getReceptor(N->getReceptorsCount()-1);
                    r -> doReset();
                    for (auto &jscope: jreceptor.value()["scopes"].items()) {
                        pos.clear();
                        for (auto &jposition: jscope.value().items()) {
                            pos.push_back(jposition.value().get<float>());
                        }
                        r -> doCreateNewScope();
                        r -> setPos(new indk::Position(nsize, pos));
                    }
                }
            }

            auto l = links.equal_range(nname);
            for (auto it = l.first; it != l.second; it++) {
                N -> doLinkOutput(it->second);
                if (indk::System::getVerbosityLevel() > 1) std::cout << nname << " -> " << it->second << std::endl;
            }

            N -> setName(nname);
            Neurons.insert(std::make_pair(nname, N));
        }

        if (indk::System::getVerbosityLevel() > 1) {
            for (const auto &e: Ensembles) {
                std::cout << e.first << " -";
                for (const auto &en: e.second) {
                    std::cout << " " << en;
                }
                std::cout << std::endl;
            }
        }

        if (InterlinkService && InterlinkService->isInterlinked()) {
            InterlinkService -> setStructure(Str);
        }
    } catch (std::exception &e) {
        if (indk::System::getVerbosityLevel() > 0) std::cerr << "Error parsing structure: " << e.what() << std::endl;
    }
}

/**
 * Set neural network to `learned` state.
 * @param LearnedFlag
 */
void indk::NeuralNet::setLearned(bool LearnedFlag) {
    for (const auto& N: Neurons) {
        N.second -> setLearned(LearnedFlag);
    }
}

/**
 * Enable neuron state synchronization.
 * @param enable
 */
void indk::NeuralNet::setStateSyncEnabled(bool enabled) {
    StateSyncEnabled = enabled;
}

/**
 * Check if neural network is in learned state.
 * @return
 */
bool indk::NeuralNet::isLearned() {
    for (const auto& N: Neurons) {
        if (!N.second -> isLearned()) return false;
    }
    return true;
}

/**
 * Get neuron by name.
 * @param NName Neuron name.
 * @return indk::Neuron object pointer.
 */
indk::Neuron* indk::NeuralNet::getNeuron(const std::string& NName) {
    auto N = Neurons.find(NName);
    if (N != Neurons.end()) return N->second;
    return nullptr;
}

/**
 * Get neuron list.
 * @return Vector of indk::Neuron object pointers.
 */
std::vector<indk::Neuron*> indk::NeuralNet::getNeurons() {
    std::vector<indk::Neuron*> neurons;
    for (auto &n: Neurons) {
        neurons.push_back(n.second);
    }
    return neurons;
}

/**
 * Get count of neurons in neural network.
 * @return Count of neurons.
 */
uint64_t indk::NeuralNet::getNeuronCount() {
    return Neurons.size();
}

/**
 * Get neural network structure in JSON format.
 * @return JSON string that contains neural network structure.
 */
std::string indk::NeuralNet::getStructure(bool minimized) {
    json j;

    for (const auto& e: Entries) {
        j["entries"].push_back(e.first);
    }

    for (const auto& n: Neurons) {
        json jn;
        jn["name"] = n.second -> getName();
        jn["size"] = n.second -> getXm();
        jn["dimensions"] = n.second -> getDimensionsCount();

        auto nentries = n.second -> getEntries();
        for (const auto& ne: nentries) {
            jn["input_signals"].push_back(ne);
        }
        for (const auto& en: Ensembles) {
            for (const auto& nen: en.second) {
                if (nen == n.second->getName()) {
                    jn["ensemble"] = en.first;
                    break;
                }
            }
        }

        for (int i = 0; i < n.second->getEntriesCount(); i++) {
            auto ne = n.second -> getEntry(i);
            for (int s = 0; s < ne->getSynapsesCount(); s++) {
                auto ns = ne -> getSynapse(s);
                json js;
                js["entry"] = i;
                js["k1"] = ns -> getk1();

                switch (ns->getNeurotransmitterType()) {
                    case 0:
                        js["neurotransmitter"] = "activation";
                        break;
                    case 1:
                        js["neurotransmitter"] = "deactivation";
                        break;
                }

                for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                    js["position"].push_back(ns->getPos()->getPositionValue(p));
                }
                jn["synapses"].push_back(js);
            }
        }

        for (int r = 0; r < n.second->getReceptorsCount(); r++) {
            auto nr = n.second ->getReceptor(r);
            json jr;
            jr["type"] = "single";
            for (int p = 0; p < n.second -> getDimensionsCount(); p++) {
                jr["position"].push_back(nr->getPos0()->getPositionValue(p));
            }

            json jscopes;
            auto scopes = nr -> getReferencePosScopes();
            for (const auto &s: scopes) {
                json jscope;
                for (int p = 0; p < s->getDimensionsCount(); p++) {
                    jscope.push_back(s->getPositionValue(p));
                }
                jscopes.push_back(jscope);
            }
            if (!scopes.empty())
                jr["scopes"] = jscopes;

            jn["receptors"].push_back(jr);
        }

        if (n.second->getLatency()) {
            jn["latency"] = n.second->getLatency();
        }

        j["neurons"].push_back(jn);
    }

    for (const auto& o: Outputs) {
        j["output_signals"].push_back(o);
    }

    j["name"] = Name;
    j["desc"] = Description;
    j["version"] = Version;

    if (indk::System::getVerbosityLevel() > 2) {
        std::cout << j.dump(4) << std::endl;
    }

    return minimized ? j.dump() : j.dump(4);
}

/**
 * Get neural network structure name.
 * @return String that contains name.
 */
std::string indk::NeuralNet::getName() {
    return Name;
}

/**
 * Get neural network structure description.
 * @return String that contains description.
 */
std::string indk::NeuralNet::getDescription() {
    return Description;
}

/**
 * Get neural network structure version.
 * @return String that contains version.
 */
std::string indk::NeuralNet::getVersion() {
    return Version;
}

/**
 * Get group of neurons by name.
 * @param ename Ensemble name.
 * @return Vector of indk::Neuron object pointers.
 */
std::vector<indk::Neuron*> indk::NeuralNet::getEnsemble(const std::string& ename) {
    auto e = Ensembles.find(ename);
    if (e != Ensembles.end()) {
        std::vector<indk::Neuron*> neurons;
        for (const auto& en: e->second) {
            neurons.push_back(getNeuron(en));
        }
        return neurons;
    }
    return {};
}

uint64_t indk::NeuralNet::getTotalParameterCount() {
    uint64_t count = 0;
    for (const auto& n: Neurons) {
        count += n.second->getReceptorsCount() * n.second->getDimensionsCount();
    }

    return count;
}

int indk::NeuralNet::getInstanceCount() {
    return InstanceManager.getInstanceCount();
}

indk::NeuralNet::~NeuralNet() {
    for (const auto& N: Neurons) delete N.second;
}
