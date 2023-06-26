#include <iostream>
#include <fstream>
#include <string>
#include "txt_files.h"


void printFile(std::string path) {
    std::string line;

    std::ifstream file(path);

    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    file.close();
}
