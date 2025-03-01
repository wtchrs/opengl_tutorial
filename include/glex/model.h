#ifndef __MODEL_H__
#define __MODEL_H__


#include <assimp/scene.h>
#include <memory>
#include <vector>
#include "glex/common.h"
#include "glex/mesh.h"

/// # Model
///
/// A class that represents a 3D model loaded from a file.
class Model {
    std::vector<std::shared_ptr<Mesh>> meshes_;

public:
    /// ## Model::load
    ///
    /// Loads a model from the specified file path.
    ///
    /// @param filepath: The path to the model file.
    ///
    /// @returns `std::unique_ptr` to a `Model` object if successful, or `nullptr` if loading fails.
    static std::unique_ptr<Model> load(const std::string &filepath);

    /// ## Model::get_mesh_count
    ///
    /// @returns The number of meshes in the model.
    size_t get_mesh_count() const {
        return meshes_.size();
    }

    /// ## Model::get_mesh
    ///
    /// Retrieves a mesh by its index.
    ///
    /// @param index: The index of the mesh to retrieve.
    ///
    /// @returns Shared pointer to the `Mesh` object.
    std::shared_ptr<Mesh> get_mesh(const size_t index) const {
        return meshes_[index];
    }

    /// ## Model::draw
    ///
    /// Draws the model by rendering all its meshes.
    void draw() const;

private:
    /// ## Model::load_by_assimp
    ///
    /// Loads a model using the Assimp library.
    ///
    /// @param filepath: The path to the model file.
    ///
    /// @returns `true` if the model is loaded successfully, `false` otherwise.
    bool load_by_assimp(const std::string &filepath);

    /// ## Model::process_node
    ///
    /// Processes a node in the Assimp scene hierarchy.
    ///
    /// @param node: Pointer to the Assimp node.
    /// @param scene: Pointer to the Assimp scene.
    void process_node(const aiNode *node, const aiScene *scene);

    /// ## Model::process_mesh
    ///
    /// Processes a mesh in the Assimp scene.
    ///
    /// @param mesh: Pointer to the Assimp mesh.
    void process_mesh(const aiMesh *mesh);

    Model() {}
};


#endif
