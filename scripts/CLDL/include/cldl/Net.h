#pragma once

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <ctgmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <iostream>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include <numeric>
#include <vector>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

#include "Layer.h"
#include "bpThread.h"

/** Net is the main class used to set up a neural network used for
 * closed-loop Deep Learning. It initialises all the layers and the
 * neurons internally.
 *
 * (C) 2019,2020, Bernd Porr <bernd@glasgowneuro.tech>
 * (C) 2019,2020, Sama Daryanavard <2089166d@student.gla.ac.uk>
 *
 * GNU GENERAL PUBLIC LICENSE
 **/
class Net {

public:
    Net(int _nLayers, int *_nNeurons, int _nInputs, int _nInternalErrors);
    ~Net();
    enum propagationDirection {BACKWARD = 0 , FORWARD = 1};
    void initNetwork(Neuron::weightInitMethod _wim, Neuron::biasInitMethod _bim, Neuron::actMethod _am);
    void setLearningRate(double _learningRate);
    void setInputs(const double *_inputs);
    void propInputs();
    void masterPropagate(std::vector<int> &injectionLayerIndex,
                         int _internalErrorIndex, propagationDirection _propDir,
                         double _controlError, Neuron::errorMethod _errorMethod, bool _doThread);
    void customBackProp(std::vector<int> &startLayerIndex,
                        int internalErrorIndex, double _controlError,
                        Neuron::errorMethod _errorMethod,  bool _doThread);
    void customBackProp(std::vector<int> &startLayerIndex,
                        int internalErrorIndex, double _controlError,
                        Neuron::errorMethod _errorMethod);
    void customForwardProp(std::vector<int> &injectionLayerIndex,
                           int _internalErrorIndex, double _controlError,
                           Neuron::errorMethod _errorMethod);
    void updateWeights();
    Layer *getLayer(int _layerIndex);
    double getOutput(int _neuronIndex);
    double getSumOutput(int _neuronIndex);
    int getnLayers();
    int getnInputs();
    double getWeightDistance();
    double getLayerWeightDistance(int _layerIndex);
    double getWeights(int _layerIndex, int _neuronIndex, int _weightIndex);
    int getnNeurons();
    double getInputs(int _inputIndex);
    void saveWeights();
    void snapFistLayerWeights();
    void snapWeights();
    void printNetwork();

private:
    int nLayers = 0;
    int nNeurons = 0;
    int nInternalErrors = 0;
    int nWeights = 0;
    int nInputs = 0;
    int nOutputs = 0;
    double learningRate = 0;
    Layer **layers = nullptr;
    const double *inputs = nullptr;
    double controlError = 0;
    double echoError = 0;

    // OpenCL member variable
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;

    void initCL();
    void buildKernels();

};
