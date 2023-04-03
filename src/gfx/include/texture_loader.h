/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TEXTURE_LOADER_H
#define SALMIAC_TEXTURE_LOADER_H

#include "texture.h"

namespace sal {


class Texture_loader {
public:
    static Texture from_file(std::string const& full_path, Texture::Type const type) noexcept;
};

} // namespace sal

#endif //SALMIAC_TEXTURE_LOADER_H
