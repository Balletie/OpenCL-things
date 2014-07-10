#define N 1024

__kernel void square(__global float* input, __global float* output, const unsigned int count) {
	int i = get_global_id(0);
	int start = i * N;
	int end = start + N;
	if (end <= count) {
		for (int j = start; j < end; j++) {
			float num = input[j];
			output[j] = num*num;
		}
	}
}
