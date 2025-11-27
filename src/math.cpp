/////////////////////////////////////////////////////////////////////////////
// Name:        math.cpp
// Purpose:     Basic mathematical methods
// Author:      Nickolay Babich
// Created:     07.11.2025
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <indk/math.h>
#include <cstring>

std::vector<float> indk::Math::doCompareCPFunction(std::vector<indk::Position*> CP, std::vector<indk::Position*> CPf) {
    std::vector<float> R;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    R.reserve(L);
    for (long long i = 0; i < L; i++) R.push_back(indk::Position::getDistance(CP[i], CPf[i]));
    return R;
}

float indk::Math::doCompareCPFunctionD(std::vector<indk::Position*> CP, std::vector<indk::Position*> CPf) {
    float R = 0;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    for (int i = 0; i < L; i++) R += indk::Position::getDistance(CP[i], CPf[i]);
    return R;
}

float indk::Math::doCompareFunction(indk::Position *R, indk::Position *Rf) {
    return indk::Position::getDistance(R, Rf);
}

void indk::Math::doClearPosition(float *position, uint64_t dimensions) {
    memset(position, 0, dimensions*sizeof(float));
}

void indk::Math::doAddPosition(float *position1, const float *position2, uint64_t dimensions) {
    for (int i = 0; i < dimensions; i++) {
        position1[i] += position2[i];
    }
}

float indk::Math::getGammaFunctionValue(float oG, float k1, float k2, float Xt) {
    float nGamma;
    nGamma = oG + (k1*Xt-oG/k2);
    return nGamma;
}

std::pair<float, float> indk::Math::getFiFunctionValue(float Lambda, float Gamma, float dGamma, float D) {
    float E = Lambda * std::exp(-Lambda*D);
    return std::make_pair(Gamma*E, dGamma*E);
}

float indk::Math::getReceptorInfluenceValue(bool active, const float *position, uint64_t dimensions) {
    float Yn = 0;
    float d = 0;
    for (int i = 0; i < dimensions; i++) {
        d += (position[i])*(position[i]);
    }
    d = std::sqrt(d);
    if (d > 0 && active) Yn = d; // TODO: research output variants
    return Yn;
}

float indk::Math::getRcValue(float k3, float Rs, float Fi, float dFi) {
    if (Fi >= Rs && dFi > 0) Rs += dFi / k3;
    else Rs = Rs / (k3*Rs+1);
    return Rs;
}

void indk::Math::getNewPosition(indk::Position *nRPos, indk::Position *R, indk::Position *S, float FiL, float D) {
    nRPos -> setPosition(R);
    nRPos -> doSubtract(S);
    nRPos -> doDivide(D);
    nRPos -> doMultiply(FiL);
}

void indk::Math::getNewReceptorPosition(float *buffer, const float *r, const float *s, float length, float d, uint64_t dimensions) {
    for (int i = 0; i < dimensions; i++) {
        buffer[i] += (r[i]-s[i]) / d * length;
    }
}

float indk::Math::getLambdaValue(unsigned int Xm) {
    return pow(10, -(log(Xm)/log(2)-6));
}

float indk::Math::getFiVectorLength(float dFi) {
    return sqrt(dFi);
}

float indk::Math::getDistance(const float *s, const float *r, uint64_t dimensions) {
    float d = 0;
    for (int i = 0; i < dimensions; i++) {
        d += (s[i]-r[i])*(s[i]-r[i]);
    }
    d = std::sqrt(d);
    return d;
}

bool indk::Math::isReceptorActive(float fi, float rs) {
    return fi >= rs;
}

float indk::Math::getSynapticSensitivityValue(unsigned int W, unsigned int OW) {
    float S = float(W) / OW;
    return 39.682*S*S*S*S - 170.22*S*S*S + 267.81*S*S - 178.8*S + 43.072;
}
