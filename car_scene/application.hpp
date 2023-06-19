// ################################################################################
// Common Framework for Computer Graphics Courses at FI MUNI.
//
// Copyright (c) 2021-2022 Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// ################################################################################

#pragma once

#include "camera.h"
#include "cube.hpp"
#include "pv112_application.hpp"
#include "sphere.hpp"
#include "teapot.hpp"
#include <irrKlang.h>

// ----------------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------------

#define FRONT 0
#define LEFT 1
#define BACK 2
#define RIGHT 3
#define SPHERE_OBJ 0
#define LAMP_OBJ 1
#define CUBE_OBJ 2
#define CAR_OBJ 3
#define TREE_OBJ 4
#define NUM_OF_TREE_OBJS 6
#define HOUSES_OBJ ((TREE_OBJ + NUM_OF_TREE_OBJS))
#define NUM_OF_HOUSES 4

#define SOUND

// ----------------------------------------------------------------------------
// CUSTOM STRUCTS
// ----------------------------------------------------------------------------

struct keys {
    bool w;
    bool a;
    bool s;
    bool d;
};

struct p_camera {
    glm::vec3 camera_pos   = glm::vec3(.0f, -.25f, .5f);
    glm::vec3 camera_front = glm::vec3(1.f,   .0f, .0f);
    glm::vec3 camera_up    = glm::vec3(.0f,   1.f, .0f);
    double yaw   = -90.0f;
    double pitch = 0.f;
};

struct player_vars {
    double lastX;
    double lastY;
    struct p_camera cam;
    struct keys wasd = {false, false, false, false};
    bool sprint = false;
};

struct tree_stats {
    float x;
    float y;
    float scale;
    float rotation;
    int object;
};

// ----------------------------------------------------------------------------
// UNIFORM STRUCTS
// ----------------------------------------------------------------------------
struct CameraUBO {
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 position;
};

struct alignas(16) LightUBO {
    glm::vec4 position;

    glm::vec4 ambient_color;
    glm::vec4 diffuse_color;
    glm::vec4 specular_color;

    glm::vec4 direction;
    glm::vec4 cut_off; // only 1st & 2nd values
};

struct alignas(256) ObjectUBO {
    glm::mat4 model_matrix;  // [  0 -  64) bytes
    glm::vec4 ambient_color; // [ 64 -  80) bytes
    glm::vec4 diffuse_color; // [ 80 -  96) bytes

    // Contains shininess in .w element
    glm::vec4 specular_color; // [ 96 - 112) bytes
};

struct FogUBO {
    glm::vec4 color;
    float density;
};

// Constants
const float clear_color[4] = {0.0, 0.0, 0.0, 1.0};
const float clear_depth[1] = {1.0};

class Application : public PV112Application {

    // ----------------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------------
    std::filesystem::path images_path;
    std::filesystem::path objects_path;
    std::filesystem::path assets_path;
    std::filesystem::path trees_objs_path;
    std::filesystem::path trees_text_path;

    // Main program
    GLuint main_program;
    // TODO: feel free to add as many as you need/like
    GLuint lights_program;
    GLuint skybox_program;

    // List of geometries used in the project
    std::vector<std::shared_ptr<Geometry>> geometries;
    int num_of_street_lights = 19;
    int num_of_pathways = 6;
    std::vector<glm::vec2> pathway_scales;

    // Object and light positions in vector
    int ground;
    int road;
    int car;
    int lamp_s;
    int yellow_s;
    int car_lights;
    int car_lights_s;
    int trees;
    int houses;
    int pathways;

    // Random generated trees
    std::vector<tree_stats> generated_trees;
    float minimal_gap = .5f;
    int num_of_trees = 1500;
    int num_of_static_trees = 1;

#ifdef SOUND
    // Sound variables
    irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();
    irrklang::ISoundSource* car_source;
    irrklang::ISoundSource* click_source;
    irrklang::ISoundSource* slider_source;
    irrklang::ISound* car_sound;
#endif

    // Animation variables
    float start_pos = -14.f;
    glm::vec3 car_pos = glm::vec3(start_pos, -0.28f, 0.8f);
    glm::vec3 car_lights_pos[4] = {
        glm::vec3(.287f, -.296f - car_pos.y, .685f - car_pos.z),
        glm::vec3(.287f, -.296f - car_pos.y, .915f - car_pos.z),
        glm::vec3(-.289f, -.285f - car_pos.y, .654f - car_pos.z),
        glm::vec3(-.289f, -.285f - car_pos.y, .946f - car_pos.z)
    };
    int blinking_light = 10;

    // Default camera that rotates around center.
    Camera camera;

    // UBOs
    CameraUBO camera_ubo;
    GLuint camera_buffer = 0;

    ObjectUBO car_ubo;
    GLuint car_buffer = 0;

    std::vector<LightUBO> lights;
    GLuint lights_buffer = 0;

    std::vector<ObjectUBO> objects_ubos;
    GLuint objects_buffer = 0;

    FogUBO fog_ubo;
    GLuint fog_buffer = 0;

    // Player variables
    float delta_time = 0.0f; // Time between current frame and last frame
    float last_frame = 0.0f; // Time of last frame
    float last_sec = float(glfwGetTime());
    struct player_vars plr = {static_cast<double>(width) / 2, static_cast<double>(height) / 2};

    // Textures
    GLuint skybox = 0;
    GLuint road_texture = 0;
    GLuint car_texture = 0;
    GLuint tree_6_texture = 0;
    GLuint ground_texture = 0;
    GLuint pathway_texture = 0;

    GLuint tree_textures[NUM_OF_TREE_OBJS];
    GLuint house_textures[NUM_OF_HOUSES];

    // Global booleans
    bool p_view = true;
    bool first_mouse = true;
    bool show_cursor = false;
    bool toon_shading = false;
    bool light_on = true;
    bool night_vision = false;
    bool ui = false;
    bool controls = false;
    bool settings = false;
    bool fog = false;

    // Global variables
    int car_pause = 1; // 0 - paused, 1 - not paused
    int toon_levels = 10;

    // ----------------------------------------------------------------------------
    // Constructors & Destructors
    // ----------------------------------------------------------------------------
  public:
    /**
     * Constructs a new @link Application with a custom width and height.
     *
     * @param 	initial_width 	The initial width of the window.
     * @param 	initial_height	The initial height of the window.
     * @param 	arguments	  	The command line arguments used to obtain the application directory.
     */
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    void init();

    /** Destroys the {@link Application} and releases the allocated resources. */
    ~Application() override;

    // ----------------------------------------------------------------------------
    // Methods
    // ----------------------------------------------------------------------------

    /** @copydoc PV112Application::compile_shaders */
    void compile_shaders() override;

    /** @copydoc PV112Application::delete_shaders */
    void delete_shaders() override;

    /** @copydoc PV112Application::update */
    void update(float delta) override;

    /** @copydoc PV112Application::render */
    void render() override;

    /** @copydoc PV112Application::render_ui */
    void render_ui() override;

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------

    /** @copydoc PV112Application::on_resize */
    void on_resize(int width, int height) override;

    /** @copydoc PV112Application::on_mouse_move */
    void on_mouse_move(double x, double y) override;

    /** @copydoc PV112Application::on_mouse_button */
    void on_mouse_button(int button, int action, int mods) override;

    /** @copydoc PV112Application::on_key_pressed */
    void on_key_pressed(int key, int scancode, int action, int mods) override;

    void key_released_callback(int key);

    // ----------------------------------------------------------------------------
    // Help Functions
    // ----------------------------------------------------------------------------

    float rand_float(float low, float high);

    unsigned int load_cubemap(std::vector<std::string> faces);
};
