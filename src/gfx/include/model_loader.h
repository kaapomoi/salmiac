/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_MODEL_LOADER_H
#define SALMIAC_MODEL_LOADER_H

#include "model.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace sal {

/*
 *  TODO: Perhaps this should not be a static class in the end.
 */
class Model_loader {
public:
    static Model from_file(std::string const& full_path, std::uint64_t const import_flags) noexcept;

private:
    static void process_node(aiNode* node,
                             aiScene const* scene,
                             Model& model,
                             std::string const& directory) noexcept;

    static Mesh process_mesh(aiMesh* mesh,
                             aiScene const* scene,
                             std::string const& directory) noexcept;

    static std::vector<Texture> load_material_textures(aiMaterial* mat,
                                                       aiTextureType const type,
                                                       std::string const& directory) noexcept;

    static Texture load_texture(std::string const& full_path, Texture::Type const type) noexcept;

    static Texture::Type ai_texture_type_to_sal_texture_type(aiTextureType const type) noexcept;
};

} // namespace sal

#endif //SALMIAC_MODEL_LOADER_H
