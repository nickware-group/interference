import indk


def main():
    # create the neural net structure with two entries and two neurons (one class per neuron)
    structure = indk.structure({
        "entries": ["E1", "E2"],
        "neurons": [
            {
                "name": "N1",
                "size": 100,
                "dimensions": 2,
                "input_signals": ["E1"],
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
                        "count": 2,
                        "radius": 5
                    }
                ]
            },
            {
                "name": "N2",
                "size": 100,
                "dimensions": 2,
                "input_signals": ["E2"],
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
                        "count": 2,
                        "radius": 5
                    }
                ]
            }
        ],
        "output_signals": ["N1", "N2"],
        "name": "binary classification net",
        "desc": "neural net structure for binary classification example",
        "version": "1.0"
    })

    # create the neural net object
    net = indk.NeuralNet()

    # load the structure from string
    net.set_structure(structure)

    # print some model info
    print(f"Model name            : {net.name}")
    print(f"Model desc            : {net.description}")
    print(f"Model ver             : {net.version}")
    print(f"Neuron count          : {net.neuron_count}")
    print(f"Total parameter count : {net.total_parameter_count}")
    print()

    # reference signals for learning
    signal1 = [1, 2, 3, 4]
    signal2 = [1, 1, 2, 3]

    # test signals for recognition
    # some elements differ from learning signals
    test_signal1 = [0.5, 1.5, 3, 4]
    test_signal2 = [1, 1, 2.5, 3.5]

    # learn the values
    # send two signals (with length 4) to two neural network entries (E1 and E2)
    # two classes are formed (signal1 -> neuron N1 creates class 1, signal2 -> neuron N2 creates class 2)
    net.do_learn([signal1, signal2])

    # recognise the same signals
    # do_compare_patterns method will give a list with two values (since we have two neurons at the output)

    # first value is 0 (the recognized signal fully complies with class 1)
    # the second value reflects the deviation compared to class 2
    net.do_recognise([signal1, signal1])
    print(f"Deviation from class 1: {net.do_compare_patterns()[0]}")
    print(f"Deviation from class 2: {net.do_compare_patterns()[1]}")
    print()

    # now second value is 0 (the recognized signal fully complies with class 2)
    net.do_recognise([signal2, signal2])
    print(f"Deviation from class 1: {net.do_compare_patterns()[0]}")
    print(f"Deviation from class 2: {net.do_compare_patterns()[1]}")
    print()

    # now recognize modified signals

    # recognized signal is more suitable for class 1 than for class 2 (first deviation value is less than the second)
    net.do_recognise([test_signal1, test_signal1])
    print(f"Deviation from class 1: {net.do_compare_patterns()[0]}")
    print(f"Deviation from class 2: {net.do_compare_patterns()[1]}")
    print()

    # recognized signal is more suitable for class 2 than for class 1 (second deviation value is less than the first)
    net.do_recognise([test_signal2, test_signal2])
    print(f"Deviation from class 1: {net.do_compare_patterns()[0]}")
    print(f"Deviation from class 2: {net.do_compare_patterns()[1]}")
    print()


if __name__ == "__main__":
    main()
