#include "../include/cldl/Neuron.h"
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

//*************************************************************************************
// constructor de-constructor
//*************************************************************************************

Neuron::Neuron(int _nInputs, int _numBuses)
{
    nInputs=_nInputs;
    weights = new double[nInputs];
    initialWeights = new double[nInputs];
    inputs = new double[nInputs];
    inputErrors = new double[nInputs];
    numBuses = _numBuses;
    rawInternalErrors = new double[numBuses];
    busIsSet = new bool[numBuses];
    busMethod = new int(numBuses);
    internalErrors = new double(numBuses);
    for (int i = 0 ; i <numBuses; i++){
        busIsSet[i] = false;
        busMethod[i] = 0;
    }
}

Neuron::~Neuron(){
    delete [] weights;
    delete [] initialWeights;
    delete [] inputs;
    delete [] inputErrors;
    delete [] rawInternalErrors;
    delete [] busIsSet;
}

//*************************************************************************************
//initialisation:
//*************************************************************************************

void Neuron::initNeuron(int _neuronIndex, int _layerIndex, weightInitMethod _wim, biasInitMethod _bim, actMethod _am){
    myLayerIndex = _layerIndex;
    myNeuronIndex = _neuronIndex;
    for (int i=0; i<nInputs; i++) {
        switch (_wim) {
            case W_ZEROS:
                weights[i] = 0;
                break;
            case W_ONES:
                weights[i] = 1;
                break;
            case W_RANDOM:
                weights[i] = (((double) rand() / (RAND_MAX))); //* 2) -1;
                break;
                //cout << " Neuron: weight is: " << weights[i] << endl;
                /* rand function generates a random function between
                 * 0 and RAND_MAX, after the division the weights are
                 * set to a value between 0 and 1 */
        }
        initialWeights[i] = weights[i];
    }
    weightSum = 0;
    for (int i=0; i<nInputs; i++){
        weightSum += fabs(weights[i]);
        maxWeight = max(maxWeight, weights[i]);
        minWeight = min (minWeight, weights[i]);
    }

    switch (_bim){
        case B_NONE:
            bias=0;
            break;
        case B_RANDOM:
            bias=((double)rand()/RAND_MAX);
            break;
    }
    switch(_am){
        case Act_Sigmoid:
            actMet = 0;
            break;
        case Act_Tanh:
            actMet = 1;
            break;
        case Act_NONE:
            actMet = 2;
            break;
    }
}

void Neuron::setLearningRate(double _learningRate){
    learningRate=_learningRate;
}

void Neuron::setInput(int _index,  double _value) {
    /* the seInput function sets one input value at the given index,
     * it has to be implemented in a loop inside the layer class to set
     * all the inputs associated with all the neurons in that layer*/
    assert((_index>=0)&&(_index<nInputs) && "Neuron failed");
    /*checking _index is a valid int, non-negative and within boundary*/
    inputs[_index] = _value;
    //cout << "Neuron the input is: " << _value << endl;
}

void Neuron::propInputs(int _index,  double _value){
    /*works like setInput function expect it only applies
     * to the neurons in the hidden and output layers
     * and not the input layer*/
    assert((_index>=0)&&(_index<nInputs) && "Neuron failed");
    inputs[_index] = _value;
}

int Neuron::calcOutput(int _layerHasReported){
    sum=0;
    for (int i=0; i<nInputs; i++){
        sum += inputs[i] * weights[i];
    }
    sum += bias;
    if (myLayerIndex == 0){
        sum = sum / weightBoost;
    }
    assert(std::isfinite(sum) && "Neuron failed");
    output = doActivation(sum);
    iHaveReported = _layerHasReported;
    if (output > 0.49 && iHaveReported == 0){
        //cout << "I'm saturating, " << output << " layer: " << myLayerIndex << " neuron: " << myNeuronIndex << endl;
        iHaveReported = 1;
    }
    assert(std::isfinite(output) && "Neuron failed");
    return iHaveReported;
}

void Neuron::setErrorInputsAndCalculateInternalError(int _inputIndex,
                                                     double _value, int _internalErrorIndex,
                                                     errorMethod _errorMethod){
    assert((_inputIndex>=0)&&(_inputIndex<nInputs) && "Inadequate number of inputs provided");
    inputErrors[_inputIndex] = _value;
    countInputErrors += 1;
    if (countInputErrors == nInputs){
        double errorSum =0;
        for (int i=0; i<nInputs; i++) {
            errorSum += inputErrors[i] * weights[i];
        }
        assert(std::isfinite(errorSum) && "Neuron failed");
        rawInternalErrors[_internalErrorIndex] = errorSum * doActivationPrime(sum);
        //cout << _internalErrorIndex << " setting this to true second time" <<endl;
        busIsSet[_internalErrorIndex] = true;
        busMethod[_internalErrorIndex] = _errorMethod;
        switch(_errorMethod){
            case(Value):
                internalErrors[_internalErrorIndex] =
                        rawInternalErrors[_internalErrorIndex];
                assert(isfinite(internalErrors[_internalErrorIndex]));
                break;
            case(Absolute):
                internalErrors[_internalErrorIndex] =
                        fabs(rawInternalErrors[_internalErrorIndex]);
                assert(isfinite(internalErrors[_internalErrorIndex]));
                break;
            case(Sign):
                if(rawInternalErrors[_internalErrorIndex] == 0){
                    internalErrors[_internalErrorIndex] = 0;
                    assert(isfinite(internalErrors[_internalErrorIndex]));
                }else{
                    if(rawInternalErrors[_internalErrorIndex] > 0){
                        internalErrors[_internalErrorIndex] = 1;
                        assert(isfinite(internalErrors[_internalErrorIndex]));

                    }else{
                        internalErrors[_internalErrorIndex] = -1;
                        assert(isfinite(internalErrors[_internalErrorIndex]));
                    }
                }
                break;
        }
        countInputErrors = 0; // set the counter to zero again
    }
}

void Neuron::setInternalError(int _internalErrorIndex, double _sumValue,
                              errorMethod _errorMethod){
//    cout << "index: " << _internalErrorIndex << " value: " << _sumValue << endl;
    assert((std::isfinite(_sumValue)) && (_internalErrorIndex<numBuses)
        && (_internalErrorIndex>=0) && "Neuron: set internal error failed");
    rawInternalErrors[_internalErrorIndex] = _sumValue * doActivationPrime(sum);
    assert(std::isfinite(rawInternalErrors[_internalErrorIndex]) && "Neuron failed");
    //cout << _internalErrorIndex << " setting this to true" <<endl;
    busIsSet[_internalErrorIndex] = true;
    busMethod[_internalErrorIndex] = _errorMethod;
//    cout << "being called? " << endl;
    switch(_errorMethod){
        case(Value):
            internalErrors[_internalErrorIndex] =
                    rawInternalErrors[_internalErrorIndex];
            assert(isfinite(internalErrors[_internalErrorIndex]));
            break;
        case(Absolute):
            internalErrors[_internalErrorIndex] =
                    fabs(rawInternalErrors[_internalErrorIndex]);
            assert(isfinite(internalErrors[_internalErrorIndex]));
            break;
        case(Sign):
            if(rawInternalErrors[_internalErrorIndex] == 0){
                internalErrors[_internalErrorIndex] = 0;
                assert(isfinite(internalErrors[_internalErrorIndex]));
            }else{
                if(rawInternalErrors[_internalErrorIndex] > 0){
                    internalErrors[_internalErrorIndex] = 1;
                    assert(isfinite(internalErrors[_internalErrorIndex]));

                }else{
                    internalErrors[_internalErrorIndex] = -1;
                    assert(isfinite(internalErrors[_internalErrorIndex]));
                }
                }
            break;
        }
}

double Neuron::getRawInternalErrors(int _internalErrorIndex){
    assert((_internalErrorIndex>=0) && (_internalErrorIndex<numBuses) && "Neuron failed");
    assert(busIsSet[_internalErrorIndex] == true && "Neuron failed");
    return rawInternalErrors[_internalErrorIndex];
}

void Neuron::updateWeights(){
    for(int i=0; i<numBuses; i++){
        //cout<< i << " value: " << busMethod[i] << " : got here 16th Oct............" <<endl;
        //cout<< myNeuronIndex << "    " << myLayerIndex << endl;
        assert(busMethod[i] != 0 && "Neuron failed");
        assert( busIsSet[i] == true && "Neuron: Inadequate number of propagation lines.");
    }
    weightSum = 0;
    maxWeight = 0;
    minWeight = 0;
    double force = 1;
    if (myLayerIndex == 0){
        force  = weightBoost; //forces a bigger change on the first layer for visualisation in greyscale
    }

    resultantInternalError = 1;
    for (int j = 0 ; j<numBuses; j++){
        resultantInternalError *= internalErrors[j];
    }
//    cout << "ie: " << resultantInternalError << endl;

    for (int i=0; i<nInputs; i++){
        assert(isfinite(inputs[i]) && isfinite(resultantInternalError));
        weights[i] += learningRate * inputs[i] * resultantInternalError * force;
        assert(isfinite(weights[i]));
        weightSum += fabs(weights[i]);
        maxWeight = max (maxWeight,weights[i]);
        minWeight = min (maxWeight,weights[i]);
    }
}

double Neuron::doActivation(double _sum){
    double thisoutput = 0;
    switch(actMet){
        case 0:
            thisoutput = (1/(1+(exp(-_sum)))) - 0.5;
            break;
        case 1:
            thisoutput = tanh(_sum);
            break;
        case 2:
            thisoutput = _sum;
            break;
    }
    return (thisoutput);
}

double Neuron::doActivationPrime(double _input){
    double result = 0;
    switch(actMet){
        case 0:
            result = 1 * (0.5 + doActivation(_input)) * (0.5 - doActivation(_input)); //exp(-_input) / pow((exp(-_input) + 1),2);
            break;
        case 1:
            result = 1 - pow (tanh(_input), 2);
            break;
        case 2:
            result = 1;
            break;
    }
    return (result);
}

//*************************************************************************************
// getters:
//*************************************************************************************

double Neuron::getOutput(){
    return (output);
}

double Neuron::getSumOutput(){
    return (sum);
}

double Neuron::getMaxWeight(){
  return maxWeight;
}

double Neuron::getMinWeight(){
  return minWeight;
}

double Neuron::getSumWeight(){
  return weightSum;
}

double Neuron::getWeightChange(){
    weightsDifference = 0;
    weightChange = 0;
    for (int i=0; i<nInputs; i++){
        weightsDifference = weights[i] - initialWeights[i];
        weightChange += pow(weightsDifference,2);
    }
//    cout << "wc: " << weightChange << endl;
    return (weightChange);
}

double Neuron::getWeightDistance(){
    getWeightChange();
    return sqrt(weightChange);
}

int Neuron::getnInputs(){
    return (nInputs);
}

double Neuron::getWeights(int _inputIndex){
    assert((_inputIndex>=0)&&(_inputIndex<nInputs) && "Neuron failed");
    return (weights[_inputIndex]);
}

double Neuron::getInputs(int _inputIndex){
    assert((_inputIndex>=0)&&(_inputIndex<nInputs) && "Neuron failed");
    return (inputs[_inputIndex]);
}

double Neuron::getInitWeights(int _inputIndex){
    assert((_inputIndex>=0)&&(_inputIndex<nInputs) && "Neuron failed");
    return (initialWeights[_inputIndex]);
}

//*************************************************************************************
//saving and inspecting
//*************************************************************************************

void Neuron::saveWeights(){
  char l = '0';
  char n = '0';
  l += myLayerIndex + 1;
  n += myNeuronIndex + 1;
  string name = "w";
  name += 'L';
  name += l;
  name += 'N';
  name += n;
  name += ".csv";
  std::ofstream Icofile;
  Icofile.open(name, fstream::app);
  for (int i=0; i<nInputs; i++){
    Icofile << weights[i] << " " ;
  }
  Icofile << "\n";
  Icofile.close();
}

void Neuron::printNeuron(){
    cout<< "\t \t This neuron has " << nInputs << " inputs:";
    for (int i=0; i<nInputs; i++){
        cout<< " " << inputs[i];
    }
    cout<<endl;
    cout<< "\t \t The weights for those inputs are:";
    for (int i=0; i<nInputs; i++){
        cout<< " " << weights[i];
    }
    cout<<endl;
    cout<< "\t \t The bias of the neuron is: " << bias << endl;
    cout<< "\t \t The sum and output of this neuron are: " << sum << ", " << output << endl;
}
