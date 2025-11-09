/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     05.11.2025
// Copyright:   (c) NickWare Group
// Licence:
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
