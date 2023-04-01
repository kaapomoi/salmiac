/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */


#include "file_reader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace sal {

std::string File_reader::read_file(std::string const& file) noexcept
{
    std::ifstream infile{file, std::ios::in};
    std::string result;

    if (infile && infile.is_open()) {
        std::stringstream buffer;
        buffer << infile.rdbuf();
        result.assign(buffer.str());
    }
    else {
        std::cout << "Could not open file\n";
    }

    infile.close();

    return result;
}


} // namespace sal