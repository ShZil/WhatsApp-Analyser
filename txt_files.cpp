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
