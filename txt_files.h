#include <string>
#include <vector>
#include <fstream>
void printFile(std::string);
std::vector<std::string> getPaths(std::string);
std::streampos currentPosition(std::ifstream&);
void extract(std::ifstream&, char*, int);
