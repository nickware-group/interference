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
    // Creating
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

    indk::NeuralNet net;
    net.setStructure(structure);

    std::cout << "Model name            : " << net.getName() << std::endl;
    std::cout << "Model desc            : " << net.getDescription() << std::endl;
    std::cout << "Model ver             : " << net.getVersion() << std::endl;
    std::cout << "Neuron count          : " << net.getNeuronCount() << std::endl;
    std::cout << "Total parameter count : " << net.getTotalParameterCount() << std::endl;
    std::cout << std::endl;

    // learning the values
    net.doLearn({{4, 3, 2, 1}, {1, 2, 3, 4}});

    net.doRecognise({{4, 3, 2, 1}, {1, 2, 3, 4}});
    for (const auto &d: net.doComparePatterns()) std::cout << d << std::endl;

    net.doRecognise({{1, 2, 3, 4}, {1, 2, 3, 4}});
    for (const auto &d: net.doComparePatterns()) std::cout << d << std::endl;

    net.doRecognise({{1, 2, 3, 4}, {4, 3, 2, 1}});
    for (const auto &d: net.doComparePatterns()) std::cout << d << std::endl;

    return 0;
}
