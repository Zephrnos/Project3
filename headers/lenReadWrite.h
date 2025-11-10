#ifndef lenReadWrite_h
#define lenReadWrite_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void lenRead(std::ofstream& output, const std::string& record);
void processFile(std::string& inputFileName, const std::string& outputFileName);



#endif // lenReadWrite_h
