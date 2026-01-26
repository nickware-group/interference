/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/receptor.cpp
// Purpose:     Neuron receptor class
// Author:      Nickolay Babich
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuron.h>
#include <indk/system.h>

indk::Neuron::Receptor::Receptor() {
	DefaultPos = new indk::Position();
    k3 = 0;
    Rs = 0.01;
    Fi = 0;
    dFi = 0;
}

indk::Neuron::Receptor::Receptor(const Receptor &R) {
    CP = R.getCP();
    CPf = R.getCPf();
	DefaultPos = new indk::Position(*R.getPos0());
    k3 = R.getk3();
    Rs = R.getSensitivityValue();
    Fi = 0;
    dFi = 0;
}

indk::Neuron::Receptor::Receptor(indk::Position *_RPos, float _k3) {
	DefaultPos = new indk::Position(*_RPos);
    k3 = _k3;
    Rs = 0.01;
    Fi = 0;
    dFi = 0;
}

void indk::Neuron::Receptor::doCreateNewScope() {
    auto pos = new indk::Position(*DefaultPos);
    Scope = ReferencePos.size();
    ReferencePos.push_back(pos);
}

void indk::Neuron::Receptor::doChangeScope(uint64_t scope) {
    if (ReferencePos.empty()) return;

    if (scope > ReferencePos.size()) {
        Scope = ReferencePos.size() - 1;
        return;
    }
    Scope = scope;
}

void indk::Neuron::Receptor::setPos(indk::Position *_RPos) {
    if (Scope >= ReferencePos.size()) return;
    ReferencePos[Scope] -> setPosition(_RPos);
}

void indk::Neuron::Receptor::setRs(float _Rs) {
    Rs = _Rs;
}

void indk::Neuron::Receptor::setk3(float _k3) {
    k3 = _k3;
}

std::vector<indk::Position*> indk::Neuron::Receptor::getCP() const {
    return CP;
}

std::vector<indk::Position*> indk::Neuron::Receptor::getCPf() const {
    return CPf;
}

indk::Position* indk::Neuron::Receptor::getPos() const {
    if (Scope >= ReferencePos.size()) return nullptr;
    return ReferencePos[Scope];
}

indk::Position* indk::Neuron::Receptor::getPos0() const {
    return DefaultPos;
}

std::vector<indk::Position*> indk::Neuron::Receptor::getReferencePosScopes() {
    return ReferencePos;
}

float indk::Neuron::Receptor::getRs() const {
    return Rs;
}

float indk::Neuron::Receptor::getk3() const {
    return k3;
}

float indk::Neuron::Receptor::getFi() {
    return Fi;
}

float indk::Neuron::Receptor::getdFi() {
    return dFi;
}

float indk::Neuron::Receptor::getSensitivityValue() const {
    return Rs;
}
