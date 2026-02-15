<p align="center">
  <img width="750" height="200" src="https://nickware.group/repository/products/indk/logo.png"><br><br>
Cross-platform C++ library - universal neurobiology-based machine learning framework<br>
Version 3.1.0
</p>

[![Interference NDK (linux)](https://github.com/nickware-group/interference/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/nickware-group/interference/actions/workflows/cmake-multi-platform.yml)

----------------------------------------------------------------
### WHAT IS INTERFERENCE NDK
Interference NDK includes a cross-platform C++ library and tools for neural networks debugging. The library implements a neurobiology-based `Interference` architecture. Unlike traditional artificial neural networks, `Interference` models neural processing using spatial mathematics, where neurons works in multidimensional space and process signals through biologically-inspired components: synapses, receptors, and neurotransmitters.

This framework is highly scalable. It is suitable for both powerful computing clusters and embedded systems, including real-time systems.

----------------------------------------------------------------
### KEY FEATURES
- Spatial neuron modeling with multidimensional positioning
- Synaptic signal transmission with activation/deactivation neurotransmitters
- Pattern recognition through receptor position adaptation
- Multiple computation backends - native CPU (single-thread/multi-thread), OpenCL, Vulkan
- JSON-based network structure definition
- Support for learning and inference modes (including on embedded devices)
- Can be used to perform a variety of tasks: data classification, anomaly detection, image and sound recognition, NLP, etc

----------------------------------------------------------------
### PLATFORMS
|                                | x86 | aarch64 | armle.v7 |  e2k |
|:------------------------------:|:---:|:-------:|:--------:|:---:|
|          **Windows**           |  +  |    +    |          |     |
|           **Linux**            |  +  |    +    |    +     |  +  |
|            **QNX**             |  +  |         |    +     |     |

----------------------------------------------------------------
### COMPUTING
- Native CPU (single thread)
- Native CPU (multithread)
- OpenCL
- Vulkan (experimental)

----------------------------------------------------------------
### REQUIREMENTS
- CMake 3.12 or newer
- g++ 8.3.0 or newer (MinGW under Windows)

----------------------------------------------------------------
### LICENCE
Interference library is distributed under the MIT Licence.

interference_vision example uses the part of COIL-100 dataset.

"Columbia Object Image Library (COIL-100)," S. A. Nene, S. K. Nayar and H. Murase, Technical Report CUCS-006-96, February 1996.
http://www1.cs.columbia.edu/CAVE/software/softlib/coil-100.php

----------------------------------------------------------------
### QUICK START
A simple example of classifying signals. The two signals belong to the same class.
This example demonstrates how to determine how closely the recognized signals match the one learned by the model.

```c++
#include <indk/neuralnet.h>

int main() {
   // create the neural net structure with two entries and one neuron (one class)
    INDK_STRUCTURE(structure, {
      "entries": ["E1", "E2"],
      "neurons": [
        {
          "name": "N1",
          "size": 100,
          "dimensions": 2,
          "input_signals": ["E1", "E2"],
          "ensemble": "A1",

          "synapses": [
            {
              "type": "cluster",
              "position": [50, 50],
              "radius": 10,
              "neurotransmitter": "activation",
              "k1": 10
            }
          ],

          "receptors": [
            {
              "type": "cluster",
              "position": [50, 50],
              "count": 4,
              "radius": 5
            }
          ]
        }
      ],
      "output_signals": ["N1"],
      "name": "classification net",
      "desc": "neural net structure for data classification example",
      "version": "1.0"
    });

    indk::NeuralNet net;

    // load the structure from string
    net.setStructure(structure);

    // learn the values
    // send two signals (with length 4) to two neural network entries (E1 and E2)
    net.doLearn({{4, 3, 2, 1}, {1, 2, 3, 4}});

    // recognise the same signals
    // doComparePatterns method will give a vector with one value (since we only have one neuron at the output)
    // this value is 0 (recognised signal completely coincides with the learned signal)
    net.doRecognise({{4, 3, 2, 1}, {1, 2, 3, 4}});
    std::cout << net.doComparePatterns()[0] << std::endl;

    // recognise the signals, one of them has changed
    // doComparePatterns method will return a non-zero value
    net.doRecognise({{4, 3, 2, 2}, {1, 2, 3, 4}});
    std::cout << net.doComparePatterns()[0] << std::endl;

    // recognise the signals, the signals are swapped
    // doComparePatterns method will give a non-zero value, the value is greater than last time
    net.doRecognise({{1, 2, 3, 4}, {4, 3, 2, 1}});
    std::cout << net.doComparePatterns()[0] << std::endl;
    
    return 0;
}
```

Output:
```
0
0.177792
0.236595
```

----------------------------------------------------------------
### SAMPLES
- [Classification](samples/classification) - data classification example
- [Test](samples/test) - benchmark
- [Vision](samples/vision) - example of image recognition system
- [Multimodal](samples/multimodal) - example of multimodal (image+text) data processing

----------------------------------------------------------------
### HOW TO BUILD
Note that %INTERFERENCE_ROOT% is root directory of Interference library files, %BUILD_TYPE% is "debug" or "release". After the last step, library and binaries will be in %INTERFERENCE_ROOT%/lib and %INTERFERENCE_ROOT%/bin respectively.
#### Building for Windows (MinGW)
Run CMD and follow this steps:
1. Prepare build directory
```
cd %INTERFERENCE_ROOT%
mkdir cmake-build-%BUILD_TYPE%
cd cmake-build-%BUILD_TYPE%
```
2. Configure build files
```
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_MAKE_PROGRAM=make.exe -G "MinGW Makefiles" ..
```
3. Build and install the library
```
make install
```
#### Building for Linux
Run terminal and follow this steps:
1. Prepare build directory
```
cd %INTERFERENCE_ROOT%
mkdir cmake-build-%BUILD_TYPE%
cd cmake-build-%BUILD_TYPE%
```
2. Configure build files
```
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
```
3. Build and install the library
```
make install
```
