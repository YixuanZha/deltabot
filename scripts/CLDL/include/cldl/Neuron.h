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

using namespace std;
class Neuron {
public:
    Neuron(int _nInputs, int _nrawInternalErrors);
    ~Neuron();
    float getRawInternalErrors(int _internalErrorIndex);
    enum biasInitMethod {B_NONE = 0, B_RANDOM = 1};
    enum weightInitMethod {W_ZEROS = 0, W_ONES = 1, W_RANDOM = 2};
    enum actMethod {Act_Sigmoid = 0, Act_Tanh = 1, Act_NONE = 2};
    enum errorMethod {Value = 1, Absolute = 2, Sign = 3};
    void initNeuron(int _neuronIndex, int _layerIndex, weightInitMethod _wim,
                    biasInitMethod _bim, actMethod _am);
    void setLearningRate(float _learningRate);
    void setInput(int _index, float _value);
    void propInputs(int _index, float _value);
    int calcOutput(int _layerHasReported);
    void setInternalError(int _internalErrorIndex, float _sumValue,
                          errorMethod _errorMethod);
    void setErrorInputsAndCalculateInternalError(int _inputIndex,
                                                 float _value, int _internalErrorIndex,
                                                 errorMethod _errorMethod);
    void updateWeights();
    float doActivation(float _sum);
    float doActivationPrime(float _input);
    float getOutput();
    float getSumOutput();
    float getWeights(int _inputIndex);
    float getInputs(int _inputIndex);
    float getInitWeights(int _inputIndex);
    float getWeightChange();
    float getMaxWeight();
    float getMinWeight();
    float getSumWeight();
    float getWeightDistance();
    int getnInputs();

    void saveWeights();
    void printNeuron();
    inline void setWeight(int _index, float _weight) {
        assert((_index >= 0) && (_index < nInputs));
        weights[_index] = _weight;
    }

private:
    // initialisation:
    int nInputs = 0;
    float weightBoost = 1000;
    int myLayerIndex = 0;
    int myNeuronIndex = 0;
    float *initialWeights = nullptr;
    float learningRate = 0;
    float *inputErrors = nullptr;
    int numBuses = 0;
    float *rawInternalErrors = nullptr;
    float* internalErrors = nullptr;
    bool *busIsSet = nullptr;
    int countInputErrors = 0;
    int* busMethod = nullptr;
    float resultantInternalError = 1;
    
    int iHaveReported = 0;
    float *inputs = nullptr;
    float bias = 0;
    float sum = 0;
    float output = 0;

    float *weights = nullptr;
    float weightSum = 0;
    float maxWeight = 1;
    float minWeight = 1;
    float weightChange = 0;
    float weightsDifference = 0;
    int actMet = 0;
};
