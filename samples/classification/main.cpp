/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     Binary classification sample main
// Author:      Nickolay Babich
// Created:     19.02.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuralnet.h>

int main() {
    // create the neural net structure with two entries and two neurons (one class per neuron)
    INDK_STRUCTURE(structure, {
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
    });

    // create the neural net object
    indk::NeuralNet net;

    // load the structure from string
    net.setStructure(structure);

    // print some model info
    std::cout << "Model name            : " << net.getName() << std::endl;
    std::cout << "Model desc            : " << net.getDescription() << std::endl;
    std::cout << "Model ver             : " << net.getVersion() << std::endl;
    std::cout << "Neuron count          : " << net.getNeuronCount() << std::endl;
    std::cout << "Total parameter count : " << net.getTotalParameterCount() << std::endl;
    std::cout << std::endl;

    // reference signals for learning
    std::vector<float> signal1 = {1, 2, 3, 4};
    std::vector<float> signal2 = {1, 1, 2, 3};

    // test signals for recognition
    // some elements differ from learning signals
    std::vector<float> test_signal1 = {0.5, 1.5, 3, 4};
    std::vector<float> test_signal2 = {1, 1, 2.5, 3.5};

    // learn the values
    // send two signals (with length 4) to two neural network entries (E1 and E2)
    // two classes are formed (signal1 -> neuron N1 creates class 1, signal2 -> neuron N2 creates class 2)
    net.doLearn({signal1, signal2});


    // recognise the same signals
    // doComparePatterns method will give a vector with two values (since we have two neurons at the output)

    // first value is 0 (the recognized signal fully complies with class 1)
    // the second value reflects the deviation compared to class 2
    net.doRecognise({signal1, signal1});
    std::cout << "Deviation from class 1: " << net.doComparePatterns()[0] << std::endl;
    std::cout << "Deviation from class 2: " << net.doComparePatterns()[1] << std::endl;
    std::cout << std::endl;

    // now second value is 0 (the recognized signal fully complies with class 2)
    net.doRecognise({signal2, signal2});
    std::cout << "Deviation from class 1: " << net.doComparePatterns()[0] << std::endl;
    std::cout << "Deviation from class 2: " << net.doComparePatterns()[1] << std::endl;
    std::cout << std::endl;


    // now recognize modified signals

    // recognized signal is more suitable for the class 1 than for class 2 (first deviation value is less than the second)
    net.doRecognise({test_signal1, test_signal1});
    std::cout << "Deviation from class 1: " << net.doComparePatterns()[0] << std::endl;
    std::cout << "Deviation from class 2: " << net.doComparePatterns()[1] << std::endl;
    std::cout << std::endl;

    // recognized signal is more suitable for the class 2 than for class 1 (second deviation value is less than the first)
    net.doRecognise({test_signal2, test_signal2});
    std::cout << "Deviation from class 1: " << net.doComparePatterns()[0] << std::endl;
    std::cout << "Deviation from class 2: " << net.doComparePatterns()[1] << std::endl;
    std::cout << std::endl;

    return 0;
}
