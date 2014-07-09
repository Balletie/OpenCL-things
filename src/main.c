#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#include <stdio.h>

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
	cl_int err = 0;
	cl_uint num_devices;
	cl_uint num_plats;
	cl_platform_id *plat;

	err = clGetPlatformIDs(0, NULL, &num_plats);
	if (err != CL_SUCCESS)		printf("ERROR at line %u", __LINE__);
	if (num_plats > 1)		printf("There are %u platforms\n", num_plats);
	else if (num_plats == 1)	printf("There's exactly one platform, namely:\n");
	else				printf("There are no available platforms"), exit(0);

	plat = (cl_platform_id*) malloc(sizeof(cl_platform_id) * num_plats);
	err = clGetPlatformIDs(num_plats, plat, NULL);
	if (err != CL_SUCCESS)		printf("ERROR at line %u", __LINE__);

	for (int i = 0; i < num_plats; i++) {
		char plat_info[1024];

		err = clGetPlatformInfo(plat[i], CL_PLATFORM_VERSION, 1024, plat_info, NULL);
		if (err != CL_SUCCESS)		printf("ERROR at line %u", __LINE__);
		printf("Platform %d:\n", i);
		printf(" * %s\n", plat_info);

		err = clGetDeviceIDs(plat[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		if (err != CL_SUCCESS)		printf("ERROR at line %u", __LINE__);
		printf("There are %u devices found, namely:\n", num_devices);

		cl_device_id devices[num_devices];
		err = clGetDeviceIDs(plat[i], CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
		if (err != CL_SUCCESS)		printf("ERROR at line %u", __LINE__);
		
		for (int j = 0; j < num_devices; j++) {
			printCLDeviceInfo(devices[j],true);
			if (j != num_devices - 1) putchar('\n');
		}
	}
}
