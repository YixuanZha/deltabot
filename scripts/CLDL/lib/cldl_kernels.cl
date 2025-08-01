// CLDL Kernels for Forward Propagation, Backpropagation, and Weight Updates
// This file contains OpenCL kernels for a simple neural network implementation.

/**
 * Forward propagation kernel
 * Computes the output of neurons in a layer given inputs, weights, and biases.
 * Applies the tanh activation function.
 * @param inputs Input values to the layer.
 * @param weight Weights for the connections between inputs and neurons.
 * @param biases Biases for each neuron.
 * @param sum_outputs Output sums before activation.
 * @param activated_outputs Output values after applying the activation
 * function.
 * @param n_inputs Number of inputs to the layer.
 */
__kernel void
forward_prop_kernel(__global const float *inputs, __global const float *weight,
                    __global const float *biases, __global float *sum_outputs,
                    __global float *activated_outputs, int n_inputs) {
  int neuron_idx = get_global_id(0); // Get the index of the neuron being processed

  float sum = 0.0;

  for (int i = 0; i < n_inputs; i++) {
    sum += inputs[i] * weight[neuron_idx * n_inputs + i]; // Compute the weighted sum of inputs
  }

  sum += biases[neuron_idx]; // Add the bias for the neuron

  sum_outputs[neuron_idx] = sum; // Store the sum before activation

  activated_outputs[neuron_idx] = tanh(sum); // Apply the tanh activation function
}

/**
 * Backpropagation error kernel
 * Computes the error for each neuron in a layer during backpropagation.
 * @param errors_L Errors from the next layer.
 * @param weights_L Weights for the connections between this layer and the next.
 * @param sum_outputs_L_minus_1 Output sums from the previous layer.
 * @param errors_L_minus_1 Errors for the previous layer.
 */
__kernel void backprop_error_kernel(__global const float *errors_L,
                                    __global const float *weights_L,
                                    __global const float *sum_outputs_L_minus_1,
                                    __global float *errors_L_minus_1,
                                    int n_neurons_L, int n_neurons_L_minus_1) {
  int neuron_idx = get_global_id(0);

  float weighted_error_sum = 0.0;

  for (int i = 0; i < n_neurons_L; i++) {
    weighted_error_sum +=
        errors_L[i] * weights_L[i * n_neurons_L_minus_1 + neuron_idx]; // Compute the weighted sum of errors from the next layer
  }

  float activation_val = tanh(sum_outputs_L_minus_1[neuron_idx]); // Get the activation value from the previous layer
  float activation_prime = 1.0 - activation_val * activation_val; // Compute the derivative of the activation function

  errors_L_minus_1[neuron_idx] = weighted_error_sum * activation_prime; // Calculate the error for the previous layer neuron
}

/**
 * Update weights kernel
 * Updates the weights of the connections between inputs and neurons based on
 * the errors and learning rate.
 * @param errors Errors for each neuron in the layer.
 * @param inputs Input values to the layer.
 * @param weights Weights to be updated.
 * @param learning_rate Learning rate for weight updates.
 */
__kernel void update_weights_kernel(__global const float *errors,
                                    __global const float *inputs,
                                    __global float *weights,
                                    float learning_rate, int n_inputs,
                                    int n_neurons) {
  int neuron_idx = get_global_id(0);
  int input_idx = get_global_id(1);

  if (neuron_idx >= n_neurons || input_idx >= n_inputs) // Check for valid indices
  {
    return;
  }

  float delta = learning_rate * errors[neuron_idx] * inputs[input_idx]; // Calculate the weight update based on the error and input
  weights[neuron_idx * n_inputs + input_idx] += delta; // Update the weight
}

/**
 * Calculate output error kernel
 * Computes the error for each neuron in the output layer.
 * @param network_error Overall error for the network.
 * @param sum_outputs Output sums from the final layer.
 * @param errors_out Output errors for each neuron.
 * @param error_gain Gain factor for the error.
 */
__kernel void calculate_output_error_kernel(float network_error,
                                            __global const float *sum_outputs,
                                            __global float *errors_out,
                                            float error_gain) {
  int neuron_idx = get_global_id(0);
  float activation_val = tanh(sum_outputs[neuron_idx]); // Get the activation value for the neuron
  float activation_prime = 1.0f - activation_val * activation_val; // Compute the derivative of the activation function
  errors_out[neuron_idx] = network_error * activation_prime * error_gain; // Calculate the output error based on the network error, activation value, and gain factor
}