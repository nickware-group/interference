/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     Classification sample main
// Author:      Nickolay Babich
// Created:     23.01.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuralnet.h>

int main() {
    // create the neural net structure with two entries and one neuron (one class)
    std::string structure = R"(
    {
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
    })";

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
