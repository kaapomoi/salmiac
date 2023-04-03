/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "model_loader.h"

#include "log.h"
#include "mesh_binder.h"
#include "texture_loader.h"

namespace sal {


Model Model_loader::from_file(std::string const& full_path,
                              std::uint64_t const import_flags) noexcept
{
    Model model{};

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFile(full_path, import_flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Log::warn("Error in model import: {}", importer.GetErrorString());
        return model;
    }

    // Add one to include the last '/' in the directory string as well
    auto const directory = full_path.substr(0, full_path.find_last_of('/') + 1);

    process_node(scene->mRootNode, scene, model, directory);

    for (auto& mesh : model.meshes) {
        Mesh_binder::setup(mesh);
    }

    return model;
}

void Model_loader::process_node(aiNode* node,
                                aiScene const* scene,
                                Model& model,
                                std::string const& directory) noexcept
{
    // Process all the node's meshes (if any),
    for (std::uint32_t i{0}; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.push_back(process_mesh(mesh, scene, directory));
    }

    // then do the same for each of its children
    for (std::uint32_t i{0}; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, model, directory);
    }
}

Mesh Model_loader::process_mesh(aiMesh* mesh,
                                aiScene const* scene,
                                std::string const& directory) noexcept
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        // Process vertex positions, normals and texture coordinates
        vertex.position =
            glm::vec3{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};

        if (mesh->HasNormals()) {
            vertex.normal =
                glm::vec3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }

        // Does the mesh contain texture coordinates?
        if (mesh->mTextureCoords[0]) {
            vertex.uv = glm::vec2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        }
        else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Process material
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps =
            load_material_textures(material, aiTextureType_DIFFUSE, directory);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps =
            load_material_textures(material, aiTextureType_SPECULAR, directory);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        if (textures.empty()) {
            Texture texture{Model_loader::load_texture("default.png", Texture::Type::diffuse)};
            textures.push_back(texture);
        }
    }

    return {vertices, indices, textures};
}

std::vector<Texture> Model_loader::load_material_textures(aiMaterial* mat,
                                                          aiTextureType const type,
                                                          std::string const& directory) noexcept
{
    std::vector<Texture> textures;
    auto const texture_type = ai_texture_type_to_sal_texture_type(type);

    Log::info("Loading textures of type {} from directory {}", Texture::str(texture_type),
              directory);

    for (std::uint32_t i{0}; i < mat->GetTextureCount(type); i++) {
        aiString path;
        mat->GetTexture(type, i, &path);

        textures.emplace_back(Model_loader::load_texture(directory + path.C_Str(), texture_type));
    }

    return textures;
}

Texture Model_loader::load_texture(std::string const& full_path, Texture::Type const type) noexcept
{
    // TODO: Add caching
    return Texture_loader::from_file(full_path, type);
}

Texture::Type Model_loader::ai_texture_type_to_sal_texture_type(aiTextureType const type) noexcept
{
    if (type == aiTextureType_DIFFUSE) {
        return Texture::Type::diffuse;
    }
    else if (type == aiTextureType_SPECULAR) {
        return Texture::Type::specular;
    }
    else {
        return Texture::Type::unknown_texture_type;
    }
}

} // namespace sal