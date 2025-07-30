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
class Net
{

public:
    Net(int _nLayers, int *_nNeurons, int _nInputs, int _nInternalErrors);
    ~Net();
    enum propagationDirection
    {
        BACKWARD = 0,
        FORWARD = 1
    };
    void initNetwork(Neuron::weightInitMethod _wim, Neuron::biasInitMethod _bim, Neuron::actMethod _am);
    void setLearningRate(float _learningRate);
    void setInputs(const float *_inputs);
    void propInputs();
    void masterPropagate(std::vector<int> &injectionLayerIndex,
                         int _internalErrorIndex, propagationDirection _propDir,
                         float _controlError, Neuron::errorMethod _errorMethod, bool _doThread);
    void customBackProp(std::vector<int> &startLayerIndex,
                        int internalErrorIndex, float _controlError,
                        Neuron::errorMethod _errorMethod, bool _doThread);
    void customBackProp(std::vector<int> &startLayerIndex,
                        int internalErrorIndex, float _controlError,
                        Neuron::errorMethod _errorMethod);
    void customBackProp(cl_mem target_outputs_buffer);
    void customForwardProp(std::vector<int> &injectionLayerIndex,
                           int _internalErrorIndex, float _controlError,
                           Neuron::errorMethod _errorMethod);
    void updateWeights();
    Layer *getLayer(int _layerIndex);
    float getOutput(int _neuronIndex);
    float getSumOutput(int _neuronIndex);
    int getnLayers();
    int getnInputs();
    float getWeightDistance();
    float getLayerWeightDistance(int _layerIndex);
    float getWeights(int _layerIndex, int _neuronIndex, int _weightIndex);
    int getnNeurons();
    float getInputs(int _inputIndex);
    void saveWeights();
    void snapFistLayerWeights();
    void snapWeights();
    void printNetwork();

    cl_context context;
    cl_command_queue command_queue;

private:
    int nLayers = 0;
    int nNeurons = 0;
    int nInternalErrors = 0;
    int nWeights = 0;
    int nInputs = 0;
    int nOutputs = 0;
    float learningRate = 0;
    Layer **layers = nullptr;
    const float *inputs = nullptr;
    float controlError = 0;
    float echoError = 0;

    // OpenCL member variable
    cl_platform_id platform_id;
    cl_device_id device_id;

    cl_program program;

    cl_kernel forward_prop_kernel;
    cl_kernel backprop_error_kernel;
    cl_kernel update_weights_kernel;
    cl_kernel calculate_output_error_kernel;

    cl_mem net_input_buffer = nullptr   ;

    void initCL();
    void buildKernels();
};
