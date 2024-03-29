/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "texture_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "log.h"
#include "stb_image.h"

#include "GL/glew.h"

namespace sal {


Texture Texture_loader::from_file(std::string const& full_path, Texture::Type const type) noexcept
{
    Texture texture{};

    std::int32_t channels;
    auto const pixel_data =
        stbi_load(full_path.c_str(), &texture.width, &texture.height, &channels, 0);
    if (pixel_data == nullptr) {
        Log::warn("Image {} could not be loaded", full_path);
        stbi_image_free(pixel_data);
        return texture;
    }

    static std::unordered_map<std::int32_t, GLenum> const channels_to_color_format{
        {1, GL_RED}, {3, GL_RGB}, {4, GL_RGBA}};

    GLenum format;
    try {
        format = channels_to_color_format.at(channels);
    }
    catch (...) {
        Log::warn("Image {} could not be loaded. Reason: invalid amount of color channels in "
                  "texture ({})",
                  full_path, channels);
        stbi_image_free(pixel_data);
        return texture;
    }

    glGenTextures(1, &texture.id);

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), texture.width, texture.height, 0,
                 format, GL_UNSIGNED_BYTE, pixel_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixel_data);

    texture.path = full_path;
    texture.type = type;

    return texture;
}

} // namespace sal
