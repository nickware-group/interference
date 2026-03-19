"""
indk - Python bindings for the Interference neural network library.

Usage:
    import indk

    s = indk.structure({
        "entries": ["E1", "E2"],
        "neurons": [...],
        "output_signals": ["N1"],
    })
    net = indk.NeuralNet()
    net.set_structure(s)
    net.do_create_instance()
    net.do_learn([[1, 2, 3], [4, 5, 6]])
    net.do_recognise([[1, 2, 3], [4, 5, 6]])
    result = net.do_compare_patterns()
"""

import json as _json
import os as _os
import sys as _sys

# MinGW runtime DLLs need to be discoverable on Windows
if _sys.platform == "win32":
    _msys2_paths = [
        _os.path.join(_os.environ.get("MSYS2_ROOT", r"C:\msys64"), "ucrt64", "bin"),
        _os.path.join(_os.environ.get("MSYS2_ROOT", r"C:\msys64"), "mingw64", "bin"),
    ]
    for _p in _msys2_paths:
        if _os.path.isdir(_p):
            _os.add_dll_directory(_p)
            break

from ._indk import (  # noqa: F401
    # Version
    __version__,
    # Exception
    Error,
    # Enums
    ComputeBackend,
    CompareFlag,
    ProcessingMethod,
    ProcessingMode,
    OutputMode,
    ProfilerEvent,
    # Enum values (exported at module level by pybind11)
    NativeCPU,
    NativeCPUMultithread,
    OpenCL,
    Vulkan,
    Default,
    Normalized,
    Min,
    Average,
    # Data types
    OutputValue,
    ComputeBackendInfo,
    Position,
    # Classes
    Neuron,
    NeuralNet,
    # Module-level functions
    get_compute_backends_info,
    is_compute_backend_available,
    set_verbosity_level,
    get_verbosity_level,
    set_cpu_multithread_workers,
    set_opencl_device,
    set_vulkan_device,
    profiler_attach,
)


def structure(definition: dict) -> str:
    """Convert a Python dict to a JSON string for use with NeuralNet.set_structure().

    Analog of the C++ INDK_STRUCTURE macro.

    :param definition: Neural network structure as a Python dictionary.
    :return: JSON string ready to pass to NeuralNet.set_structure().
    """
    return _json.dumps(definition)
