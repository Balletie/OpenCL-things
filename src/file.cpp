#include "file.h"
#include <iostream>
#include <fstream>

char* readFile(const char* filename) {
	std::ifstream file(filename, std::ifstream::binary);
	file.seekg(0, file.end);
	int length = file.tellg(); 
	file.seekg(0, file.beg);

	char* contents = new char[length+1];
	file.read(contents, length);
	contents[length] = '\0';
	return contents;
}
