#include <iostream>
#include <fstream>
#include <string>
#include "txt_files.h"
#include <filesystem>
#include <vector>


void printFile(std::string path) {
    std::string line;
    std::ifstream file(path);

    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    file.close();
}

std::vector<std::string> getPaths(std::string directoryPath) {
    std::vector<std::string> paths;

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        std::filesystem::path outfilename = entry.path();
        std::string outfilename_str = outfilename.string();
        paths.push_back(outfilename_str);
    }
    
    return paths;
}

std::streampos currentPosition(std::ifstream& f) {
    return f.tellg();
}

void extract(std::ifstream& f, char* buffer, int pos) {
    f.clear();
    f.seekg(pos, f.beg);
    f.read(buffer, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
}

bool endswithTXT(std::string const &path) {
    if (path.length() < 4) return false;
    return 0 == path.compare(path.length() - 4, 4, ".txt");
}
