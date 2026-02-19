# indk - Python bindings for Interference

Python bindings for the [Interference](https://github.com/nickware-group/interference) neural network library via pybind11.

## Requirements

- Python >= 3.11
- CMake >= 3.15
- g++ 8.3.0 or newer (MinGW-w64 / MSYS2 on Windows)
- pybind11 (fetched automatically if not found)

## Installation

### Windows

Install [MSYS2](https://www.msys2.org/) and the UCRT64 toolchain:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake
```

Make sure `C:\msys64\ucrt64\bin` is in your system PATH.

**From project root:**

```bash
python -m venv .venv
.venv\Scripts\activate
cd python
```

**Set MinGW as the CMake generator:**

```bash
$env:CMAKE_GENERATOR = "MinGW Makefiles"
```

You can also enable additional backends:

```bash
$env:SKBUILD_CMAKE_ARGS = "-DINDK_OPENCL_SUPPORT=ON;-DINDK_VULKAN_SUPPORT=ON"
```

**Install package to venv:**

```bash
pip install . -v
```

**Verify installation:**

```bash
cd ..
python -c "import indk; print('Ok')"
```

### Linux

**From project root:**

```bash
python3 -m venv .venv
source .venv/bin/activate
cd python
```

**Install:**

```bash
pip install . -v
```

**Verify installation:**

```bash
cd ..
python -c "import indk; print('Ok')"
```

## Quick Start

```python
import indk

# create the neural net structure with two entries and one neuron
structure = indk.structure({
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
net.set_structure(structure)

# learn two signals
net.do_learn([[4, 3, 2, 1], [1, 2, 3, 4]])

# recognise with a slightly changed signal
net.do_recognise([[4, 3, 2, 2], [1, 2, 3, 4]])
print(net.do_compare_patterns()[0])

# recognise with swapped signals — larger deviation
net.do_recognise([[1, 2, 3, 4], [4, 3, 2, 1]])
print(net.do_compare_patterns()[0])
```

## Running Samples

**From project root:**

```bash
cd python/samples
python quick_start.py
python classification.py
python test.py
```
