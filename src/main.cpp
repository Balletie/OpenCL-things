#include <OpenCL/cl.h>
#include <stdio.h>

int main() {
	cl_int err = 0;
	cl_uint num_devices;
	cl_uint num_plats;
	cl_platform_id plat;

	err = clGetPlatformIDs(0, NULL, &num_plats);
	if (err != CL_SUCCESS) printf("ERROR at line %u", __LINE__);
	if (num_plats > 1) printf("There are %u platforms\n", num_plats);
	else if (num_plats == 1) printf("There's exactly one platform, namely:\n");
	else printf("There are no available platforms"), exit(0);

	err = clGetPlatformIDs(1, &plat, NULL);
	if (err != CL_SUCCESS) printf("ERROR at line %u", __LINE__);
	char plat_info[1024];
	err = clGetPlatformInfo(plat, CL_PLATFORM_VERSION, 1024, plat_info, NULL);
	if (err != CL_SUCCESS) printf("ERROR at line %u", __LINE__);
	printf(" * %s\n", plat_info);

	err = clGetDeviceIDs(plat, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	if (err != CL_SUCCESS) printf("ERROR at line %u", __LINE__);
	printf("There are %u devices found, namely:\n", num_devices);

	cl_device_id devices[num_devices];

	err = clGetDeviceIDs(plat, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
	if (err != CL_SUCCESS) printf("ERROR at line %u", __LINE__);
	
	char info[num_devices][1024];
	for (int i = 0; i < num_devices; i++) {
		clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, &info[i], NULL);
		printf(" * %s\n", info[i]);
	}
}
