/////////////////////////////////////////////////////////////////////////////
// Name:        indk/error.h
// Purpose:     Exception system class header
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_ERROR_H
#define INTERFERENCE_ERROR_H

#include <iostream>
#include <exception>
#include <vector>
#include <string>

namespace indk {
    /// Error handler class.
    class Error: public std::exception {
    public:
        typedef unsigned int ExceptionType;
        /**
         * Exception types.
         */
        typedef enum {
            /// Out of neuron list.
            EX_NEURALNET_NEURONS,
            /// The number of input signals does not match the neural net entries count.
            EX_NEURALNET_INPUT,
            /// Out of neural net entries list.
            EX_NEURALNET_ENTRIES,
            /// The number of links more than the neuron entries count.
            EX_NEURALNET_NEURON_ENTRIES,
            EX_NEURALNET_LINKTYPE,
            /// The number of input signals does not match the neuron entries count.
            EX_NEURON_INPUT,
            /// Out of entry list.
            EX_NEURON_ENTRIES,
            /// Out of receptor list.
            EX_NEURON_RECEPTORS,
            /// Coordinates out of range.
            EX_POSITION_OUT_RANGES,
            /// Not equal coordinates ranges.
            EX_POSITION_RANGES,
            /// Not equal space dimensions of positions.
            EX_POSITION_DIMENSIONS,
            /// Compute Instance out of range (selected instance > total instances)
            EX_INSTANCE_OUT_OF_RANGE,
            /// Compute Instance is not ready
            EX_INSTANCE_BUSY,
            /// Compute Instance model data is null
            EX_INSTANCE_MODEL_DATA_ERROR,
            /// No neurons for translation
            EX_INSTANCE_TRANSLATION_NO_NEURONS,
            /// The number of received signals does not match the number of inputs
            EX_INSTANCE_RUN_INPUT_ERROR,
            /// OpenCL device not found
            EX_BACKEND_CL_DEVICE_NOT_FOUND,
        } Exceptions;

        Error();
        explicit Error(ExceptionType);
        explicit Error(ExceptionType, std::string);
        const char* what() const noexcept override;
    private:
        ExceptionType ET;
        std::string ED;
    };
}

#endif //INTERFERENCE_ERROR_H
