#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void forward_prop_kernel(__global const double *inputs,
                                  __global const double *weight,
                                  __global const double *biases,
                                  __global double *sum_outputs,
                                  __global double *activated_outputs,
                                  int n_inputs) {
  int neuron_idx = get_global_id(0);

  double sum = 0.0;

  for (int i = 0; i < n_inputs; i++) {
    sum += inputs[i] * weight[neuron_idx * n_inputs + i];
  }

  sum += biases[neuron_idx];

  sum_outputs[neuron_idx] = sum;

  activated_outputs[neuron_idx] = tanh(sum);
}

__kernel void
backprop_error_kernel(__global const double *errors_L,
                      __global const double *weights_L,
                      __global const double *sum_outputs_L_minus_1,
                      __global double *errors_L_minus_1, int n_neurons_L,
                      int n_neurons_L_minus_1) {
  int neuron_idx = get_global_id(0);

  double weighted_error_sum = 0.0;

  for (int i = 0; i < n_neurons_L; i++) {
    weighted_error_sum +=
        errors_L[i] * weights_L[i * n_neurons_L_minus_1 + neuron_idx];
  }

  double activation_val = tanh(sum_outputs_L_minus_1[neuron_idx]);
  double activation_prime = 1.0 - activation_val * activation_val;

  errors_L_minus_1[neuron_idx] = weighted_error_sum * activation_prime;
}

__kernel void update_weights_kernel(__global const double *errors,
                                    __global const double *inputs,
                                    __global double *weights,
                                    __global double *biases,
                                    double learning_rate, int n_inputs,
                                    int n_neurons) {
  int neuron_idx = get_global_id(0);
  int input_idx = get_global_id(1);

  if (neuron_idx >= n_neurons || input_idx >= n_inputs) {
    return;
  }

  double delta = learning_rate * errors[neuron_idx] * inputs[input_idx];
  weights[neuron_idx * n_inputs + input_idx] += delta;

  if (input_idx == 0) {
    biases[neuron_idx] += learning_rate * errors[neuron_idx];
  }
}

__kernel void
calculte_output_error_kernel(__global const double *activated_outputs,
                             __global const double *target_outputs,
                             __global const double *sum_outputs,
                             __global double *errors_out) {
  int neuron_idx = get_global_id(0);

  // For f(x) = tanh(x), the derivative is 1 - tanh(x)^2.
  double activation_val = activated_outputs[neuron_idx];
  double activation_prime = 1.0 - activation_val * activation_val;

  errors_out[neuron_idx] =
      (activation_val - target_outputs[neuron_idx]) * activation_prime;
}