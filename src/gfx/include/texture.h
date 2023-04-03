/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_TEXTURE_H
#define SALMIAC_TEXTURE_H

#include <cstdint>
#include <string>

namespace sal {


struct Texture {
    enum class Type { unknown_texture_type, diffuse, specular };

    std::uint32_t id{0};
    std::int32_t width{0};
    std::int32_t height{0};

    Type type;
    std::string path;

    static std::string str(Type const texture_type) noexcept
    {
        switch (texture_type) {
        case Type::diffuse:
            return "diffuse";
        case Type::specular:
            return "specular";
        case Type::unknown_texture_type:
            return "unknown_texture_type";
        default:
            return "invalid_texture_type";
        }
    }
};

} // namespace sal

#endif //SALMIAC_TEXTURE_H
