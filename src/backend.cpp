/////////////////////////////////////////////////////////////////////////////
// Name:        backend.cpp
// Purpose:     Compute Backend abstract class
// Author:      Nickolay Babich
// Created:     05.11.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/backend.h>

std::string indk::ComputeBackend::getBackendName() {
    return BackendName;
}

std::string indk::ComputeBackend::getTranslatorName() {
    return TranslatorName;
}

bool indk::ComputeBackend::isReady() {
    return Ready;
}
