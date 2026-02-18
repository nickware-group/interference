import indk

def main() -> None:
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

if __name__ == "__main__":
    main()
