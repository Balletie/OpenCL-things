__kernel void square(__global float* input, __global float* output, const unsigned int count) {
	int i = get_global_id(0);
	int start = i * 1024;
	if (start + 1024 <= count) {
		for (int j = 0; j < 1024; j++) {
			float num = input[start + j];
			output[start + j] = num*num;
		}
	}
}
