/////////////////////////////////////////////////////////////////////////////
// Name:        indk/interlink.h
// Purpose:     Interlink service class header
// Author:      Nickolay Babich
// Created:     19.07.2023
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_INTERLINK_H
#define INTERFERENCE_INTERLINK_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

namespace indk {
    /**
     * Interlink class.
     */
    class Interlink {
    private:
//        void *LinkedObject;
        void *WebServer;
        void *Input;
        void *Output;
        std::string Host;
        std::string InputPort, OutputPort;
        std::thread DataThread;
        std::thread WebThread;
        bool StructureUpdated;
        bool DataUpdated;
        std::atomic<bool> Interlinked;
        std::string Structure;
        std::vector<std::string> DataBatches;
        std::mutex StructureLock;
        std::mutex DataLock;

        void doInitOutput();

        void doSend(const std::string&, const std::string&);
    public:
        Interlink();
        void doInitWebServer(const std::string& path, int port);
        void doInitInput(int port, int timeout);
        void doUpdateStructure(const std::string&);
        void doUpdateModelData(const std::string&);
        void doUpdateMetrics(const std::string&);
        void setStructure(const std::string&);
        std::string getStructure();
        bool isInterlinked() const;
        ~Interlink();
    };
}

#endif //INTERFERENCE_INTERLINK_H
