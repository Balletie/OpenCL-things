#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif
#include "file.h"
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printCLDeviceInfo(cl_device_id device, bool print_extensions) {
	char name[256];
	clGetDeviceInfo(device, CL_DEVICE_NAME, 256, name, NULL);
	printf(" * %s\n", name);
	if (!print_extensions) return;

	char info[1024];
	printf("    Extensions:\n     ");
	clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 1024, info, NULL);
	for (int j = 0; j < 1024; j++) {
		if (info[j] == 0x00) break;
		if (info[j] == 0x20) {
			putchar('\n');
			if (info[j+1] == 0x00) continue;
			for (int k = 0; k < 5; k++) putchar(' ');
			continue;
		}
		putchar(info[j]);
	}
}

int main() {
	std::ios::sync_with_stdio();
	cl_int err = 0;
	cl_uint num_devices;
	cl_uint num_plats;
	cl_platform_id *plat;
	cl_device_id final_device;

	err = clGetPlatformIDs(0, NULL, &num_plats);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
	if (num_plats > 1)		printf("There are %u platforms\n", num_plats);
	else if (num_plats == 1)	printf("There's exactly one platform, namely:\n");
	else				printf("There are no available platforms"), exit(0);

	plat = (cl_platform_id*) malloc(sizeof(cl_platform_id) * num_plats);
	err = clGetPlatformIDs(num_plats, plat, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	for (int i = 0; i < num_plats; i++) {
		char plat_info[1024];

		err = clGetPlatformInfo(plat[i], CL_PLATFORM_VERSION, 1024, plat_info, NULL);
		if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
		printf("Platform %d:\n", i);
		printf(" * %s\n", plat_info);

		err = clGetDeviceIDs(plat[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
		printf("There are %u devices found, namely:\n", num_devices);

		cl_device_id devices[num_devices];
		err = clGetDeviceIDs(plat[i], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
		if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
		
		for (int j = 0; j < num_devices; j++) {
			cl_device_type current_type;
			clGetDeviceInfo(devices[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &current_type, NULL);
			if (current_type == CL_DEVICE_TYPE_GPU) {
				printf("Found GPU\n");
				final_device = devices[j];
			}

			printCLDeviceInfo(devices[j],false);
			if (j != num_devices - 1) putchar('\n');
		}
	}

	unsigned int count = 1024;
	size_t local;
	size_t global = count;
	float input_data[count];
	// Fill the array
	srand(time(NULL));
	for (int i = 0; i < count; i++) input_data[i] = rand() / (float)RAND_MAX;	
	float output_data[count];
	float results[count];
	const char* program_source = readFile("kernel/square.cl");
	printf("%s\n", program_source);

	// Set OpenCL context
	printf("Check1\n");
	cl_context context = clCreateContext(0, 1, &final_device, NULL, NULL, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u: Failed to make OpenCL context\n", __LINE__);

	// Create command queue
	printf("Check2\n");
	cl_command_queue commands = clCreateCommandQueue(context, final_device, 0, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Create the program from source and build it.
	printf("Check3\n");
	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &program_source, NULL, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
	printf("Check4\n");
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	printf("Check5\n");
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
	cl_kernel kernel = clCreateKernel(program, "square", &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Create the buffers for input and output arrays.
	cl_mem read_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*count, NULL, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
	cl_mem write_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*count, NULL, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Copy the input data (blocking)
	err = clEnqueueWriteBuffer(commands, read_buffer, CL_TRUE, 0, sizeof(float)*count, input_data, 0, NULL, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Set the kernel arguments
	err = 0;
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &read_buffer);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &write_buffer);
	err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);	
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Get the maximum work group size
	err = clGetKernelWorkGroupInfo(kernel, final_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Execute the kernel
	err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Give it time to finish
	clFinish(commands);

	err = clEnqueueReadBuffer(commands, write_buffer, CL_TRUE, 0, sizeof(float)*count, output_data, 0, NULL, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Validate everything
	int correct = 0;
	for (int i = 0; i < count; i++) {
		if (output_data[i] == input_data[i]*input_data[i]) correct++;
	}
	printf("%d/%d correct results\n", correct, count);
}
