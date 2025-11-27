/////////////////////////////////////////////////////////////////////////////
// Name:        error.cpp
// Purpose:     Exception system class
// Author:      Nickolay Babich
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/error.h>

#include <utility>

indk::Error::Error() {
    ET = 0;
}

indk::Error::Error(ExceptionType _ET) {
    ET = _ET;
}

indk::Error::Error(ExceptionType _ET, std::string _ED) {
    ET = _ET;
	ED = std::move(_ED);
}

const char* indk::Error::what() const noexcept {
    std::string Msg;

    switch (ET) {
        case EX_NEURALNET_NEURONS:
            Msg = std::string("EX_NEURALNET_NEURONS ~ Out of neuron list");
            break;
        case EX_NEURALNET_INPUT:
            Msg = std::string("EX_NEURALNET_INPUT ~ The number of input signals does not match the neural net entries count");
            break;
        case EX_NEURALNET_ENTRIES:
            Msg = std::string("EX_NEURALNET_ENTRIES ~ Out of neural net entries list");
            break;
        case EX_NEURALNET_NEURON_ENTRIES:
            Msg = std::string("EX_NEURALNET_NEURON_ENTRIES ~ The number of links more than the neuron entries count");
            break;
        case EX_NEURALNET_LINKTYPE:
            Msg = std::string("EX_NEURALNET_LINKTYPE ~ Unknown link type");
            break;
        case EX_NEURON_INPUT:
            Msg = std::string("EX_NEURON_INPUT ~ The number of input signals does not match the neuron entries count");
            break;
        case EX_NEURON_ENTRIES:
            Msg = std::string("EX_NEURON_ENTRIES ~ Out of entry list");
            break;
        case EX_NEURON_RECEPTORS:
            Msg = std::string("EX_NEURON_RECEPTORS ~ Out of receptor list");
            break;
        case EX_POSITION_OUT_RANGES:
			Msg = std::string("EX_POSITION_OUT_RANGES ~ Coordinates out of range");
            break;
        case EX_POSITION_RANGES:
            Msg = std::string("EX_POSITION_RANGES ~ Not equal coordinates ranges");
            break;
        case EX_POSITION_DIMENSIONS:
            Msg = std::string("EX_POSITION_DIMENSIONS ~ Not equal space dimensions of positions");
            break;
        case EX_INSTANCE_OUT_OF_RANGE:
            Msg = std::string("EX_INSTANCE_OUT_OF_RANGE ~ Compute Instance out of range (selected instance > total instances)");
            break;
        case EX_INSTANCE_BUSY:
            Msg = std::string("EX_INSTANCE_BUSY ~ This Compute Instance is busy");
            break;
        case EX_INSTANCE_MODEL_DATA_ERROR:
            Msg = std::string("EX_INSTANCE_MODEL_DATA_ERROR ~ Compute Instance model data is null");
            break;
        case EX_INSTANCE_TRANSLATION_NO_NEURONS:
            Msg = std::string("EX_INSTANCE_TRANSLATION_NO_NEURONS ~ No neurons for translation (selected instance > total instances)");
            break;
        case EX_INSTANCE_RUN_INPUT_ERROR:
            Msg = std::string("EX_INSTANCE_RUN_INPUT_ERROR ~ The number of received signals does not match the number of inputs");
            break;
        case EX_BACKEND_CL_DEVICE_NOT_FOUND:
            Msg = std::string("EX_BACKEND_CL_DEVICE_NOT_FOUND ~ OpenCL device not found");
            break;
        case EX_BACKEND_CL_KERNEL_BUILD_ERROR:
            Msg = std::string("EX_BACKEND_CL_KERNEL_BUILD_ERROR ~ OpenCL kernel build error");
            break;
        case EX_BACKEND_NATIVE_CPU_PROCESSING_ERROR:
            Msg = std::string("EX_BACKEND_NATIVE_CPU_PROCESSING_ERROR ~ No signal passing. Computing fell into endless deadloop");
            break;
        case EX_BACKEND_CONSISTENCY_ERROR:
            Msg = std::string("EX_BACKEND_CONSISTENCY_ERROR ~ The length of the signals is not the same");
            break;
        case EX_BACKEND_NOSIGNAL_ERROR:
            Msg = std::string("EX_BACKEND_NOSIGNAL_ERROR ~ No signal");
            break;
        default:
            Msg = std::string("No exception");
    }

    if (!ED.empty()) Msg += " ("+ED+")";

    auto S = new char[Msg.size()+1];
    sprintf(S, "%s", Msg.c_str());
    return S;
}
