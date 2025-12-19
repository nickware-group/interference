<p align="center">
  <img width="750" height="200" src="https://nickware.group/repository/products/indk/logo.png"><br><br>
Cross-platform C++ library - universal neurobiology-based machine learning framework<br>
Version 3.0.0
</p>

----------------------------------------------------------------
### WHAT IS INTERFERENCE
Interference library is a cross-platform C++ library implementing a neurobiology-based `Interference` architecture. Unlike traditional artificial neural networks, Interference models neural processing using spatial mathematics, where neurons works in multidimensional space and process signals through biologically-inspired components: synapses, receptors, and neurotransmitters.

This framework is highly scalable. It is suitable for both powerful computing clusters and embedded systems, including real-time systems.

----------------------------------------------------------------
### KEY FEATURES
- Spatial neuron modeling with multidimensional positioning
- Synaptic signal transmission with activation/deactivation neurotransmitters
- Pattern recognition through receptor position adaptation
- Multiple computation backends - native CPU (single-thread/multi-thread), OpenCL
- JSON-based network structure definition
- Support for learning and inference modes (including on embedded devices)

----------------------------------------------------------------
### PLATFORMS
|                                | x86 | aarch64 | armle.v7 | PowerPC | e2k | MIPS |
|:------------------------------:|:---:|:-------:|:--------:|:-------:|:---:|:----:|
|          **Windows**           |  +  |         |          |         |     |      |
|           **Linux**            |  +  |    +    |    +     |         |  +  |      |
|            **QNX**             |  +  |         |    +     |         |     |      |
| **Secured RTOS KPDA 10964-01** |  +  |    +    |    +     |    +    |  +  |  +   |

----------------------------------------------------------------
### COMPUTING
- Native CPU (single thread)
- Native CPU (multithread)
- OpenCL

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
### SAMPLES
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
