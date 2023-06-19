// ################################################################################
// Common Framework for Computer Graphics Courses at FI MUNI.
//
// Copyright (c) 2021-2022 Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// ################################################################################

#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application {
    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
  protected:
    /** The lake object. */
    SceneObject lake_object;
    /** The outer terrain object. */
    SceneObject outer_terrain_object;
    /** The snow terrain object. */
    SceneObject snow_terrain_object;
    /** The grassy castle base. */
    SceneObject castel_base;
    /** The castle object. */
    SceneObject castle_object;

    /** The broom position. */
    glm::vec2 broom_center;

    /** The scene object storing information about the broom model. */
    SceneObject broom_object;
    /** The scene object representing the light. */
    SceneObject light_object;

    /** The data for the particles. */
    GLuint snow_positions_bo;
    GLuint snow_vao;

    // ----------------------------------------------------------------------------
    // Variables (Materials)
    // ----------------------------------------------------------------------------
  protected:
    /** The material for the terrain. */
    PhongMaterialUBO brown_material_ubo;
    /** The material for the castle. */
    PhongMaterialUBO castle_material_ubo;
    /** The material for the snow. */
    PhongMaterialUBO snow_material_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
  protected:
    /** The texture for broom. */
    GLuint broom_tex;
    /** The ice texture. */
    GLuint ice_albedo_tex;
    /** The texture for the snow particles. */
    GLuint particle_tex;
    /** The snow albedo texture. */
    GLuint snow_albedo_tex;
    /** The snow normal texture. */
    GLuint snow_normal_tex;
    /** The snow height texture. */
    GLuint snow_height_tex;
    /** The snow roughness. */
    GLuint snow_roughness_tex;

    /** The accumulated snow textures: snow1,snow2,depth1,depth2 */
    GLuint accumulated_snow_tex[2];
    /** The orthogonal view texture. */
    GLuint ortho_tex;
    /** The orthogonal view depth texture. */
    GLuint ortho_depth_tex;

    // ----------------------------------------------------------------------------
    // Variables (Light)
    // ----------------------------------------------------------------------------
  protected:
    /** The UBO storing the data about lights - positions, colors, etc. */
    PhongLightsUBO phong_lights_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Camera)
    // ----------------------------------------------------------------------------
  protected:
    /** The camera projection matrix. */
    glm::mat4 projection_matrix;
    /** The orthogonal camera projection matrix. */
    glm::mat4 ortho_proj_matrix;
    /** The orthogonal camera view matrix. */
    glm::mat4 ortho_view_matrix;
    /** Precomputed snow matrix. */
    glm::mat4 snow_matrix;
    /** The UBO storing the information about the camera. */
    CameraUBO camera_ubo;
    CameraUBO ortho_camera_ubo;

    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------
  protected:
    /** A shader for rendering snowflakes. */
    ShaderProgram snowflakes_program;
    /** A shader for rendering tesselation snow. */
    ShaderProgram snow_program;
    /** A shader for rendering textures. */
    ShaderProgram display_texture_program;
    /** A shader for accumulating snow. */
    ShaderProgram accumulate_snow_program;
    /** A shader rendering broom into accumulated snow. */
    ShaderProgram broom_program;
    /** A shader bluring a texture. */
    ShaderProgram blur_program;

    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
  protected:
      GLuint accumulated_snow_fbo[2];
      GLuint ortho_fbo;

    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
  protected:
    long double last_frame = elapsed_time;
    const int DISPLAY_FINAL_IMAGE = 0;
    const int DISPLAY_SNOW_1 = 1;
    const int DISPLAY_SNOW_2 = 2;
    const int DISPLAY_ORTHO = 3;
    const char* DISPLAY_LABELS[4] = {
        "Final Image",
        "Snow tex 1",
        "Snow tex 2",
        "Ortho"
    };
    int what_to_display = DISPLAY_FINAL_IMAGE;

    /** The light position set in the GUI. */
    float gui_light_position = glm::radians(360.f);

    /** The desired snow particle count. */
    int desired_snow_count = 2048;

    /** The current snow particle count. */
    int current_snow_count = 256;

    /** The flag determining if a snow should be visible. */
    bool show_snow = true;

    /** The flag determining polygon fill mode. */
    bool wireframe = false;

    /** The flag determining if PBR model will be used. */
    bool pbr = false;

    /** The flag determining if tessellated snow will be visible. */
    bool show_tessellated_snow = true;

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
  public:
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    /** Destroys the {@link Application} and releases the allocated resources. */
    ~Application() override;

    // ----------------------------------------------------------------------------
    // Shaders
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::compile_shaders}
     */
    void compile_shaders() override;

    // ----------------------------------------------------------------------------
    // Initialize Scene
    // ----------------------------------------------------------------------------
  public:
    /** Prepares the required cameras. */
    void prepare_cameras();

    /** Prepares the required materials. */
    void prepare_materials();

    /** Prepares the required textures. */
    void prepare_textures();

    /** Prepares the lights. */
    void prepare_lights();

    /** Prepares the scene objects. */
    void prepare_scene();

    /** Prepares the snow.*/
    void prepare_snow();

    /** Prepares the frame buffer objects. */
    void prepare_framebuffers();

    /** Resizes the full screen textures match the window. */
    void resize_fullscreen_textures();

    /** Initializes textures for snow accumulation. */
    void initialize_fbo_textures();

    /** Applies blur. */
    void apply_blur();

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc PV227Application::update}
     */
    void update(float delta) override;

    /**
     * Updates the broom location. Note that this method expects that the currently bind depth buffer contains
     * information about the whole scene (except the broom itself).
     */
    void update_broom_location();

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
  public:
    /** @copydoc PV227Application::render */
    void render() override;

    /** Renders the whole scene. */
    void render_scene(const ShaderProgram& program, bool render_broom);

    /** Renders final scene */
    void render_final();

    /** Renders texture on the screen. */
    void show_texture(GLuint texture);

    /** Renders the snow.*/
    void render_snow();

    /** Renders the specified object. */
    void render_object(const SceneObject& object, const ShaderProgram& program, bool render_as_patches, float uv_multiplier = 1) const;

    /** Clears the accumulated snow. */
    void clear_accumulated_snow();

    /** Accumulates snow in textures. */
    void accumulate_snow();

    /** Renders scene with ortho view from above. */
    void render_ortho();

    /** Renders tesselated snow. */
    void render_tese_snow();

    /** Renders broom as black to accumulated snow texture. */
    void use_broom();

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
  public:
    /** @copydoc PV227Application::render_ui */
    void render_ui() override;

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------
  public:
    /** @copydoc PV227Application::on_resize */
    void on_resize(int width, int height) override;

    /** @copydoc PV227Application::on_mouse_move */
    void on_mouse_move(double x, double y) override;
};
