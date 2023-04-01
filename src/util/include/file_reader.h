/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_FILE_READER_H
#define SALMIAC_FILE_READER_H

#include <string>

namespace sal {

class File_reader {
public:
    static std::string read_file(std::string const& file) noexcept;
};


} // namespace sal

#endif //SALMIAC_FILE_READER_H
