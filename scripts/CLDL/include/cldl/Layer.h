#pragma once
#include <ctgmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <numeric>
#include <vector>
#include <CL/cl.h>

#include "Neuron.h"

class Layer {
public:
    // Layer(int _nNeurons, int _nInputs, int _numBuses);
    Layer(int _nNeurons, int _nInputs);
    ~Layer();

    friend class Net;

    void initLayer(int _layerIndex, Neuron::weightInitMethod _wim, Neuron::biasInitMethod _bim, Neuron::actMethod _am);
    void setLearningRate(float _learningRate);
    void setInputs(const float *_inputs);
    void propInputs(int _index, float _value);
    void calcOutputs();
    void setInternalErrors(int _internalErrorIndex, float _sumValue,
                           int _neuronIndex, Neuron::errorMethod _errorMethod);
    float getInternalErrors(int _internalErrorIndex, int _neuronIndex);
    void setErrorInputsAndCalculateInternalError(int _index, float _value,
                                                 int _internalErrorIndex,
                                                 Neuron::errorMethod _errorMethod);
    void updateWeights();
    Neuron *getNeuron(int _neuronIndex);
    int getnNeurons();
    float getOutput(int _neuronIndex);
    float getSumOutput(int _neuronIndex);
    float getWeights(int _neuronIndex, int _weightIndex);
    float getWeightChange();
    float getWeightDistance();
    float getInitWeight(int _neuronIndex, int _weightIndex);
    void saveWeights();
    void snapWeights();
    void printLayer();

private:
    // initialisation:
    int nNeurons = 0;
    int nInputs = 0;
    float learningRate = 0;
    int myLayerIndex = 0;
    Neuron **neurons = nullptr;
    int numBuses = 0;
    int layerHasReported = 0;
    const float *inputs = nullptr;
    float weightChange=0;

    cl_mem weights_buffer;
    cl_mem biases_buffer;
    cl_mem sum_outputs_buffer;
    cl_mem activated_outputs_buffer;
    cl_mem internal_errors_buffer;
};