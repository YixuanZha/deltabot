#include "../include/cldl/Net.h"
#include <algorithm>

using namespace std;
int main(){
//    unitTest* unitest;
//    unitest = new unitTest();
//    unitest->test_net_setInputs();
//    unitest->test_net_porpInputs();
//    unitest->test_net_masterPropagation();
const int NLAYERS = 5;
Net* net;
int nPROPAGATIONS = 2;
const int NetnInputs = 1;
double inputs[NetnInputs] = {1};
int N1 = 1;
int nNeurons[NLAYERS]={5,4,3,2,1};
int* nNeuronsp=nNeurons;
net = new Net(NLAYERS, nNeuronsp, NetnInputs, nPROPAGATIONS);
net->initNetwork(Neuron::W_ONES, Neuron::B_NONE, Neuron::Act_Sigmoid);
double LEARNINGRATE = 1;
net->setLearningRate(LEARNINGRATE);
std::vector<int> injectionLayers_BackProp;
injectionLayers_BackProp.reserve(1);
injectionLayers_BackProp = {NLAYERS-1};
std::vector<int> injectionLayers_ForwardProp;
injectionLayers_ForwardProp.reserve(1);
injectionLayers_ForwardProp = {0};
double error = 1;
double* pred_pointer = &inputs[0];
net->setInputs(pred_pointer);
net->propInputs();
net->masterPropagate(injectionLayers_ForwardProp, 0,
                     Net::FORWARD, error,
                     Neuron::Value, 1);
net->masterPropagate(injectionLayers_BackProp, 1,
                     Net::BACKWARD, error,
                     Neuron::Value, 0);
net->updateWeights();
cout << "*********** updateWeights ***********" << endl;
return 0;
}
