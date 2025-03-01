#include "glex/model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

std::unique_ptr<Model> Model::load(const std::string &filepath) {
    auto model = std::unique_ptr<Model>{new Model{}};
    if (!model->load_by_assimp(filepath)) {
        SPDLOG_ERROR("Failed to create model: \"{}\"", filepath);
        return nullptr;
    }
    SPDLOG_INFO("Model has been loaded: \"{}\"", filepath);
    return std::move(model);
}

void Model::draw() const {
    for (const auto &mesh : meshes_) {
        mesh->draw();
    }
}

bool Model::load_by_assimp(const std::string &filepath) {
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SPDLOG_ERROR("Failed to load model from file: \"{}\"", filepath);
        return false;
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
    meshes_.push_back(std::move(gl_mesh));
}
