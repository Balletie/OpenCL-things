#include <OpenCL/cl.h>
#include <stdio.h>

int main() {
	cl_int err = 0;
	cl_platform_id plat;
	cl_device_id dev;

	err = clGetPlatformIDs(1, &plat, NULL);
	if (err != CL_SUCCESS) printf("ERROR");
	err = clGetDeviceIDs(plat, CL_DEVICE_TYPE_ALL, 1, &dev, NULL);
	if (err != CL_SUCCESS) printf("ERROR");
	
	char info[1024];
	clGetDeviceInfo(dev, CL_DEVICE_NAME, 1024, info, NULL);
	printf("Using OpenCL device %s\n", info);
}
