//
// Created by sama on 16/10/2021.
//

#ifndef CLDL_BPTHREAD_H
#define CLDL_BPTHREAD_H

#include "CppThread.h"
#include "Layer.h"
#include "Neuron.h"

class bpThread : public CppThread {

public:
    bpThread(int _threadIndex,
             int _layerIndex,
             Layer** _layers,
             bool _inject,
             double _controlError,
             int _internalErrorIndex,
             Neuron::errorMethod _errorMethod) {
        layers = _layers;
        layerIndex = _layerIndex;
        inject = _inject;
        controlError = _controlError;
        internalErrorIndex = _internalErrorIndex;
        threadIndex = _threadIndex;
        errorMethod = _errorMethod;
    }

private:
    void run();

private:
    int layerIndex = 0;
    Layer **layers = nullptr;
    bool inject = false;
    double tempError = 0.00;
    double tempWeight =0.00;
    double controlError = 0.00;
    int internalErrorIndex = 0;
    int threadIndex = 0;
    Neuron::errorMethod errorMethod;
};

#endif