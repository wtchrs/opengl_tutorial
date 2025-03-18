#include <cstddef>
#include <cstdint>
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
#include "glex/framebuffer.h"
#include "glex/image.h"
#include "glex/mesh.h"
#include "glex/program.h"
#include "glex/texture.h"

class IBL : Context {
    std::unique_ptr<Program> simple_program_, pbr_program_, spherical_map_program_, skybox_program_,
            diffuse_irradiance_program_, prefiltered_program_, brdf_lookup_program_;
    std::shared_ptr<Mesh> cube_mesh_, plain_mesh_, sphere_mesh_;
    std::unique_ptr<Texture> hdr_map_;
    std::shared_ptr<Texture> brdf_lookup_map_;
    std::shared_ptr<CubeTexture> hdr_cube_map_, diffuse_irradiance_map_, prefiltered_map_;

    struct Material {
        glm::vec3 albedo;
        float metallic;
        float roughness;
        float ao;
    };
    static constexpr Material DEFAULT_MATERIAL = {glm::vec3{1.0f}, 0.5f, 0.5f, 0.1f};
    Material material_ = DEFAULT_MATERIAL;

    struct Light {
        glm::vec3 position;
        glm::vec3 color;
    };
    std::vector<Light> lights_;

    bool use_ibl_{true};

public:
    bool init() override;
    void render() override;
    void draw_ui() override;
    void draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) override;
    void reshape(int width, int height) override;
};

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{reinterpret_cast<Context *>(new IBL{})};
    if (!context->init()) {
        SPDLOG_ERROR("Failed to create context");
        return nullptr;
    }
    SPDLOG_INFO("Context has been created");
    return std::move(context);
}

bool IBL::init() {
    // Enable depth test and cull face.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Create meshes.
    cube_mesh_ = Mesh::create_cube();
    plain_mesh_ = Mesh::create_plain();
    sphere_mesh_ = Mesh::create_sphere();

    // Load programs.
    simple_program_ = Program::create("./shader/simple.vs", "./shader/simple.fs");
    pbr_program_ = Program::create("./shader/pbr.vs", "./shader/pbr_with_ibl.fs");
    spherical_map_program_ = Program::create("./shader/spherical_map.vs", "./shader/spherical_map.fs");
    skybox_program_ = Program::create("./shader/skybox_hdr.vs", "./shader/skybox_hdr.fs");
    diffuse_irradiance_program_ = Program::create("./shader/skybox_hdr.vs", "./shader/diffuse_irradiance.fs");
    prefiltered_program_ = Program::create("./shader/skybox_hdr.vs", "./shader/prefiltered_light.fs");
    brdf_lookup_program_ = Program::create("./shader/brdf_lookup.vs", "./shader/brdf_lookup.fs");

    if (!simple_program_ || !pbr_program_ || !spherical_map_program_ || !skybox_program_ ||
        !diffuse_irradiance_program_ || !prefiltered_program_ || !brdf_lookup_program_) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    lights_.emplace_back(glm::vec3{5.0f, 5.0f, 6.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights_.emplace_back(glm::vec3{-4.0f, 5.0f, 7.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights_.emplace_back(glm::vec3{-4.0f, -6.0f, 8.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights_.emplace_back(glm::vec3{5.0f, -6.0f, 9.0f}, glm::vec3{40.0f, 40.0f, 40.0f});

    // For draw cube map.
    auto projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    std::vector<glm::mat4> views = {
            glm::lookAt(glm::vec3{0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0, -1.0, 0.0f}),
            glm::lookAt(glm::vec3{0.0f}, glm::vec3{-1.0f, 0.0f, 0.0f}, glm::vec3{0.0, -1.0, 0.0f}),
            glm::lookAt(glm::vec3{0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0, 0.0, 1.0f}),
            glm::lookAt(glm::vec3{0.0f}, glm::vec3{0.0f, -1.0f, 0.0f}, glm::vec3{0.0, 0.0, -1.0f}),
            glm::lookAt(glm::vec3{0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0, -1.0, 0.0f}),
            glm::lookAt(glm::vec3{0.0f}, glm::vec3{0.0f, 0.0f, -1.0f}, glm::vec3{0.0, -1.0, 0.0f}),
    };

    // Generate HDR cube map from equirectangular map.
    hdr_map_ = Texture::create(*Image::load("./image/Alexs_Apt_2k.hdr"));
    hdr_cube_map_ = CubeTexture::create(1024, 1024, GL_RGB16F, GL_FLOAT);
    spherical_map_program_->use();
    hdr_map_->bind();
    spherical_map_program_->set_uniform("tex", 0);
    auto cube_framebuffer = CubeFrameBuffer::create(hdr_cube_map_);
    glViewport(0, 0, 1024, 1024);
    for (size_t i = 0; i < 6; ++i) {
        cube_framebuffer->bind(i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        spherical_map_program_->set_uniform("transform", projection * views[i]);
        cube_mesh_->draw(*spherical_map_program_);
    }
    hdr_cube_map_->generate_mipmap();

    // Generate diffuse irradiance map from cube map.
    diffuse_irradiance_map_ = CubeTexture::create(64, 64, GL_RGB16F, GL_FLOAT);
    diffuse_irradiance_program_->use();
    hdr_cube_map_->bind();
    diffuse_irradiance_program_->set_uniform("cubeMap", 0);
    diffuse_irradiance_program_->set_uniform("projection", projection);
    cube_framebuffer = CubeFrameBuffer::create(diffuse_irradiance_map_);
    glViewport(0, 0, 64, 64);
    glDepthFunc(GL_LEQUAL);
    for (size_t i = 0; i < 6; ++i) {
        cube_framebuffer->bind(i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        diffuse_irradiance_program_->set_uniform("view", views[i]);
        cube_mesh_->draw(*diffuse_irradiance_program_);
    }
    glDepthFunc(GL_LESS);

    // Generate prefiltered map.
    uint32_t max_mip_levels = 5;
    prefiltered_map_ = CubeTexture::create(128, 128, GL_RGB16F, GL_FLOAT);
    prefiltered_map_->generate_mipmap();
    prefiltered_program_->use();
    prefiltered_program_->set_uniform("projection", projection);
    hdr_cube_map_->bind();
    prefiltered_program_->set_uniform("cubeMap", 0);
    glDepthFunc(GL_LEQUAL);
    for (uint32_t mip = 0; mip < max_mip_levels; ++mip) {
        const auto framebuffer = CubeFrameBuffer::create(prefiltered_map_, mip);
        const uint32_t mip_width = 128 >> mip;
        const uint32_t mip_height = 128 >> mip;
        glViewport(0, 0, mip_width, mip_height);
        const float roughness = static_cast<float>(mip) / static_cast<float>(max_mip_levels - 1);
        prefiltered_program_->set_uniform("roughness", roughness);
        for (size_t i = 0; i < 6; ++i) {
            prefiltered_program_->set_uniform("view", views[i]);
            framebuffer->bind(i);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube_mesh_->draw(*prefiltered_program_);
        }
    }
    glDepthFunc(GL_LESS);

    // Generate BRDF lookup table map.
    brdf_lookup_map_ = Texture::create(512, 512, GL_RG16F, GL_FLOAT);
    const auto lookup_framebuffer = FrameBuffer::create({brdf_lookup_map_});
    lookup_framebuffer->bind();
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    brdf_lookup_program_->use();
    brdf_lookup_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f, -2.0, 2.0f}));
    plain_mesh_->draw(*brdf_lookup_program_);

    // Restore to default framebuffer.
    FrameBuffer::bind_to_default();
    glViewport(0, 0, width_, height_);

    glEnable(GL_CULL_FACE);

    return true;
}

void IBL::render() {
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

    skybox_program_->use();
    skybox_program_->set_uniform("projection", projection);
    skybox_program_->set_uniform("view", view);
    hdr_cube_map_->bind();
    skybox_program_->set_uniform("cubeMap", 0);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    cube_mesh_->draw(*skybox_program_);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    /*
    spherical_map_program_->use();
    spherical_map_program_->set_uniform(
            "transform", projection * view * glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, 2.0f})
    );
    glActiveTexture(GL_TEXTURE0);
    hdr_map_->bind();
    spherical_map_program_->set_uniform("tex", 0);
    glDisable(GL_CULL_FACE);
    cube_mesh_->draw(*spherical_map_program_);
    glEnable(GL_CULL_FACE);
    */


    const auto &pbr = *pbr_program_;
    pbr.use();
    for (size_t i = 0; i < lights_.size(); ++i) {
        const auto pos_name = std::format("lights[{}].position", i);
        const auto color_name = std::format("lights[{}].color", i);
        pbr.set_uniform(pos_name, lights_[i].position);
        pbr.set_uniform(color_name, lights_[i].color);
    }
    pbr.set_uniform("viewPos", camera_pos_);
    glActiveTexture(GL_TEXTURE0);
    diffuse_irradiance_map_->bind();
    glActiveTexture(GL_TEXTURE1);
    prefiltered_map_->bind();
    glActiveTexture(GL_TEXTURE2);
    brdf_lookup_map_->bind();
    glActiveTexture(GL_TEXTURE0);
    pbr.set_uniform("irradianceMap", 0);
    pbr.set_uniform("prefilteredMap", 1);
    pbr.set_uniform("brdfLookupTable", 2);
    pbr.set_uniform("useIBL", use_ibl_);
    pbr.set_uniform("material.albedo", material_.albedo);
    pbr.set_uniform("material.ao", material_.ao);
    draw_scene(view, projection, pbr);
}

void IBL::draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) {
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
            program.set_uniform("material.roughness", static_cast<float>(i + 1) / static_cast<float>(sphere_count));
            program.set_uniform("material.metallic", static_cast<float>(j + 1) / static_cast<float>(sphere_count));
            sphere_mesh_->draw(program);
        }
    }
}

void IBL::draw_ui() {
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
            ImGui::DragFloat3("Light position", glm::value_ptr(lights_[idx].position), 0.01f);
            ImGui::DragFloat3("Light color", glm::value_ptr(lights_[idx].color), 0.1f);
            ImGui::Checkbox("Use IBL", &use_ibl_);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Material Albedo", glm::value_ptr(material_.albedo));
            ImGui::SliderFloat("Material Metallic", &material_.metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Material Roughness", &material_.roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Material AO", &material_.ao, 0.0f, 1.0f);
        }
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos_ = CAMERA_POS;
            camera_yaw_ = CAMERA_YAW;
            camera_pitch_ = CAMERA_PITCH;
        }
        float w = ImGui::GetContentRegionAvail().x;
        ImGui::Image(static_cast<ImTextureID>(brdf_lookup_map_->get()), ImVec2{w, w});
    }
    ImGui::End();
}

void IBL::reshape(const int width, const int height) {
    this->width_ = width;
    this->height_ = height;
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
}
