/////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Purpose:     Vision sample main
// Author:      Nickolay Babich
// Created:     30.01.2023
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <iomanip>
#include <indk/system.h>
#include <indk/neuralnet.h>
#include "bmp.hpp"

constexpr uint8_t TEACH_COUNT = 15;
constexpr uint8_t TEST_COUNT = 15;
constexpr uint8_t TEST_ELEMENTS = 10;
constexpr uint8_t IMAGE_WIDTH = 128;
constexpr uint8_t IMAGE_HEIGHT = 128;
constexpr uint16_t IMAGE_SIZE = IMAGE_WIDTH*IMAGE_HEIGHT;
constexpr char STRUCTURE_PATH[128] = "structures/structure.json";
constexpr char IMAGES_TEACHING_PATH[128] = "images/learn/";
constexpr char IMAGES_TESTING_PATH[128] = "images/test/";

uint64_t getTimestampMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().
            time_since_epoch()).count();
}

std::vector<std::vector<float>> doBuildInputVector(std::vector<BMPImage> images) {
    std::vector<std::vector<float>> input;
    for (const auto &image: images) {
        for (int s = 0; s < 2; s++) {
            std::vector<float> channels[3];
            for (int d = s; d < image.size(); d+=2) {
                float r = image[d+s][0];
                float g = image[d+s][1];
                float b = image[d+s][2];
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
    }
    return input;
}

void doLog(const std::string& element, uint64_t time, float speed, bool endl = true) {
    std::stringstream s;
    if (time > 0) s << time << "ms";
    if (speed > 0) s << ", " << std::setprecision(2) << std::fixed << speed << " mbit/s";
    std::cout << "[" << std::setw(22) << std::left << std::setfill('.') << s.str() << "] " << std::setw(32) << std::setfill(' ') << element << "done ";
    if (endl) std::cout << std::endl;
}

void doRecognizeImages(indk::NeuralNet *NN, int start, int end, std::array<std::tuple<std::string, bool, uint64_t, float>, TEST_COUNT*TEST_ELEMENTS> &results, int instance) {
    uint64_t T;
    float S;

    for (int b = start-1; b <= end-1; b++) {
        for (int e = 0; e < TEST_ELEMENTS; e++) {
            std::string name = std::to_string(b+1)+"-"+std::to_string(e+1)+".bmp";
            auto image = doReadBMP(IMAGES_TESTING_PATH+name);
            auto rinput = doBuildInputVector({image});

            // recognizing
            T = getTimestampMS();
            NN -> doRecognise(rinput, true, {"E1", "E2", "E3", "E4", "E5", "E6"}, instance);
            T = getTimestampMS() - T;

            // computing speed
            S = (IMAGE_SIZE*24./1024/1024)*1000 / T;

            auto patterns = NN -> doComparePatterns(indk::PatternCompareFlags::CompareNormalized, indk::ScopeProcessingMethods::ProcessMin, instance);
            auto r = std::max_element(patterns.begin(), patterns.end());
            auto recognized = std::distance(patterns.begin(), r) == b;
            // Uncomment to print the response of output neurons to the input data (response - values [0, 1], 0 - minimum response, 1 - maximum response)
            // std::cout << "Difference for outputs:" << std::endl;
            // for (int i = 0; i < patterns.size(); i++) std::cout << "[" << (b+1) << "/" << (e+1) << "] " << (i+1) << ". " << patterns[i] << std::endl;

            std::tuple<std::string, bool, uint64_t, float> result = {name, recognized, T, S};
            results[b*TEST_ELEMENTS+e] = result;
        }
    }
}

int main() {
    uint64_t ttotal = 0;

    // loading neural network structure from file
    std::ifstream structure(STRUCTURE_PATH);
    auto NN = new indk::NeuralNet(STRUCTURE_PATH);

    // creating 3 instances, all of them use native CPU backend
    NN -> doCreateInstances(3);

    // setting additional parameters
    NN -> setStateSyncEnabled();

    std::cout << "Running Interlink Web..." << std::endl;
    NN -> doInterlinkWebInit("interlink-web/ui/", 8044);
    std::cout << "Now you can open http://localhost:8044 in your browser" << std::endl;

    std::cout << "Press ENTER to continue" << std::endl;
    getchar();

    // replicating neurons for classification
    for (int i = 2; i <= TEACH_COUNT; i++) NN -> doReplicateEnsemble("A1", "A"+std::to_string(i), true);

    std::cout << "Model name            : " << NN->getName() << std::endl;
    std::cout << "Model desc            : " << NN->getDescription() << std::endl;
    std::cout << "Model ver             : " << NN->getVersion() << std::endl;
    std::cout << "Neuron count          : " << NN->getNeuronCount() << std::endl;
    std::cout << "Total parameter count : " << NN->getTotalParameterCount() << std::endl;
    std::cout << "Compute Instance count: " << NN->getInstanceCount() << std::endl;
    std::cout << std::endl;

    // loading the images for the training
    std::vector<BMPImage> images;
    for (int b = 1; b <= TEACH_COUNT; b++) {
        auto image = doReadBMP(IMAGES_TEACHING_PATH+std::to_string(b)+".bmp");
        images.push_back(image);
        if (image.size() != IMAGE_SIZE) {
            std::cout << "Error loading image " << b << ".bmp" << std::endl;
            return 1;
        }
    }
    auto input = doBuildInputVector(images);
    doLog("Loading images", 0, 0);

    // teaching the neural network
    // only 15 images in the training dataset
    auto T = getTimestampMS();
    NN -> doLearn(input, true, {}, 0);
    T = getTimestampMS() - T;

    ttotal += T;
    auto S = (IMAGE_SIZE*TEACH_COUNT*24.f/1024/1024)*1000 / T;
    doLog("Teaching neural network", T, S);

    // recognizing all the 150 images in 3 instances in parallel
    // 50 images for each instance
    std::array<std::tuple<std::string, bool, uint64_t, float>, TEST_COUNT*TEST_ELEMENTS> results = {};
    T = getTimestampMS();

    std::thread worker1([NN, &results]() {
        doRecognizeImages(NN, 1, 5, results, 0);
    });

    std::thread worker2([NN, &results]() {
        doRecognizeImages(NN, 6, 10, results, 1);
    });

    std::thread worker3([NN, &results]() {
        doRecognizeImages(NN, 11, 15, results, 2);
    });

    worker1.join();
    worker2.join();
    worker3.join();

    T = getTimestampMS() - T;
    ttotal += T;
    float stotal = (TEST_COUNT*TEST_ELEMENTS*IMAGE_SIZE*24.f/1024/1024)*1000 / ttotal;

    // printing the results
    int rtotal = 0;
    for (const auto &result: results) {
        auto name = std::get<0>(result);
        auto recognized = std::get<1>(result);
        auto time = std::get<2>(result);
        auto speed = std::get<3>(result);

        doLog("Recognizing "+name, time, speed, false);

        if (recognized) {
            std::cout << "[RECOGNIZED]" << std::endl;
            rtotal++;
        } else std::cout << "[NOT RECOGNIZED]" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "================================== SUMMARY ===================================" << std::endl;
    std::cout << "Recognition accuracy: " << rtotal/float(TEST_COUNT*TEST_ELEMENTS) << " (" << rtotal << "/" << TEST_COUNT*TEST_ELEMENTS << ")" << std::endl;
    std::cout << "Recognition time: " << ttotal << " ms" << std::endl;
    std::cout << "Recognition speed: " << stotal << " mbit/s (" << 1000/(ttotal/float(TEST_COUNT*TEST_ELEMENTS))  << " FPS)" << std::endl;

    std::cout << "Press ENTER to exit" << std::endl;
    getchar();

    return 0;
}
