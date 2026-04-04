# indk — Python bindings for Interference

## Prerequisites

**Linux:**
```bash
sudo apt-get install python3 python3-venv python3-dev g++ cmake git
```

**Windows:**
- Python 3.11+
- MSYS2 with `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake`
- Add `C:\msys64\ucrt64\bin` to PATH

## Build
The script creates a venv at the project root, builds and installs the package.
Check it with: `pip list` after activate venv

```bash
cd ./python
```
Options:
- `--opencl` - build with OpenCL Compute Backend support
- `--vulkan` - build with Vulkan Compute Backend support

#### Windows
Just run `build.cmd` script.

#### Linux
Just run `build.sh` script.



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

## Samples
**From project root:**
```bash
cd python/samples
python quick_start.py
python classification.py
python test.py
```
