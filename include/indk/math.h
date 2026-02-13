/////////////////////////////////////////////////////////////////////////////
// Name:        indk/math.h
// Purpose:     Basic mathematical methods header
// Author:      Nickolay Babich
// Created:     17.10.2022
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_MATH_H
#define INTERFERENCE_MATH_H

#include <cstdint>
#include <queue>
#include <indk/position.h>

namespace indk {
    class Math {
    public:
        static std::vector<float> doCompareCPFunction(std::vector<indk::Position*>, std::vector<indk::Position*>);
        static float doCompareCPFunctionD(std::vector<indk::Position*>, std::vector<indk::Position*>);
        static float doCompareFunction(indk::Position*, indk::Position*);
        static void doClearPosition(float*, uint64_t dimensions);
        static void doAddPosition(float *position1, const float *position2, uint64_t dimensions);
        static float getGammaFunctionValue(float, float, float, float);
        static std::pair<float, float> getFiFunctionValue(float, float, float, float);
        static float getReceptorInfluenceValue(bool active, const float *position, uint64_t dimensions);
        static float getRcValue(float, float, float, float);
        static void getNewPosition(indk::Position*, indk::Position*, indk::Position*, float, float);
        static void getNewReceptorPosition(float *buffer, const float *r, const float *s, float length, float d, uint64_t dimensions);
        static float getLambdaValue(unsigned int);
        static float getFiVectorLength(float);
        static float getDistance(const float*, const float*, uint64_t dimensions);
        static bool isReceptorActive(float, float);
        static float getSynapticSensitivityValue(unsigned int, unsigned int);
    };
}

#endif //INTERFERENCE_MATH_H
