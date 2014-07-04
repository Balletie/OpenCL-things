#define __CL_ENABLE_EXCEPTIONS
#include "OpenCL/cl.hpp"
#include <stdio.h>
#include <iostream>

int main() {
	try {
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() > 1) 	printf("There are %u platforms:\n", platforms.size());
		else if (platforms.size() == 1) printf("There's exactly one platform:\n");
		else 				printf("There are no available platforms :("), exit(0);
		for (int i = 0; i < platforms.size(); i++) {
			std::string plat_info;
			std::vector<cl::Device> devices;
			platforms[i].getInfo(CL_PLATFORM_VERSION, &plat_info);
			printf("Platform %d:\n", i);
			printf(" * %s\n", plat_info.c_str());
			platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
			printf("There are %u devices found:\n", devices.size());
			for (int j = 0; j < devices.size(); j++) {
				std::string device_name;
				std::string device_info;
				devices[i].getInfo(CL_DEVICE_NAME, &device_name);
				devices[i].getInfo(CL_DEVICE_EXTENSIONS, &device_info);
				printf(" * %s\n    Extensions:\n     ", device_name.c_str());
				const char* device_info_c_str = device_info.c_str();
				for (int k = 0; k < device_info.size(); k++) {
					if (device_info_c_str[k] == 0x00) break;
					if (device_info_c_str[k] == 0x20) {
						putchar('\n');
						if (device_info_c_str[k+1] == 0x00) continue;
						for (int l = 0; l < 5; l++) putchar(' ');
						continue;
					}
					putchar(device_info_c_str[k]);
				}
				putchar('\n');
			}
		}
	} catch (cl::Error err) {
		std::cerr
		<< "ERROR: "
		<< err.what()
		<< "("
		<< err.err()
		<< ")"
		<< std::endl;
	}
}
