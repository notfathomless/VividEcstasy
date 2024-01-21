#pragma once
#include <common/common.hpp>
#include <scene/scene.hpp>
#include <ecstasy/ecstasy.hpp>
#include <controller/InputController.hpp>
#include <controller/EditorController.hpp>
#include <memory>

#include <filament/FilamentAPI.h>
#include <filament/Engine.h>
#include <filament/Options.h>
#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/Color.h>
#include <filament/VertexBuffer.h>
#include <filament/Fence.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filamat/MaterialBuilder.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Stream.h>
#include <filament/SwapChain.h>
#include <filament/View.h>
#include <filament/Texture.h>
#include <filament/Skybox.h>
#include <filament/TextureSampler.h>
#include <filament/Viewport.h>
#include <filament/IndirectLight.h>
#include <filament-iblprefilter/IBLPrefilterContext.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <math/norm.h>
#include <math/mat3.h>
#include <numbers>

#include <common/imgui.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <fstream>
#include <filesystem>

#include "stb_image.h"

namespace ecstasy {

Eigen::Quaternionf getQuatFromEuler(Eigen::Vector3f _euler) {
    Eigen::Quaternionf q;
    q = Eigen::AngleAxisf(_euler.x(), Eigen::Vector3f::UnitX()) *
        Eigen::AngleAxisf(_euler.y(), Eigen::Vector3f::UnitY()) *
        Eigen::AngleAxisf(_euler.z(), Eigen::Vector3f::UnitZ());
    q.normalize();

    return q;
}

Eigen::Vector4f getFloat4FromEuler(Eigen::Vector3f _euler) {
    auto q = getQuatFromEuler(_euler);
    return Eigen::Vector4f{q.x(), q.y(), q.z(), q.w()};
}

namespace scene {

const static std::vector<uint32_t> indices = {0, 1, 2};

const static std::vector<Eigen::Vector3f> vertices = {
    Eigen::Vector3f{-5, -5, 0},
    Eigen::Vector3f{0, 5, 0},
    Eigen::Vector3f{5, -5, 0},
};

const static std::vector<Eigen::Vector4f> normals{getFloat4FromEuler({0.0f, 0.0f, 1.0f}),
                                                  getFloat4FromEuler({0.0f, 0.0f, 1.0f}),
                                                  getFloat4FromEuler({0.0f, 0.0f, 1.0f})};

class sandbox : public scene {
    app* app_;
    filament::Engine* filament_engine_;
    filament::Renderer* renderer_;
    utils::Entity camera_entity_;
    filament::Camera* camera_;
    filament::View* view_;
    filament::Scene* scene_;
    filament::Skybox* skybox_;
    InputController* input_controller_;
    EditorController* editor_controller_;
    filament::Texture* skybox_texture_;
    filament::Texture* ibl_texture_;
    filament::Texture* fog_texture_;
    filament::IndirectLight* indirect_light_;

    filament::VertexBuffer* vertex_buffer_;
    filament::IndexBuffer* index_buffer_;
    filament::Material* material_;
    filament::MaterialInstance* material_instance_;

    utils::Entity light_;
    utils::Entity renderable_;

  public:
    sandbox() = delete;

    sandbox(filament::Engine* _filament_engine, filament::Renderer* _renderer,
            InputController* _input_controller) {

        filament_engine_ = _filament_engine;
        renderer_ = _renderer;
        auto viewport_dimension = _input_controller->getViewportDimension();

        /*         auto path = std::filesystem::path("../hdri/dancing_hall_8k.hdr").lexically_normal();

                int w, h;
                stbi_info(path.c_str(), &w, &h, nullptr);
                log::info("{}, {}", w, h);
                if (w != h * 2) {
                    log::error("not an equirectangular image!");
                }

                // load image as float
                int n;
                const size_t size = w * h * sizeof(filament::math::float3);
                filament::math::float3* const data = (filament::math::float3*)stbi_loadf(path.c_str(), &w, &h,
           &n, 3); if (data == nullptr || n != 3) { log::error("Could not decode image ");
                }

                // now load texture
                filament::Texture::PixelBufferDescriptor buffer(
                    data, size, filament::Texture::Format::RGB, filament::Texture::Type::FLOAT,
                    [](void* buffer, size_t size, void* user) { stbi_image_free(buffer); }); */

        // filament::Texture* const equirect = filament::Texture::Builder()
        //                                         .width((uint32_t)w)
        //                                         .height((uint32_t)h)
        //                                         .levels(0xff)
        //                                         .format(filament::Texture::InternalFormat::R11F_G11F_B10F)
        //                                         .sampler(filament::Texture::Sampler::SAMPLER_2D)
        //                                         .build(*filament_engine_);

        // equirect->setImage(*filament_engine_, 0, std::move(buffer));

        // IBLPrefilterContext context(*filament_engine_);
        // IBLPrefilterContext::EquirectangularToCubemap equirectangularToCubemap(context);
        // IBLPrefilterContext::SpecularFilter specularFilter(context);
        // IBLPrefilterContext::IrradianceFilter irradianceFilter(context);

        // skybox_texture_ = equirectangularToCubemap(equirect);
        // filament_engine_->destroy(equirect);

        // ibl_texture_ = specularFilter(skybox_texture_);

        // fog_texture_ = irradianceFilter({.generateMipmap = false}, skybox_texture_);
        // fog_texture_->generateMipmaps(*filament_engine_);

        auto cameraEntity = utils::EntityManager::get().create();
        camera_ = filament_engine_->createCamera(cameraEntity);
        view_ = filament_engine_->createView();
        scene_ = filament_engine_->createScene();

        indirect_light_ = filament::IndirectLight::Builder()
                              .reflections(ibl_texture_)
                              .intensity(60000.0f)
                              .build(*filament_engine_);
        scene_->setIndirectLight(indirect_light_);

        // skybox_ =
        //     filament::Skybox::Builder().environment(skybox_texture_).showSun(false).build(*filament_engine_);

        // light_ = utils::EntityManager::get().create();
        /*filament::LightManager::Builder(filament::LightManager::Type::SUN)
            .color(filament::Color::toLinear<filament::ACCURATE>(filament::sRGBColor(0.98f, 0.92f, 0.89f)))
            .intensity(150'000)
            .direction({0, 0, 5})
            .sunAngularRadius(1.9f)
            .castShadows(true)
            .build(*filament_engine_, light_);

        scene_->addEntity(light_);*/

        skybox_ = filament::Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*filament_engine_);

        scene_->setSkybox(skybox_);

        view_->setCamera(camera_);
        view_->setScene(scene_);
        view_->setViewport({0, 0, static_cast<std::uint32_t>(viewport_dimension.x()),
                            static_cast<std::uint32_t>(viewport_dimension.y())});

        editor_controller_ =
            new EditorController(_input_controller, camera_ /* , {0, 0, 50.0f}, {0, 0, 0} */);

        view_->setPostProcessingEnabled(false);

        vertex_buffer_ = filament::VertexBuffer::Builder()
                             .vertexCount(3)
                             .bufferCount(2)
                             .attribute(filament::VertexAttribute::POSITION, 0,
                                        filament::VertexBuffer::AttributeType::FLOAT3)
                             .attribute(filament::VertexAttribute::TANGENTS, 1,
                                        filament::VertexBuffer::AttributeType::FLOAT4)
                             .normalized(filament::VertexAttribute::TANGENTS)
                             .build(*filament_engine_);

        vertex_buffer_->setBufferAt(
            *filament_engine_, 0,
            filament::VertexBuffer::BufferDescriptor(vertices.data(),
                                                     vertex_buffer_->getVertexCount() * sizeof(vertices[0])));
        vertex_buffer_->setBufferAt(
            *filament_engine_, 1,
            filament::VertexBuffer::BufferDescriptor(normals.data(),
                                                     vertex_buffer_->getVertexCount() * sizeof(normals[0])));

        index_buffer_ = filament::IndexBuffer::Builder().indexCount(6).build(*filament_engine_);

        index_buffer_->setBuffer(*filament_engine_,
                                 filament::IndexBuffer::BufferDescriptor(
                                     indices.data(), index_buffer_->getIndexCount() * sizeof(uint32_t)));

        filamat::MaterialBuilder::init();
        filamat::MaterialBuilder builder;
        ecstasy::shader::simple(builder);
        filamat::Package package = builder.build(filament_engine_->getJobSystem());

        material_ = filament::Material::Builder()
                        .package(package.getData(), package.getSize())
                        .build(*filament_engine_);
        material_->setDefaultParameter("baseColor", filament::RgbType::LINEAR,
                                       filament::math::float3{0, 1, 0});
        material_->setDefaultParameter("metallic", 0.0f);
        material_->setDefaultParameter("roughness", 0.4f);
        material_->setDefaultParameter("reflectance", 0.5f);

        material_instance_ = material_->createInstance();

        renderable_ = utils::EntityManager::get().create();

        filament::RenderableManager::Builder(1)
            .boundingBox({{-1, -1, -1}, {1, 1, 1}})
            .material(0, material_instance_)
            .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, vertex_buffer_, index_buffer_,
                      0, 3)
            .culling(false)
            //.receiveShadows(false)
            //.castShadows(false)
            .build(*filament_engine_, renderable_);

        scene_->addEntity(renderable_);
    }

    void build() {}
    void animate(std::chrono::steady_clock::duration _last_animation_time) {

        /* ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame(); */

        editor_controller_->animate(_last_animation_time);

        renderer_->render(view_);

        // Render UI

        /*   ImGui::Render();
          ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); */
    }

    void destroy() {
        // ImGui_ImplOpenGL3_Shutdown();
        // ImGui_ImplGlfw_Shutdown();
        // ImGui::DestroyContext();
    }
};
} // namespace scene

} // namespace ecstasy