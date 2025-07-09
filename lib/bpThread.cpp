//
// Created by sama on 16/10/2021.
//

#include "../include/cldl/bpThread.h"

#include <cstdio>
#include <chrono>
#include <thread>
#include <iostream>
#include <cassert>
using namespace std;

void bpThread::run() {
        double thisSum = 0.00;
        cout << "working on neuron: " << threadIndex << endl;
    for (int n_index = 0; n_index < layers[layerIndex]->getnNeurons(); n_index++){ //iterate through the neurons of each layer
        if( inject){
            tempError = controlError;
        }else{
            tempError = layers[layerIndex]->getInternalErrors(internalErrorIndex, n_index);
        }
        tempWeight = layers[layerIndex]->getWeights(n_index, threadIndex);
        thisSum += (tempError * tempWeight);
    }
    assert(std::isfinite(thisSum) && "Thread failed");
    layers[layerIndex-1]->setInternalErrors(internalErrorIndex, thisSum,
                                                        threadIndex, errorMethod);
}
