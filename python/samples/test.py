import sys
import time
import math
from pathlib import Path

import indk

nn: indk.NeuralNet = indk.NeuralNet()
X: list[list[float]] = []
backends: list[indk.ComputeBackendInfo] = []

# path to C++ test structures (shared with C++ sample)
STRUCTURES_DIR = Path(__file__).resolve().parent.parent.parent / "samples" / "test" / "structures"


def get_timestamp_ms() -> int:
    return int(time.time() * 1000)


def do_check_available_backends() -> int:
    rcount = 0
    print("=== AVAILABLE COMPUTE BACKENDS ===")
    print(f"{'ID':<10}{'Backend name':<40}{'Translator name':<20}{'Ready':<20}")
    print("-" * 77)
    for info in backends:
        ready_str = "Yes" if info.ready else "No"
        print(f"{info.backend_id:<10}{info.backend_name:<40}{info.translator_name:<20}{ready_str:<20}")
        rcount += info.ready
    print("-" * 77)
    print()

    return rcount


def do_create_instances() -> None:
    for info in backends:
        if info.ready:
            nn.do_create_instance(info.backend_id)


def do_load_model(path: Path, size: int) -> None:
    nn.do_load_structure(str(path))

    for i in range(2, size + 1):
        nn.do_replicate_ensemble("A1", f"A{i}")

    print(f"Model name            : {nn.name}")
    print(f"Model desc            : {nn.description}")
    print(f"Model ver             : {nn.version}")
    print(f"Neuron count          : {nn.neuron_count}")
    print(f"Total parameter count : {nn.total_parameter_count}")
    print(f"Compute Instance count: {nn.instance_count}")
    print()


def do_test(ref: float, instance: int) -> int:
    t = get_timestamp_ms()
    Y = nn.do_learn(X, True, [], instance)
    t = get_timestamp_ms() - t
    print(f"{'done [' + str(t) + ' ms]':<20}", end="")

    passed = False
    for y in Y:
        if math.fabs(y.value - ref) > 1e-3:
            print("[FAILED]")
            print(f"Output value {y.value} is not {ref}")
            print()
            passed = False
            break
        else:
            passed = True

    if passed:
        print("[PASSED]")

    return int(passed)


def do_tests(name: str, ref: float) -> int:
    count = 0
    instance_id = 0

    for info in backends:
        if info.ready:
            nn.do_translate_to_instance([], instance_id)
            print(f"{name + ' (' + info.backend_name + '):':<60}", end="")
            count += do_test(ref, instance_id)
            instance_id += 1

    print()
    print()

    return count


def main() -> None:
    global backends, X

    backends = indk.get_compute_backends_info()
    backend_count: int = do_check_available_backends()

    structure_count = 2
    superstructure_test_reference_output = 0.0294
    benchmark_test_reference_output = 2.7622
    total_test_count = structure_count * backend_count

    # setting up parameters
    indk.set_cpu_multithread_workers(4)

    # indk.set_opencl_device("NVIDIA GeForce RTX 5070")

    indk.set_verbosity_level(1)

    count = 0

    do_create_instances()

    # creating data array
    X = [[50.0] * 170, [50.0] * 170]

    # running tests
    print("=== SUPERSTRUCTURE TEST ===")
    do_load_model(STRUCTURES_DIR / "structure_general.json", 100)
    count += do_tests("Superstructure test", superstructure_test_reference_output)

    print("=== BENCHMARK ===")
    do_load_model(STRUCTURES_DIR / "structure_bench.json", 10000)
    count += do_tests("Benchmark", benchmark_test_reference_output)

    print(f"Tests passed: [{count}/{total_test_count}]")

    if count != total_test_count:
        sys.exit(1)


if __name__ == "__main__":
    main()
