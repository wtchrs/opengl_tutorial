#include <cstddef>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"
#include "glex/context.h"
#include "glex/image.h"
#include "glex/mesh.h"

namespace {

    struct Object {
        glm::vec3 pos;
        glm::vec3 scale;
        glm::vec3 rotDir;
        float rotAngle;
        std::shared_ptr<Mesh> mesh;
    };

} // namespace

class PBRTexture : Context {
    std::unique_ptr<Program> simple_program_, pbr_program_;
    std::shared_ptr<Mesh> cube_mesh_, plain_mesh_, sphere_mesh_;

    struct Material {
        std::unique_ptr<Texture> albedo;
        std::unique_ptr<Texture> normal;
        std::unique_ptr<Texture> metallic;
        std::unique_ptr<Texture> roughness;
        float ao;
    };
    Material material_;

    struct Light {
        glm::vec3 position;
        glm::vec3 color;
    };
    std::vector<Light> lights;

public:
    bool init();
    void render();
    void draw_ui();
    void draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program);
    void reshape(int width, int height);
};

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{reinterpret_cast<Context *>(new PBRTexture{})};
    if (!context->init()) {
        SPDLOG_ERROR("Failed to create context");
        return nullptr;
    }
    SPDLOG_INFO("Context has been created");
    return std::move(context);
}

bool PBRTexture::init() {
    // Create meshes.
    cube_mesh_ = Mesh::create_cube();
    plain_mesh_ = Mesh::create_plain();
    sphere_mesh_ = Mesh::create_sphere();

    material_.albedo = Texture::create(*Image::load("./image/rusted_iron/rustediron2_basecolor.png", false));
    material_.normal = Texture::create(*Image::load("./image/rusted_iron/rustediron2_normal.png", false));
    material_.metallic = Texture::create(*Image::load("./image/rusted_iron/rustediron2_metallic.png", false));
    material_.roughness = Texture::create(*Image::load("./image/rusted_iron/rustediron2_roughness.png", false));

    // Load programs.
    simple_program_ = Program::create("./shader/simple.vs", "./shader/simple.fs");
    pbr_program_ = Program::create("./shader/pbr_texture.vs", "./shader/pbr_texture.fs");

    if (!simple_program_ || !pbr_program_) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    lights.emplace_back(glm::vec3{5.0f, 5.0f, 6.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights.emplace_back(glm::vec3{-4.0f, 5.0f, 7.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights.emplace_back(glm::vec3{-4.0f, -6.0f, 8.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights.emplace_back(glm::vec3{5.0f, -6.0f, 9.0f}, glm::vec3{40.0f, 40.0f, 40.0f});

    // Enable depth test and cull face.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return true;
}

void PBRTexture::render() {
    // Clear color buffer with `glClearColor` and depth buffer with 1.0.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Dear ImGui UI
    draw_ui();

    // Calculate camera front direction.
    camera_front_ = glm::rotate(glm::mat4{1.0f}, glm::radians(camera_yaw_), glm::vec3{0.0f, 1.0f, 0.0f}) *
                    glm::rotate(glm::mat4{1.0f}, glm::radians(camera_pitch_), glm::vec3{1.0f, 0.0f, 0.0f}) *
                    glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

    // Projection and view matrix
    // When the `Near` value is too small, inaccurate depth test, known as "z-fighting", arise on far objects,
    // due to the z-value distortion introduced by the projection transform.
    const auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio_, 0.01f, 150.0f);
    const auto view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

    const auto &program = *pbr_program_;
    program.use();
    for (size_t i = 0; i < lights.size(); ++i) {
        const auto pos_name = std::format("lights[{}].position", i);
        const auto color_name = std::format("lights[{}].color", i);
        program.set_uniform(pos_name, lights[i].position);
        program.set_uniform(color_name, lights[i].color);
    }
    program.set_uniform("viewPos", camera_pos_);
    glActiveTexture(GL_TEXTURE0);
    material_.albedo->bind();
    glActiveTexture(GL_TEXTURE1);
    material_.normal->bind();
    glActiveTexture(GL_TEXTURE2);
    material_.metallic->bind();
    glActiveTexture(GL_TEXTURE3);
    material_.roughness->bind();
    program.set_uniform("material.albedo", 0);
    program.set_uniform("material.normal", 1);
    program.set_uniform("material.metallic", 2);
    program.set_uniform("material.roughness", 3);
    program.set_uniform("material.ao", material_.ao);
    draw_scene(view, projection, program);
}

void PBRTexture::draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) {
    program.use();
    const int sphere_count = 7;
    const float offset = 1.2f;
    for (size_t j = 0; j < sphere_count; ++j) {
        const float y = (static_cast<float>(j) - static_cast<float>(sphere_count - 1) * 0.5f) * offset;
        for (size_t i = 0; i < sphere_count; ++i) {
            const float x = (static_cast<float>(i) - static_cast<float>(sphere_count - 1) * 0.5f) * offset;
            const auto model_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{x, y, 0.0f});
            const auto transform = projection * view * model_transform;
            program.set_uniform("modelTransform", model_transform);
            program.set_uniform("transform", transform);
            sphere_mesh_->draw(program);
        }
    }
}

void PBRTexture::draw_ui() {
    // ImGui Components.
    if (ImGui::Begin("UI")) {
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", glm::value_ptr(camera_pos_), 0.1f);
            ImGui::DragFloat("Yaw", &camera_yaw_, 0.5f);
            ImGui::DragFloat("Pitch", &camera_pitch_, 0.5f, -89.0f, 89.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            static int idx = 0;
            ImGui::DragInt("Light index", &idx, 1, 0, 3);
            ImGui::DragFloat3("Light position", glm::value_ptr(lights[idx].position), 0.01f);
            ImGui::DragFloat3("Light color", glm::value_ptr(lights[idx].color), 0.1f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Material AO", &material_.ao, 0.0f, 1.0f);
        }
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos_ = CAMERA_POS;
            camera_yaw_ = CAMERA_YAW;
            camera_pitch_ = CAMERA_PITCH;
        }
    }
    ImGui::End();
}

void PBRTexture::reshape(const int width, const int height) {
    width_ = width;
    height_ = height;
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
}
