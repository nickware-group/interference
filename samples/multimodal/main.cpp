/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     16.03.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <iomanip>
#include "indk/system.h"
#include "indk/neuralnet.h"
#include "bmp.hpp"

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

std::vector<std::vector<float>> doBuildInputVector(std::vector<BMPImage> &images, std::vector<std::string> &texts) {
    std::vector<std::vector<float>> input;
    for (int i = 0; i < images.size(); i++) {
        for (int s = 0; s < 2; s++) {
            std::vector<float> channels[4];
            for (int d = s; d < images[i].size(); d+=2) {
                float r = images[i][d+s][0];
                float g = images[i][d+s][1];
                float b = images[i][d+s][2];
                auto rgbn = std::vector<float>({r/255, g/255, b/255});
                auto HSI = RGB2HSI(rgbn[0], rgbn[1], rgbn[2]);
                channels[0].emplace_back(HSI[0]/(2*M_PI));
                channels[1].emplace_back(HSI[1]);
                channels[2].emplace_back(HSI[2]);
            }
            input.push_back(channels[0]);
            input.push_back(channels[1]);
            input.push_back(channels[2]);
        }

        auto words = texts[i];
        std::vector<float> winput;
        for (int d = 0; d < images[i].size()/2; d++) {
            if (d < words.size()) winput.emplace_back(float(words[d])/255);
            else winput.emplace_back(0);
        }

        input.push_back(winput);
    }

    return input;
}

int main() {
    constexpr uint8_t COUNT = 3;
    constexpr uint16_t IMAGE_SIZE = 128*128;
    constexpr char STRUCTURE_PATH[128] = "structures/structure.json";
    constexpr char IMAGES_PATH[128] = "images/";
    std::vector<std::string> names = {"mug with a melons", "mug with a big apple", "tomato"};
    std::vector<std::string> variants = {"mug with a melons", "mug wth a meln", "mug with a big apple", "mug w th a bi ap ple", "tomato", "tomto"};

    // load neural network structure from file
    auto NN = new indk::NeuralNet(STRUCTURE_PATH);

    // replicate neurons
    for (int i = 2; i <= COUNT; i++) NN -> doReplicateEnsemble("A1", "A"+std::to_string(i), true);
    NN -> doCreateInstance();

    std::cout << "Model name            : " << NN->getName() << std::endl;
    std::cout << "Model desc            : " << NN->getDescription() << std::endl;
    std::cout << "Model ver             : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count          : " << NN->getNeuronCount() << std::endl;
    std::cout << "Total parameter count : " << NN->getTotalParameterCount() << std::endl;
    std::cout << "Compute Instance count: " << NN->getInstanceCount() << std::endl;
    std::cout << std::endl;

    std::cout << "List of objects:" << std::endl;
    std::vector<BMPImage> images;
    for (int b = 1; b <= COUNT; b++) {
        auto image = doReadBMP(IMAGES_PATH+std::to_string(b)+".bmp");
        images.push_back(image);
        std::cout << b << ". Image [" << b << ".bmp] - " << names[b-1] << std::endl;
        if (image.size() != IMAGE_SIZE) {
            std::cout << "Error loading image " << b << ".bmp" << std::endl;
            return 1;
        }
    }
    auto input = doBuildInputVector(images, names);

    auto T = getTimestampMS();

    // teach the neural network
    NN -> doLearn(input);

    // detect
    std::cout << std::endl << "Detecting objects:" << std::endl;
    for (int b = 1; b <= COUNT; b++) {
        std::string name = std::to_string(b)+".bmp";
        auto image = doReadBMP(IMAGES_PATH+name);
        std::vector<BMPImage> rimages;
        for (int i = 1; i <= COUNT; i++) rimages.push_back(image);

        std::cout << "Image [" << b << ".bmp]" << std::endl;

        for (int e = 0; e < variants.size(); e++) {
            std::cout << std::setw(35) << std::left << std::to_string(e+1)+". "+variants[e];

            std::vector<std::string> rvariants;
            for (int i = 1; i <= COUNT; i++) rvariants.push_back(variants[e]);

            auto rinput = doBuildInputVector(rimages, rvariants);

            NN -> doRecognise(rinput);

            auto patterns = NN -> doComparePatterns(indk::PatternCompareFlags::CompareNormalized);
            auto r = std::max_element(patterns.begin(), patterns.end());
            if (std::distance(patterns.begin(), r) == b-1) {
                std::cout << "[CORRECT ANSWER]" << std::endl;
            } else {
                std::cout << "[INCORRECT ANSWER]" << std::endl;
                // Uncomment to print the response of output neurons to the input data (response - values [0, 1], 0 - minimum response, 1 - maximum response)
//                 std::cout << "Difference for outputs:" << std::endl;
//                 for (int i = 0; i < patterns.size(); i++) std::cout << (i+1) << ". " << patterns[i] << std::endl;
            }
        }
        std::cout << std::endl;
    }

    T = getTimestampMS() - T;
    std::cout << "Done in " << T << " ms" << std::endl;
    return 0;
}
