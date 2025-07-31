__kernel void
forward_prop_kernel(__global const float *inputs, __global const float *weight,
                    __global const float *biases, __global float *sum_outputs,
                    __global float *activated_outputs, int n_inputs) {
  int neuron_idx = get_global_id(0);

  float sum = 0.0;

  for (int i = 0; i < n_inputs; i++) {
    sum += inputs[i] * weight[neuron_idx * n_inputs + i];
  }

  sum += biases[neuron_idx];

  sum_outputs[neuron_idx] = sum;

  activated_outputs[neuron_idx] = tanh(sum);
}

__kernel void backprop_error_kernel(__global const float *errors_L,
                                    __global const float *weights_L,
                                    __global const float *sum_outputs_L_minus_1,
                                    __global float *errors_L_minus_1,
                                    int n_neurons_L, int n_neurons_L_minus_1) {
  int neuron_idx = get_global_id(0);

  float weighted_error_sum = 0.0;

  for (int i = 0; i < n_neurons_L; i++) {
    weighted_error_sum +=
        errors_L[i] * weights_L[i * n_neurons_L_minus_1 + neuron_idx];
  }

  float activation_val = tanh(sum_outputs_L_minus_1[neuron_idx]);
  float activation_prime = 1.0 - activation_val * activation_val;

  errors_L_minus_1[neuron_idx] = weighted_error_sum * activation_prime;
}

__kernel void update_weights_kernel(__global const float *errors,
                                    __global const float *inputs,
                                    __global float *weights,
                                    float learning_rate, int n_inputs,
                                    int n_neurons) {
  int neuron_idx = get_global_id(0);
  int input_idx = get_global_id(1);

  if (neuron_idx >= n_neurons || input_idx >= n_inputs) {
    return;
  }

  float delta = learning_rate * errors[neuron_idx] * inputs[input_idx];
  weights[neuron_idx * n_inputs + input_idx] += delta;
}

__kernel void calculate_output_error_kernel(float network_error,
                                            __global const float *sum_outputs,
                                            __global float *errors_out,
                                            float error_gain) {
  int neuron_idx = get_global_id(0);
  float activation_val = tanh(sum_outputs[neuron_idx]);
  float activation_prime = 1.0f - activation_val * activation_val;
  errors_out[neuron_idx] = network_error * activation_prime * error_gain;
}