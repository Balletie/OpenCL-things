#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif
#include "file.h"
#include <ios>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>

void printCLDeviceInfo(cl_device_id device, bool print_extensions) {
	char name[256];
	clGetDeviceInfo(device, CL_DEVICE_NAME, 256, name, NULL);
	printf(" * %s\n", name);
	size_t work_item_sizes[10];
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*10, &work_item_sizes, NULL);
	for (int i = 0; i < 3; i++) {
		printf("  Max work item size (dimension %d): %u\n",i, (unsigned int) work_item_sizes[i]);
	}
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
	printf("\n");

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
	printf("Loaded kernel program source:\n%s\n", program_source);

	// Set OpenCL context
	cl_context context = clCreateContext(0, 1, &final_device, NULL, NULL, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u: Failed to make OpenCL context\n", __LINE__);

	// Create command queue
	cl_command_queue commands = clCreateCommandQueue(context, final_device, 0, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Create the program from source and build it.
	cl_program program = clCreateProgramWithSource(context, 1, (const char **) &program_source, NULL, &err);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[2048];

		printf("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(program, final_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
	}
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

	std::clock_t c_start = std::clock();
	// Execute the kernel
	err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);

	// Give it time to finish
	clFinish(commands);

	err = clEnqueueReadBuffer(commands, write_buffer, CL_TRUE, 0, sizeof(float)*count, output_data, 0, NULL, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u\n", __LINE__);
	std::clock_t c_end = std::clock();
	printf("Test duration (OpenCL): %f ms\n", 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC);

	float test_result[count];
	c_start = std::clock();
	for (int i = 0; i < count; i++) {
		float num = input_data[i];
		test_result[i] = num*num;
	}
	c_end = std::clock();
	printf("Test duration (regular): %f ms\n", 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC);
	printf("Validating..:\n");
	// Validate everything
	int correct_opencl = 0;
	int correct_regular = 0;
	for (int i = 0; i < count; i++) {
		if (output_data[i] == input_data[i]*input_data[i]) correct_opencl++;
		if (test_result[i] == input_data[i]*input_data[i]) correct_regular++;
	}
	printf("%d/%d correct results (OpenCL)\n", correct_opencl, count);
	printf("%d/%d correct results (regular)\n", correct_regular, count);
}
