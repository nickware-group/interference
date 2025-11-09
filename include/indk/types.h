/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     05.11.2025
// Copyright:   (c) NickWare Group
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_TYPES_H
#define INTERFERENCE_TYPES_H

#include <tuple>
#include <vector>
#include <map>

namespace indk {
    typedef std::tuple<std::string, std::string, void*, void*, int> LinkDefinition;
    typedef std::vector<LinkDefinition> LinkList;
    typedef std::map<std::string, std::vector<std::string>> StateSyncMap;

    typedef std::tuple<float, int> PatternDefinition;

    typedef struct compute_backends_info {
        int id;
        std::string backend_name;
        std::string translator_name;
        bool ready;
    } ComputeBackendsInfo;
}

#endif //INTERFERENCE_TYPES_H
