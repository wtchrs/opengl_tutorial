#include "glex/model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

static std::shared_ptr<Texture>
load_texture(const std::string &dirname, const aiMaterial *material, aiTextureType type);

std::unique_ptr<Model> Model::load(const std::string &filepath) {
    auto model = std::unique_ptr<Model>{new Model{}};
    if (!model->load_by_assimp(filepath)) {
        SPDLOG_ERROR("Failed to create model: \"{}\"", filepath);
        return nullptr;
    }
    SPDLOG_INFO("Model has been loaded: \"{}\"", filepath);
    return std::move(model);
}

void Model::draw(const Program *program) const {
    for (const auto &mesh : meshes_) {
        mesh->draw(program);
    }
}


bool Model::load_by_assimp(const std::string &filepath) {
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SPDLOG_ERROR("Failed to load model from file: \"{}\"", filepath);
        return false;
    }
    const auto dirname = filepath.substr(0, filepath.find_last_of('/'));
    for (size_t i = 0; i < scene->mNumMaterials; ++i) {
        const auto material = scene->mMaterials[i];
        auto diffuse = load_texture(dirname, material, aiTextureType_DIFFUSE);
        auto specular = load_texture(dirname, material, aiTextureType_SPECULAR);
        materials_.push_back(std::make_shared<Material>(diffuse, specular));
    }
    process_node(scene->mRootNode, scene);
    return true;
}

void Model::process_node(const aiNode *node, const aiScene *scene) {
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        const auto mesh_index = node->mMeshes[i];
        const auto mesh = scene->mMeshes[mesh_index];
        process_mesh(mesh);
    }
    for (size_t i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i], scene);
    }
}

void Model::process_mesh(const aiMesh *mesh) {
    SPDLOG_DEBUG("Processing mesh: {}, #vert: {}, #face: {}", mesh->mName.C_Str(), mesh->mNumVertices, mesh->mNumFaces);
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        auto &[vx, vy, vz] = mesh->mVertices[i];
        auto &[nx, ny, nz] = mesh->mNormals[i];
        auto &[tx, ty, tz] = mesh->mTextureCoords[0][i];
        vertices.emplace_back(glm::vec3{vx, vy, vz}, glm::vec3{nx, ny, nz}, glm::vec2{tx, ty});
    }
    std::vector<uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }
    auto gl_mesh = Mesh::create(vertices, indices, GL_TRIANGLES);
    if (mesh->mMaterialIndex >= 0) {
        gl_mesh->set_material(materials_[mesh->mMaterialIndex]);
    }
    meshes_.push_back(std::move(gl_mesh));
}

static std::shared_ptr<Texture>
load_texture(const std::string &dirname, const aiMaterial *material, const aiTextureType type) {
    if (material->GetTextureCount(type) <= 0) {
        return nullptr;
    }
    aiString filepath;
    material->GetTexture(type, 0, &filepath);
    const auto image = Image::load(std::format("{}/{}", dirname, filepath.C_Str()));
    if (!image) {
        return nullptr;
    }
    const std::shared_ptr texture = Texture::create();
    texture->set_texture_image(0, *image);
    return texture;
}
