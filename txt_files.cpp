#include <iostream>
#include <fstream>
#include <string>
#include "txt_files.h"


void printFile(std::string path) {
    std::string line;
    int count = 0;

    std::ifstream file(path);

    while (std::getline(file, line)) {
        std::cout << line << std::endl;
        count++;
    }

    std::cout << "Count: " << count << std::endl;

    file.close();
}
