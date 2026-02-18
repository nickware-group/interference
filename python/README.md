# indk - Python bindings for Interference

Python bindings for the Interference neural network library via pybind11.

## Requirements

- Python >= 3.11
- CMake >= 3.15
- g++ 8.3.0 or newer (MinGW-w64 / MSYS2 on Windows)
- pybind11 (fetched automatically if not found)

### Windows prerequisites

Install [MSYS2](https://www.msys2.org/) and the UCRT64 toolchain:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake
```

Make sure `C:\msys64\ucrt64\bin` is in your system PATH (needed for `g++`, `mingw32-make`).

## Build & Install

```bash
cd python
pip install .
```

Or for development:

```bash
pip install -e .
```

### Manual CMake build (Windows / MinGW)

```bash
cd python
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DPython3_EXECUTABLE=<path-to-python.exe> -DPYTHON_EXECUTABLE=<path-to-python.exe> -DPython3_FIND_REGISTRY=NEVER -DPython3_FIND_STRATEGY=LOCATION ..
mingw32-make -j4
```

Replace `<path-to-python.exe>` with the full path to your Python interpreter, e.g.
`C:/Users/You/AppData/Local/Programs/Python/Python313/python.exe`.

Both `-DPython3_EXECUTABLE` and `-DPYTHON_EXECUTABLE` must be set because pybind11
uses the legacy FindPython module internally.

The built `_indk.cpXYZ-win_amd64.pyd` file will be in the build directory. Copy it to `indk/`:

```bash
copy build\_indk.cp313-win_amd64.pyd indk\
```

### Manual CMake build (Linux)

```bash
cd python
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

### Optional GPU backends

To enable OpenCL/Vulkan support, add flags to the cmake command:

```bash
cmake ... -DINDK_OPENCL_SUPPORT=ON -DINDK_VULKAN_SUPPORT=ON
```

## Quick Start

```python
import indk

# create the neural net structure with two entries and one neuron (one class)
structure: str = indk.structure({
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
})

net = indk.NeuralNet()

# load the structure from string
net.set_structure(structure)

# learn the values
# send two signals (with length 4) to two neural network entries (E1 and E2)
net.do_learn([[4, 3, 2, 1], [1, 2, 3, 4]])

# recognise the same signals
# do_compare_patterns method will give a vector with one value (since we only have one neuron at the output)
# this value is 0 (recognised signal completely coincides with the learned signal)
net.do_recognise([[4, 3, 2, 2], [1, 2, 3, 4]])
print(net.do_compare_patterns()[0])

# recognise the signals, the signals are swapped
# do_compare_patterns method will give a non-zero value, the value is greater than last time
net.do_recognise([[1, 2, 3, 4], [4, 3, 2, 1]])
print(net.do_compare_patterns()[0])
```