#ifndef convertCSV_h
#define convertCSV_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void lenRead(std::ofstream& output, const std::string& record);
void processFile(std::string& inputFileName, const std::string& outputFileName);
void binaryToCSV();



#endif // convertCSV_h
