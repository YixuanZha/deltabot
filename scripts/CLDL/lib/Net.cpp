#include "../include/cldl/Net.h"
#include "../include/cldl/Layer.h"
#include "../include/cldl/Neuron.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <fstream>
#include <string>
using namespace std;

void checkError(cl_int err, const char *operations)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error during:" << operations << "(error code: " << err << ")" << std::endl;
        exit(1);
    }
}

//*************************************************************************************
// initialisation:
//*************************************************************************************

Net::Net(int _nLayers, int *_nNeurons, int _nInputs, int _nInternalErrors)
{
    cout << "*******************************************************************************************************" << endl;
    nLayers = _nLayers; // no. of layers including inputs and outputs layers
    layers = new Layer *[nLayers];
    nInternalErrors = _nInternalErrors;
    int *nNeuronsp = _nNeurons; // number of neurons in each layer
    nInputs = _nInputs;         // the no. of inputs to the network (i.e. the first layer)
    // cout << "nInputs: " << nInputs << endl;
    int nInput = 0; // temporary variable to use within the scope of for loop

    std::cout << "Initializing OpenCL environment" << std::endl;
    initCL();

    for (int i = 0; i < nLayers; i++)
    {
        int numNeurons = *nNeuronsp; // no.
        // neurons in this layer
        if (i == 0)
        {
            nInput = nInputs;
        }
        /* no. inputs to the first layer is equal to no. inputs to the network */
        layers[i] = new Layer(numNeurons, nInput);
        nNeurons += numNeurons;
        nWeights += (numNeurons * nInput);
        nInput = numNeurons;
        /*no. inputs to the next layer is equal to the number of neurons
         * in the current layer. */
        nNeuronsp++; // point to the no. of neurons in the next layer
    }
    nOutputs = layers[nLayers - 1]->getnNeurons();
    for (int i = 0; i < nLayers; i++)
    {
        int numNeurons = *nNeuronsp;
        if (i == 0)
        {
            nInput = nInputs;
        }

        layers[i] = new Layer(numNeurons, nInput);

        cl_int err;
        size_t weights_size = sizeof(float) * numNeurons * nInput;
        size_t biases_size = sizeof(float) * numNeurons;

        std::vector<float> temp_weights(numNeurons * nInput);
        std::vector<float> temp_biases(numNeurons);
        for (size_t j = 0; j < temp_weights.size(); ++j)
            temp_weights[j] = ((float)rand() / (RAND_MAX));
        for (size_t j = 0; j < temp_biases.size(); ++j)
            temp_biases[j] = ((float)rand() / (RAND_MAX));

        layers[i]->weights_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, weights_size, temp_weights.data(), &err);
        checkError(err, "Weights buffer creation");
        layers[i]->biases_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, biases_size, temp_biases.data(), &err);
        checkError(err, "Biases buffer creation");

        layers[i]->sum_outputs_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, biases_size, NULL, &err);
        checkError(err, "Sum outputs buffer creation");
        layers[i]->activated_outputs_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, biases_size, NULL, &err);
        checkError(err, "Activated outputs buffer creation");
        layers[i]->internal_errors_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, biases_size, NULL, &err);
        checkError(err, "Internal errors buffer creation");

        nInput = numNeurons;
        nNeuronsp++;
    }
    nOutputs = layers[nLayers - 1]->getnNeurons();

    buildKernels();
    std::cout << "OpenCl envirnment ready" << std::endl;
}

Net::~Net()
{
    for (int i = 0; i < nLayers; i++)
    {
        delete layers[i];
    }
    delete[] layers;

    std::cout << "Releasing OpenCL resources...." << std::endl;

    clReleaseMemObject(net_input_buffer);
    clReleaseKernel(forward_prop_kernel);
    clReleaseKernel(backprop_error_kernel);
    clReleaseKernel(update_weights_kernel);
    clReleaseKernel(calculate_output_error_kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}

void Net::initCL()
{
    cl_int err;

    cl_uint num_platform;
    clGetPlatformIDs(0, NULL, &num_platform);
    if (num_platform == 0)
    {
        std::cerr << "No OpenCL platform found" << std::endl;
        exit(1);
    }
    std::vector<cl_platform_id> platforms(num_platform);
    clGetPlatformIDs(num_platform, platforms.data(), NULL);
    this->platform_id = platforms[0];

    cl_uint num_devices;
    clGetDeviceIDs(this->platform_id, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
    if (num_devices == 0)
    {
        std::cerr << "No GPU device found" << std::endl;
        exit(1);
    }
    std::vector<cl_device_id> devices(num_devices);
    clGetDeviceIDs(this->platform_id, CL_DEVICE_TYPE_GPU, num_devices, devices.data(), NULL);
    this->device_id = devices[0];

    this->context = clCreateContext(NULL, 1, &this->device_id, NULL, NULL, &err);
    checkError(err, "Net::initCL - Context creation");

    this->command_queue = clCreateCommandQueueWithProperties(this->context, this->device_id, NULL, &err);
}

void Net::buildKernels()
{
    cl_int err;

    std::ifstream kernel_file("cldl_kernels.cl"); // read kernel file (cldl_kernel.cl)
    if (!kernel_file.is_open())
    {
        std::cerr << "Could not open the kernel file 'cldl_kernels.cl'" << std::endl;
        exit(1);
    }
    std::string source_str(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
    const char *source = source_str.c_str();

    this->program = clCreateProgramWithSource(this->context, 1, &source, NULL, &err);
    checkError(err, "Net::buildKernels - Program creation");

    err = clBuildProgram(this->program, 1, &this->device_id, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t log_size;
        clGetProgramBuildInfo(this->program, this->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(this->program, this->device_id, CL_PROGRAM_BUILD_LOG, log_size, log.data(), NULL);
        std::cerr << log.data() << std::endl;
        exit(1);
    }

    this->forward_prop_kernel = clCreateKernel(this->program, "forward_prop_kernel", &err);
    checkError(err, "Kernel creation: forward_prop_kernel");
    this->backprop_error_kernel = clCreateKernel(this->program, "backprop_error_kernel", &err);
    checkError(err, "Kernel creation: backprop_error_kernel");
    this->update_weights_kernel = clCreateKernel(this->program, "update_weights_kernel", &err);
    checkError(err, "Kernel creation: update_weights_kernel");
    this->calculate_output_error_kernel = clCreateKernel(this->program, "calculate_output_error_kernel", &err);
    checkError(err, "Kernel creation: calculate_output_error_kernel");
}

void Net::initNetwork(Neuron::weightInitMethod _wim,
                      Neuron::biasInitMethod _bim, Neuron::actMethod _am)
{
    for (int i = 0; i < nLayers; i++)
    {
        layers[i]->initLayer(i, _wim, _bim, _am);
    }
}

void Net::setLearningRate(float _learningRate)
{
    learningRate = _learningRate;
    for (int i = 0; i < nLayers; i++)
    {
        layers[i]->setLearningRate(learningRate);
    }
}

//*************************************************************************************
// forward propagation of inputs:
//*************************************************************************************

void Net::setInputs(const float *_inputs)
{
    // inputs = _inputs;
    // layers[0]->setInputs(inputs); // sets the inputs to the first layer only
    cl_int err;
    if (!net_input_buffer)
    {
        net_input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * nInputs, NULL, &err);
        checkError(err, "Net input buffer creation");
    }

    err = clEnqueueWriteBuffer(command_queue, net_input_buffer, CL_TRUE, 0, sizeof(float) * nInputs, _inputs, 0, NULL, NULL);
    checkError(err, "Write to net input buffer");
}

void Net::propInputs()
{
    // for (int i = 0; i < nLayers - 1; i++)
    // {
    //     layers[i]->calcOutputs();
    //     for (int j = 0; j < layers[i]->getnNeurons(); j++)
    //     {
    //         float inputOuput = layers[i]->getOutput(j);
    //         layers[i + 1]->propInputs(j, inputOuput);
    //     }
    // }
    // layers[nLayers - 1]->calcOutputs();
    /* this calculates the final output of the network,
     * i.e. the output of the final layer
     * but this is not fed into any further layer*/
    cl_int err;
    cl_mem current_input_buffer = this->net_input_buffer;

    for (int i = 0; i < nLayers; i++)
    {
        Layer *current_layer = layers[i];

        err = clSetKernelArg(forward_prop_kernel, 0, sizeof(cl_mem), &current_input_buffer);
        err |= clSetKernelArg(forward_prop_kernel, 1, sizeof(cl_mem), &current_layer->weights_buffer);
        err |= clSetKernelArg(forward_prop_kernel, 2, sizeof(cl_mem), &current_layer->biases_buffer);
        err |= clSetKernelArg(forward_prop_kernel, 3, sizeof(cl_mem), &current_layer->sum_outputs_buffer);
        err |= clSetKernelArg(forward_prop_kernel, 4, sizeof(cl_mem), &current_layer->activated_outputs_buffer);
        err |= clSetKernelArg(forward_prop_kernel, 5, sizeof(int), &current_layer->nInputs);

        size_t global_work_size = current_layer->getnNeurons();

        err = clEnqueueNDRangeKernel(command_queue, forward_prop_kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);

        current_input_buffer = current_layer->activated_outputs_buffer;
    }
    clFinish(command_queue);
}

void Net::masterPropagate(std::vector<int> &injectionLayerIndex,
                          int _internalErrorIndex, propagationDirection _propDir,
                          float _controlError, Neuron::errorMethod _errorMethod, bool _doThread)
{
    switch (_propDir)
    {
    case BACKWARD:
        std::sort(injectionLayerIndex.rbegin(), injectionLayerIndex.rend());
        if (_doThread)
        {
            customBackProp(injectionLayerIndex, _internalErrorIndex,
                           _controlError, _errorMethod, _doThread);
        }
        else
        {
            customBackProp(injectionLayerIndex, _internalErrorIndex,
                           _controlError, _errorMethod);
        }
        break;
    case FORWARD:
        std::sort(injectionLayerIndex.begin(), injectionLayerIndex.end());
        customForwardProp(injectionLayerIndex, _internalErrorIndex,
                          _controlError, _errorMethod);
        break;
    }
}

//*************************************************************************************
// forward propagation of error:
//*************************************************************************************
void Net::customForwardProp(std::vector<int> &injectionLayerIndex,
                            int _internalErrorIndex, float _controlError,
                            Neuron::errorMethod _errorMethod)
{
    assert(injectionLayerIndex[0] == 0 && "Forward propagation must start form the first layer, include (0) in your array");
    int injectionCount = 0;
    int nextInjectionLayerIndex = injectionLayerIndex[0];
    controlError = _controlError;
    for (int i = 0; i < layers[nextInjectionLayerIndex]->getnNeurons(); i++)
    {
        layers[nextInjectionLayerIndex]->setInternalErrors(_internalErrorIndex,
                                                           controlError, i, _errorMethod); // setting the internal errors in the first layer
    }
    float inputOutput = 0.00;
    for (int layerIndex = nextInjectionLayerIndex; layerIndex < nLayers - 1; layerIndex++)
    {
        for (int N_index = 0; N_index < layers[layerIndex]->getnNeurons(); N_index++)
        {
            if (layerIndex == nextInjectionLayerIndex)
            {
                assert((injectionCount < nLayers) && (injectionCount >= 0) && "NET failed");
                inputOutput = controlError;
                injectionCount += 1;
                nextInjectionLayerIndex = injectionLayerIndex[injectionCount];
            }
            else
            {
                inputOutput = layers[layerIndex]->getInternalErrors(_internalErrorIndex, N_index);
            }
            layers[layerIndex + 1]->setErrorInputsAndCalculateInternalError(N_index,
                                                                            inputOutput, _internalErrorIndex,
                                                                            _errorMethod);
        }
    }
}

void Net::customBackProp(std::vector<int> &injectionLayerIndex,
                         int _internalErrorIndex, float _controlError,
                         Neuron::errorMethod _errorMethod, bool _doThread)
{
    assert(injectionLayerIndex[0] == nLayers - 1 && "Backpropagation must start form the last layer, include (Nlayers - 1) in your array");
    int nextInjectionLayerIndex = injectionLayerIndex[0];
    cout << nextInjectionLayerIndex << endl;
    int injectionCount = 0;
    controlError = _controlError;
    for (int neuronIndex = 0; neuronIndex < layers[nextInjectionLayerIndex]->getnNeurons(); neuronIndex++)
    { // set the internal error in the final layer
        layers[nextInjectionLayerIndex]->setInternalErrors(_internalErrorIndex,
                                                           controlError, neuronIndex, _errorMethod);
    }
    bool inject = false;
    for (int layerIndex = nextInjectionLayerIndex; layerIndex > 0; layerIndex--)
    { // iterate through the layers
        cout << "working on layer: " << layerIndex << endl;
        if (layerIndex == nextInjectionLayerIndex)
        {
            inject = true;
            injectionCount += 1;
            nextInjectionLayerIndex = injectionLayerIndex[injectionCount];
            assert((nextInjectionLayerIndex <= nLayers) && (nextInjectionLayerIndex >= 0) && "Net failed");
        }
        else
        {
            inject = false;
        }
        bpThread **myBPThread = nullptr;
        int totalThreads = layers[layerIndex - 1]->getnNeurons();
        myBPThread = new bpThread *[totalThreads];

        for (int threadIndex = 0; threadIndex < totalThreads; threadIndex++)
        {
            myBPThread[threadIndex] = new bpThread(threadIndex, layerIndex, layers, inject,
                                                   controlError, _internalErrorIndex, _errorMethod);
        }
        for (int i = 0; i < totalThreads; i++)
        {
            myBPThread[i]->start();
        }
        for (int i = 0; i < totalThreads; i++)
        {
            myBPThread[i]->join();
            delete myBPThread[i];
        }
        delete myBPThread;
    }
}

void Net::customBackProp(std::vector<int> &injectionLayerIndex,
                         int _internalErrorIndex, float _controlError,
                         Neuron::errorMethod _errorMethod)
{
    assert(injectionLayerIndex[0] == nLayers - 1 && "Backpropagation must start form the last layer, include (Nlayers - 1) in your array");
    float tempError = 0;
    float tempWeight = 0;
    int nextInjectionLayerIndex = injectionLayerIndex[0];
    int injectionCount = 0;
    controlError = _controlError;
    for (int neuronIndex = 0; neuronIndex < layers[nextInjectionLayerIndex]->getnNeurons(); neuronIndex++)
    { // set the internal error in the final layer
        layers[nextInjectionLayerIndex]->setInternalErrors(_internalErrorIndex,
                                                           controlError, neuronIndex, _errorMethod);
    }
    for (int layerIndex = nextInjectionLayerIndex; layerIndex > 0; layerIndex--)
    { // iterate through the layers
        for (int wn_index = 0; wn_index < layers[layerIndex - 1]->getnNeurons(); wn_index++)
        { // iterate through the inputs to each layer
            float thisSum = 0.00;
            for (int n_index = 0; n_index < layers[layerIndex]->getnNeurons(); n_index++)
            { // iterate through the neurons of each layer
                if (layerIndex == nextInjectionLayerIndex)
                {
                    assert((injectionCount <= nLayers) && (injectionCount >= 0) && "NET failed");
                    tempError = controlError;
                    injectionCount += 1;
                    nextInjectionLayerIndex = injectionLayerIndex[injectionCount];
                }
                else
                {
                    tempError = layers[layerIndex]->getInternalErrors(_internalErrorIndex, n_index);
                }
                tempWeight = layers[layerIndex]->getWeights(n_index, wn_index);
                thisSum += (tempError * tempWeight);
            }
            assert(std::isfinite(thisSum) && "NET failed");
            layers[layerIndex - 1]->setInternalErrors(_internalErrorIndex, thisSum,
                                                      wn_index, _errorMethod);
        }
    }
}

void Net::customBackProp(cl_mem target_outputs_buffer)
{
    cl_int err;

    // Calculate the error of output layer
    Layer *last_layer = layers[nLayers - 1];

    // Set kernel parameter for the calculation
    err = clSetKernelArg(calculate_output_error_kernel, 0, sizeof(cl_mem), &last_layer->activated_outputs_buffer);
    err |= clSetKernelArg(calculate_output_error_kernel, 1, sizeof(cl_mem), &target_outputs_buffer);
    err |= clSetKernelArg(calculate_output_error_kernel, 2, sizeof(cl_mem), &last_layer->sum_outputs_buffer);
    err |= clSetKernelArg(calculate_output_error_kernel, 3, sizeof(cl_mem), &last_layer->internal_errors_buffer);

    // execute the kernel
    size_t global_work_size = last_layer->getnNeurons();
    err = clEnqueueNDRangeKernel(command_queue, calculate_output_error_kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
    checkError(err, "Enqueue calculate_output_error_kernel");

    for (int i = nLayers - 2; i >= 0; i--)
    {
        Layer *current_layer = layers[i];
        Layer *next_layer = layers[i + 1];

        err = clSetKernelArg(backprop_error_kernel, 0, sizeof(cl_mem), &next_layer->internal_errors_buffer);
        err |= clSetKernelArg(backprop_error_kernel, 1, sizeof(cl_mem), &next_layer->weights_buffer);
        err |= clSetKernelArg(backprop_error_kernel, 2, sizeof(cl_mem), &current_layer->sum_outputs_buffer);
        err |= clSetKernelArg(backprop_error_kernel, 3, sizeof(cl_mem), &current_layer->internal_errors_buffer);
        err |= clSetKernelArg(backprop_error_kernel, 4, sizeof(cl_mem), &next_layer->nNeurons);
        err |= clSetKernelArg(backprop_error_kernel, 5, sizeof(cl_mem), &current_layer->nNeurons);

        // execute the kernel
        global_work_size = current_layer->getnNeurons();
        err = clEnqueueNDRangeKernel(command_queue, backprop_error_kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
    }
    clFinish(command_queue);
}

void Net::updateWeights()
{
    // for (int i = nLayers - 1; i >= 0; i--)
    // {
    //     layers[i]->updateWeights();
    // }

    cl_int err;
    cl_mem current_input_buffer = this->net_input_buffer;

    for (int i = 0; i < nLayers; i++)
    {
        Layer *current_layer = layers[i];

        err = clSetKernelArg(update_weights_kernel, 0, sizeof(cl_mem), &current_layer->internal_errors_buffer);
        err = clSetKernelArg(update_weights_kernel, 1, sizeof(cl_mem), &current_input_buffer);
        err = clSetKernelArg(update_weights_kernel, 2, sizeof(cl_mem), &current_layer->weights_buffer);
        err = clSetKernelArg(update_weights_kernel, 3, sizeof(cl_mem), &current_layer->biases_buffer);
        err = clSetKernelArg(update_weights_kernel, 4, sizeof(cl_mem), &learningRate);
        err = clSetKernelArg(update_weights_kernel, 5, sizeof(cl_mem), &current_layer->nInputs);
        err = clSetKernelArg(update_weights_kernel, 6, sizeof(cl_mem), &current_layer->nNeurons);

        size_t global_work_size[2] = {(size_t)current_layer->getnNeurons(), (size_t)current_layer->getnNeurons()};

        err = clEnqueueNDRangeKernel(command_queue, update_weights_kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

        current_input_buffer = current_layer->activated_outputs_buffer;
    }
    clFinish(command_queue);
}
//*************************************************************************************
// getters:
//*************************************************************************************

float Net::getOutput(int _neuronIndex)
{
    // return (layers[nLayers - 1]->getOutput(_neuronIndex));
    Layer *last_layer = layers[nLayers - 1];

    if (_neuronIndex < 0 || _neuronIndex >= last_layer->getnNeurons())
    {
        std::cerr << "ERROR: Neuron index out of bounds in gerOutput" << std::endl;
        return 0.0;
    }

    std::vector<float> host_output(last_layer->getnNeurons());

    cl_int err = clEnqueueReadBuffer(command_queue, last_layer->activated_outputs_buffer, CL_TRUE, 0, sizeof(float) * last_layer->getnNeurons(), host_output.data(), 0, NULL, NULL);
    checkError(err, "Read final output buffer in getOutput");

    return host_output[_neuronIndex];
}

float Net::getSumOutput(int _neuronIndex)
{
    return (layers[nLayers - 1]->getSumOutput(_neuronIndex));
}

int Net::getnLayers()
{
    return (nLayers);
}

int Net::getnInputs()
{
    return (nInputs);
}

Layer *Net::getLayer(int _layerIndex)
{
    assert(_layerIndex < nLayers && "NET failed");
    return (layers[_layerIndex]);
}

float Net::getWeightDistance()
{
    float weightChange = 0;
    float weightDistance = 0;
    for (int i = 0; i < nLayers; i++)
    {
        weightChange += layers[i]->getWeightChange();
    }
    weightDistance = sqrt(weightChange);
    // cout<< "Net: WeightDistance is: " << weightDistance << endl;
    return (weightDistance);
}

float Net::getLayerWeightDistance(int _layerIndex)
{
    return layers[_layerIndex]->getWeightDistance();
}

float Net::getWeights(int _layerIndex, int _neuronIndex, int _weightIndex)
{
    float weight = layers[_layerIndex]->getWeights(_neuronIndex, _weightIndex);
    return (weight);
}

int Net::getnNeurons()
{
    return (nNeurons);
}

float Net::getInputs(int _inputIndex)
{
    return inputs[_inputIndex];
}

//*************************************************************************************
// saving and inspecting
//*************************************************************************************

void Net::saveWeights()
{
    for (int i = 0; i < nLayers; i++)
    {
        layers[i]->saveWeights();
    }
}

void Net::snapFistLayerWeights()
{
    layers[0]->snapWeights();
}

void Net::snapWeights()
{
    for (int i = 0; i < nLayers; i++)
    {
        layers[i]->snapWeights();
    }
}

void Net::printNetwork()
{
    cout << "This network has " << nLayers << " layers" << endl;
    for (int i = 0; i < nLayers; i++)
    {
        cout << "Layer number " << i << ":" << endl;
        layers[i]->printLayer();
    }
    cout << "The output(s) of the network is(are):";
    for (int i = 0; i < nOutputs; i++)
    {
        cout << " " << this->getOutput(i);
    }
    cout << endl;
}
