#include "application.hpp"

#include <memory>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using std::make_shared;

GLuint load_texture_2d(const std::filesystem::path filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.generic_string().data(), &width, &height, &channels, 4);

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    glTextureStorage2D(texture, GLsizei(std::log2(width)), GL_RGBA8, width, height);

    glTextureSubImage2D(texture,
                        0,                         //
                        0, 0,                      //
                        width, height,             //
                        GL_RGBA, GL_UNSIGNED_BYTE, //
                        data);

    stbi_image_free(data);

    glGenerateTextureMipmap(texture);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV112Application(initial_width, initial_height, arguments) {
    this->width = initial_width;
    this->height = initial_height;
    srand(unsigned int(time(0)));

    images_path = lecture_folder_path / "images";
    objects_path = lecture_folder_path / "objects";
    assets_path = lecture_folder_path / "other_assets";
    trees_objs_path = objects_path / "trees";
    trees_text_path = images_path / "trees";

    // --------------------------------------------------------------------------
    //  Load/Create Objects
    // --------------------------------------------------------------------------
    geometries.push_back(make_shared<Sphere>());
    // You can use from_file function to load a Geometry from .obj file
    Geometry meh = Geometry::from_file(objects_path / "street_lamp.obj");
    geometries.push_back(make_shared<Geometry>(meh));
    geometries.push_back(make_shared<Cube>());
    geometries.push_back(make_shared<Geometry>(Geometry::from_file(objects_path / "car.obj")));

    for (size_t i = 0; i < NUM_OF_TREE_OBJS; i++)
    {
        geometries.push_back(make_shared<Geometry>(Geometry::from_file(trees_objs_path / (std::to_string(i + 1) + "_tree.obj"))));
    }

    for (size_t i = 0; i < NUM_OF_HOUSES; i++)
    {
        geometries.push_back(make_shared<Geometry>(Geometry::from_file(objects_path / ("house_" + std::to_string(i + 1) +".obj"))));
    }

    // Object positions
    yellow_s = 0;
    lamp_s = yellow_s + num_of_street_lights;
    ground = lamp_s + num_of_street_lights;
    road = ground + 1;
    car = road + 1;
    car_lights_s = car + 1;
    trees = car_lights_s + 4;
    houses = trees + num_of_trees;
    pathways = houses + NUM_OF_HOUSES;

    // Light positions
    car_lights = num_of_street_lights + 1;

    // Texture loading
    stbi_set_flip_vertically_on_load(true);

    road_texture = load_texture_2d(images_path / "road.jpg");
    car_texture = load_texture_2d(images_path / "car.jpg");
    tree_6_texture = load_texture_2d(images_path / "trees/6_tree.png");
    ground_texture = load_texture_2d(images_path / "ground.jpg");
    pathway_texture = load_texture_2d(images_path / "pathway.jpg");

    for (size_t i = 0; i < NUM_OF_TREE_OBJS; i++)
    {
        tree_textures[i] = load_texture_2d(trees_text_path / (std::to_string(i + 1) + "_tree.png"));
    }

    for (size_t i = 0; i < NUM_OF_HOUSES; i++)
    {
        house_textures[i] = load_texture_2d(images_path / ("house_" + std::to_string(i + 1) +".png"));
    }

    glTextureParameteri(road_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(road_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(ground_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(ground_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::vector<std::string> faces = {
        "skybox_right.jpg",
        "skybox_left.jpg",
        "skybox_top.jpg",
        "skybox_bottom.jpg",
        "skybox_front.jpg",
        "skybox_back.jpg"
    };
    skybox = load_cubemap(faces); 


#ifdef SOUND
    // Sound setup
    car_source = engine->addSoundSourceFromFile((assets_path / "car_sound.mp3").string().c_str());
    irrklang::vec3df position(car_pos.x, car_pos.y, car_pos.z);

    car_sound = engine->play3D(car_source, position, true, true);
    car_sound->setMinDistance(100.f);
    car_sound->setVolume(0.f);
    car_sound->setIsPaused(false);

    click_source = engine->addSoundSourceFromFile((assets_path / "click.mp3").string().c_str());
    click_source->setDefaultVolume(.6f);
    slider_source = engine->addSoundSourceFromFile((assets_path / "slider.mp3").string().c_str());
    slider_source->setDefaultVolume(.4f);
#endif

    // --------------------------------------------------------------------------
    // Generate tree locations
    // --------------------------------------------------------------------------
    double t_c = glfwGetTime();
    int counter = 0;
    while (counter < num_of_trees - 1) {
        float rand_x = rand_float(-29.f, 29.f);
        float rand_y = rand_float(2.f, 14.f);
        bool is_ok = true;
        for (size_t i = 0; i < counter; i++) {
            if (glm::distance(glm::vec2(rand_x, rand_y), glm::vec2(generated_trees[i].x, generated_trees[i].y)) < minimal_gap) {
                is_ok = false;
                break;
            }
        }
        if (is_ok) {
            float rand_scale =  1.f + rand_float(0.f, 1.f);
            float rand_rotation = rand_float(0.f, 360.f);
            int rand_object = rand() % NUM_OF_TREE_OBJS;
            generated_trees.push_back({
                .x = rand_x,
                .y = rand_y,
                .scale = rand_scale,
                .rotation = rand_rotation,
                .object = rand_object
            });
            counter++;
        }
    }
    std::cout << "\n" << "Generating trees took: " << glfwGetTime() - t_c << "\n";

    // --------------------------------------------------------------------------
    // Initialize UBO Data
    // --------------------------------------------------------------------------
    camera_ubo.position = camera.get_eye_position(); //glm::vec4(camera.get_eye_position(), 1.0f);
    camera_ubo.projection = glm::perspective(glm::radians(45.0f), float(width) / float(height), 0.01f, 1000.0f);
    camera_ubo.view = glm::lookAt(camera.get_eye_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    fog_ubo.color = glm::vec4(0.2f, 0.2f, 0.2f, 1.f);
    fog_ubo.density = 0.15f;

    // General light
    lights.push_back({
        glm::vec4(1.0f, 1.0f, 1.0f, .0f), // position
        glm::vec4(.2f),                   // ambient
        glm::vec4(.2f),                   // diffuse
        glm::vec4(.2f),                   // specular

        glm::vec4(0.f),
        glm::vec4(0.f),
    });
    
    // Street lights bulbs
    for (size_t i = 0; i < num_of_street_lights; i++) {
        objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                      glm::vec3(i * 3.f - (num_of_street_lights * 3 / 2 - 1), 0.48f, 0.06f - .4f)), 
                                                                       glm::radians(-5.f), glm::vec3(1.f, 0.f, 0.f)),
                                                           glm::vec3(0.014f, 0.005f, 0.02f)),
                                .ambient_color = glm::vec4(1.0f, 1.0f, 0.1f, 1.0f),
                                .diffuse_color = glm::vec4(0.f),
                                .specular_color = glm::vec4(0.f)});
    }

    // Street cone lights
    for (size_t i = 0; i < num_of_street_lights; i++) {
        lights.push_back({
            glm::vec4(i * 3.f - (num_of_street_lights * 3 / 2 - 1), 0.48f, 0.06f - .4f, 1.f),   // position
            glm::vec4(0.f),                                                                 // ambient
            glm::vec4(1.0f, 1.0f, 0.3f, 1.f),                                               // diffuse
            glm::vec4(1.0f, 1.0f, 0.3f, 1.f),                                               // specular
            glm::vec4(0.f, -1.f, 0.3f, 1.f),                                                // direction
            glm::vec4(glm::cos(glm::radians(45.f)), glm::cos(glm::radians(75.f)), 0.f, 0.f) // cutOff
        });
    }

    // Street lamp objects
    for (size_t i = 0; i < num_of_street_lights; i++) {
        objects_ubos.push_back({.model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 3.f - (num_of_street_lights * 3 / 2 - 1), 0.f, -.4f)),
                                .ambient_color = glm::vec4(.01f),
                                .diffuse_color = glm::vec4(1.0f),
                                .specular_color = glm::vec4(1.f, 1.f, 1.f, 10.f)});
    }

    // Ground object
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(0.f, -.46f, 0.f)), 
                                                       glm::vec3(30.0f, 0.02f, 15.f)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    // Road object
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(0.f, -.45f, .45f)), 
                                                       glm::vec3(30.0f, 0.02f, 1.f)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    // Car object
    objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                  car_pos), 
                                                                   glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)), 
                                                       glm::vec3(0.6f)),
                        .ambient_color = glm::vec4(0.f),
                        .diffuse_color = glm::vec4(1.f),
                        .specular_color = glm::vec4(.0f)});

    // Front car lights
    for (size_t i = 0; i < 2; i++) {
        lights.push_back({
            glm::vec4(start_pos + .3f, -0.28f, 0.65f + 0.3f * i, 1.f),                      // position
            glm::vec4(0.5f),                                                                // ambient
            glm::vec4(1.f, 1.f, 1.f, 1.f),                                                  // diffuse
            glm::vec4(1.f, 1.f, 1.f, 1.f),                                                  // specular
            glm::vec4(1.f, 0.f, 0.f, 1.f),                                                  // direction
            glm::vec4(glm::cos(glm::radians(30.f)), glm::cos(glm::radians(45.f)), 0.f, 0.f) // cutOff
        });
    }

    for (size_t i = 0; i < 2; i++) {
        objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                      car_pos + car_lights_pos[i]), 
                                                                       glm::radians(22.f), glm::vec3(0.f, 0.f, 1.f)), 
                                                           glm::vec3(0.005f, 0.012f, 0.033f)),
                                .ambient_color = glm::vec4(1.f),
                                .diffuse_color = glm::vec4(0.f),
                                .specular_color = glm::vec4(0.f)});
    }

    // Rear car lights
    for (size_t i = 0; i < 2; i++) {
        lights.push_back({
            glm::vec4(start_pos - .3f, -0.28f, 0.65f + 0.3f * i, 1.f),                      // position
            glm::vec4(0.f),                                                                 // ambient
            glm::vec4(.1f, 0.f, 0.f, 1.f),                                                  // diffuse
            glm::vec4(.1f, 0.f, 0.f, 1.f),                                                  // specular
            glm::vec4(-1.f, 0.f, 0.f, 1.f),                                                 // direction
            glm::vec4(glm::cos(glm::radians(30.f)), glm::cos(glm::radians(60.f)), 0.f, 0.f) // cutOff
        });
    }

    for (size_t i = 2; i < 4; i++) {
        objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                          car_pos + car_lights_pos[i]),
                                                           glm::vec3(0.009f, 0.01f, 0.015f)),
                                .ambient_color = glm::vec4(1.f, 0.f, 0.f, 1.f),
                                .diffuse_color = glm::vec4(0.f),
                                .specular_color = glm::vec4(0.f)});
    }

    // Trees
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f),
                                                                      glm::vec3(1.f, .55f, -1.f)),
                                                       glm::vec3(2.f)),
                            .ambient_color = glm::vec4(0.2f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(.0f)});

    for (size_t i = 0; i < num_of_trees - num_of_static_trees; i++) {
        float tmp_y = (generated_trees[i].scale - 1.f) / 2.f; // Adjust altitude to the terrain according to scale
        float tmp_z = (generated_trees[i].object == 3) ? .25f : .0f; // Adjust alignment of one problematic .obj
        objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                      glm::vec3(generated_trees[i].x, 
                                                                                                .06f + tmp_y, 
                                                                                                generated_trees[i].y + tmp_z)),
                                                                       glm::radians(generated_trees[i].rotation), glm::vec3(0.f, 1.f, 0.f)),
                                                           glm::vec3(generated_trees[i].scale)),
                                .ambient_color = glm::vec4(0.2f),
                                .diffuse_color = glm::vec4(1.f),
                                .specular_color = glm::vec4(.0f)});
    }

    // Houses
    objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                  glm::vec3(1.7f, .24f, -3.5f)), 
                                                                   glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)),
                                                       glm::vec3(3.f)),
                            .ambient_color = glm::vec4(1.f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(.0f)});

    objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                  glm::vec3(-10.f, 0.02f, -3.5f)), 
                                                                   glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f)),
                                                       glm::vec3(3.f)),
                            .ambient_color = glm::vec4(1.f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(.0f)});

    objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                  glm::vec3(-5.f, .37f, -3.5f)), 
                                                                   glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f)),
                                                       glm::vec3(3.f)),
                            .ambient_color = glm::vec4(1.f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(.0f)});

    objects_ubos.push_back({.model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                  glm::vec3(6.7f, 0.235f, -2.f)), 
                                                                   glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)),
                                                       glm::vec3(3.f)),
                            .ambient_color = glm::vec4(1.f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(.0f)});
    
    // Pathways
    glm::vec2 tmp_scale = glm::vec2(0.3f, 1.f);
    pathway_scales.push_back(tmp_scale);
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(1.7f, -.455f, -1.3f)), 
                                                       glm::vec3(tmp_scale.x, 0.02f, tmp_scale.y)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    tmp_scale = glm::vec2(0.2f, 1.5f);
    pathway_scales.push_back(tmp_scale);
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(-10.f, -.455f, -1.3f)), 
                                                       glm::vec3(tmp_scale.x, 0.02f, tmp_scale.y)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    tmp_scale = glm::vec2(0.18f, 1.2f);
    pathway_scales.push_back(tmp_scale);
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(-5.54f, -.455f, -1.5f)), 
                                                       glm::vec3(tmp_scale.x, 0.02f, tmp_scale.y)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});
                        
    tmp_scale = glm::vec2(0.45f, 1.f);
    pathway_scales.push_back(tmp_scale);
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(7.64f, -.455f, -1.f)), 
                                                       glm::vec3(tmp_scale.x, 0.02f, tmp_scale.y)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    tmp_scale = glm::vec2(.4f, .12f);
    pathway_scales.push_back(tmp_scale);
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(6.79f, -.455f, -1.3f)), 
                                                       glm::vec3(tmp_scale.x, 0.02f, tmp_scale.y)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    tmp_scale = glm::vec2(0.12f, .45f);
    pathway_scales.push_back(tmp_scale);
    objects_ubos.push_back({.model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      glm::vec3(6.27f, -.455f, -1.63f)), 
                                                       glm::vec3(tmp_scale.x, 0.02f, tmp_scale.y)),
                            .ambient_color = glm::vec4(.3f),
                            .diffuse_color = glm::vec4(1.f),
                            .specular_color = glm::vec4(0.f)});

    // --------------------------------------------------------------------------
    // Create Buffers
    // --------------------------------------------------------------------------
    glCreateBuffers(1, &camera_buffer);
    glNamedBufferStorage(camera_buffer, sizeof(CameraUBO), &camera_ubo, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &lights_buffer);
    glNamedBufferStorage(lights_buffer, sizeof(LightUBO) * lights.size(), lights.data(), GL_DYNAMIC_STORAGE_BIT);
    
    glCreateBuffers(1, &objects_buffer);
    glNamedBufferStorage(objects_buffer, sizeof(ObjectUBO) * objects_ubos.size(), objects_ubos.data(), GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &fog_buffer);
    glNamedBufferStorage(fog_buffer, sizeof(FogUBO), &fog_ubo, GL_DYNAMIC_STORAGE_BIT);

    compile_shaders();
}

Application::~Application() {
    delete_shaders();
    glDeleteBuffers(1, &camera_buffer);
    glDeleteBuffers(1, &lights_buffer);
    glDeleteBuffers(1, &objects_buffer);
    glDeleteBuffers(1, &fog_buffer);
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------

void Application::delete_shaders() {}

void Application::compile_shaders() {
    delete_shaders();
    main_program = create_program(lecture_shaders_path / "main.vert", lecture_shaders_path / "main.frag");
    lights_program = create_program(lecture_shaders_path / "light_objects.vert", lecture_shaders_path / "light_objects.frag");
    skybox_program = create_program(lecture_shaders_path / "skybox.vert", lecture_shaders_path / "skybox.frag");
}

void Application::update(float delta) {}

void Application::render() {
    // --------------------------------------------------------------------------
    // Update others
    // --------------------------------------------------------------------------
    // Time
    float currentFrame = float(glfwGetTime());
    delta_time = currentFrame - last_frame;
    last_frame = currentFrame;
    float camera_speed = delta_time * ((plr.sprint) ? 10.f : 1.f);
    float car_speed = delta_time * 2.f * car_pause;
    car_pos.x += (car_pos.x < 25.f) ? car_speed : -50.f;

    // Blinking light
    if (currentFrame >= last_sec + .5f) {
        last_sec += .5f;
        light_on = rand() % 2 == 0;
        objects_ubos[blinking_light].ambient_color = (light_on) ? glm::vec4(1.0f, 1.0f, 0.1f, 1.0f) : glm::vec4(0.05f);
    }
    glNamedBufferSubData(objects_buffer, blinking_light * 256, sizeof(ObjectUBO), &objects_ubos[blinking_light]);

    // General light
    glm::vec4 tmp_light = (night_vision) ? glm::vec4(1.f) : glm::vec4(.1f);
    lights[0].ambient_color = tmp_light;
    lights[0].diffuse_color = tmp_light;
    lights[0].specular_color = tmp_light;
    glNamedBufferSubData(lights_buffer, 0, sizeof(LightUBO), &lights[0]);

    // --------------------------------------------------------------------------
    // Update sound
    // --------------------------------------------------------------------------
#ifdef SOUND
    car_sound->setIsPaused();
    // Listener position
    irrklang::vec3df position(camera_ubo.position.x, camera_ubo.position.y, camera_ubo.position.z);
    
    glm::vec3 tmp;
    if (p_view) {
        tmp = glm::vec3(plr.cam.camera_front);
    } else {
        tmp = glm::vec3(0.f);
    }
    irrklang::vec3df look_direction(tmp.x, tmp.y, tmp.z);
    irrklang::vec3df velPerSecond(0, 0, 0);
    irrklang::vec3df upVector(0, -1, 0);
    engine->setListenerPosition(position, look_direction, velPerSecond, upVector);

    // Sound position
    irrklang::vec3df new_pos(car_pos.x, car_pos.y, car_pos.z);
    car_sound->setPosition(new_pos);

    // Sound volume
    float dist = float(position.getDistanceFrom(new_pos));
    car_sound->setVolume((dist > 6.f) ? 0.f : (5.f - dist) * .2f);
    car_sound->setIsPaused(false);
#endif

    // --------------------------------------------------------------------------
    // Update UBOs
    // --------------------------------------------------------------------------
    // Camera
    if (p_view) {
        glm::vec3 *camera_front = &(plr.cam.camera_front);
        glm::vec3 *camera_pos = &(plr.cam.camera_pos);
        glm::vec3 camera_dir = glm::vec3(camera_front->x , camera_front->y, camera_front->z);

        if (plr.wasd.w) {
            *camera_pos += glm::normalize(camera_dir) * camera_speed;
        }
        if (plr.wasd.a) {
            *camera_pos -= glm::normalize(glm::cross(camera_dir, plr.cam.camera_up)) * camera_speed;
        }
        if (plr.wasd.s) {
            *camera_pos -= glm::normalize(camera_dir) * camera_speed;
        }
        if (plr.wasd.d) {
            *camera_pos += glm::normalize(glm::cross(camera_dir, plr.cam.camera_up)) * camera_speed;
        }

        camera_ubo.position = *camera_pos;
        camera_ubo.projection = glm::perspective(glm::radians(65.0f), static_cast<float>(width) / static_cast<float>(height), 0.01f, 1000.0f);
        camera_ubo.view = glm::lookAt(*camera_pos, *camera_pos + *camera_front, plr.cam.camera_up);
    } else {
        camera_ubo.position = camera.get_eye_position();
        camera_ubo.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.01f, 1000.0f);
        camera_ubo.view = glm::lookAt(camera.get_eye_position(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    glNamedBufferSubData(camera_buffer, 0, sizeof(CameraUBO), &camera_ubo);

    // Car
    objects_ubos[car].model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                           car_pos), 
                                                            glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)), 
                                                glm::vec3(0.6f));
    glNamedBufferSubData(objects_buffer, car * 256, sizeof(ObjectUBO), &objects_ubos[car]);

    // Car lights
    LightUBO *car_l_p = &lights[car_lights]; // car light pointer for iterating through lights
    for (int i = 0; i < 4; i++) {
        (car_l_p + i)->position.x = car_pos.x + ((i < 2) ? .3f : -.3f);
    }
    glNamedBufferSubData(lights_buffer, car_lights * sizeof(LightUBO), 4 * sizeof(LightUBO), car_l_p);

    // Car light objects
    ObjectUBO *car_light_obj = &objects_ubos[car_lights_s]; // car light object pointer for iterating through light objects
    for (int i = 0; i < 2; i++) { // front
        (car_light_obj + i)->model_matrix = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), 
                                                                                  car_pos + car_lights_pos[i]), 
                                                                   glm::radians(22.f), glm::vec3(0.f, 0.f, 1.f)), 
                                                       glm::vec3(0.005f, 0.012f, 0.033f));
    }
    for (int i = 2; i < 4; i++) { // rear
        (car_light_obj + i)->model_matrix = glm::scale(glm::translate(glm::mat4(1.0f), 
                                                                      car_pos + car_lights_pos[i]),  
                                                       glm::vec3(0.009f, 0.01f, 0.015f));
    }
    glNamedBufferSubData(objects_buffer, car_lights_s * sizeof(ObjectUBO), 4 * sizeof(ObjectUBO), car_light_obj);

    // --------------------------------------------------------------------------
    // Draw scene
    // --------------------------------------------------------------------------
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Configure fixed function pipeline
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    if (show_cursor) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // --------------------------------------------------------------------------
    // Draw objects
    // --------------------------------------------------------------------------
    glUseProgram(main_program);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lights_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, fog_buffer);

    glUniform1i(glGetUniformLocation(main_program, "has_texture"), false);
    glUniform1i(glGetUniformLocation(main_program, "toon_shading_on"), toon_shading);
    glUniform1i(glGetUniformLocation(main_program, "toon_levels"), toon_levels);
    glUniform1i(glGetUniformLocation(main_program, "light_on"), light_on);
    glUniform1i(glGetUniformLocation(main_program, "blinking_light"), blinking_light);
    glUniform1i(glGetUniformLocation(main_program, "fog_on"), fog);
    glUniform1i(glGetUniformLocation(lights_program, "fog_on"), fog);
    glProgramUniform1i(skybox_program, 15, night_vision);
    glProgramUniform1i(skybox_program, 3, fog);

    // Bulbs
    glUseProgram(lights_program);
    for (size_t i = yellow_s; i < yellow_s + num_of_street_lights; i++)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, i * 256, sizeof(ObjectUBO));
        geometries[SPHERE_OBJ]->draw();
    }

    // Street lights
    glUseProgram(main_program);
    for (size_t i = lamp_s; i < lamp_s + num_of_street_lights; i++)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, i * 256, sizeof(ObjectUBO));
        geometries[LAMP_OBJ]->draw();
    }
    
    // Road
    glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, road * 256, sizeof(ObjectUBO));

    glUniform1i(glGetUniformLocation(main_program, "has_texture"), true);
    glUniform2f(glGetUniformLocation(main_program, "texture_scale"), 30.f, 1.f);
    
    glBindTextureUnit(4, road_texture);
    geometries[CUBE_OBJ]->draw();

    // Ground objects
    glUniform2f(glGetUniformLocation(main_program, "texture_scale"), 30.f, 15.f);
    glBindTextureUnit(4, ground_texture);
    for (size_t i = 0; i < 2; i++)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, (ground + i) * 256, sizeof(ObjectUBO));
        geometries[CUBE_OBJ]->draw();
    }

    // Car
    glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, car * 256, sizeof(ObjectUBO));
    glUniform2f(glGetUniformLocation(main_program, "texture_scale"), 1.f, 1.f);
    glTextureParameteri(road_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(road_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindTextureUnit(4, car_texture);
    geometries[CAR_OBJ]->draw();

    // Car lights
    glUseProgram(lights_program);
    for (size_t i = car_lights_s; i < car_lights_s + 4; i++)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, i * 256, sizeof(ObjectUBO));
        geometries[CUBE_OBJ]->draw();
    }

    // Tree in front of house
    glUseProgram(main_program);
    glBindTextureUnit(4, tree_textures[3]);
    glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, trees * 256, sizeof(ObjectUBO));
    geometries[TREE_OBJ + 3]->draw();

    // Random trees
    for (size_t i = 0; i < num_of_trees - num_of_static_trees; i++)
    {
        glBindTextureUnit(4, tree_textures[generated_trees[i].object]);
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, (trees + num_of_static_trees + i) * 256, sizeof(ObjectUBO));
        geometries[TREE_OBJ + generated_trees[i].object]->draw();
    }
    
    // Houses
    for (size_t i = 0; i < NUM_OF_HOUSES; i++)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, (houses + i) * 256, sizeof(ObjectUBO));
        glBindTextureUnit(4, house_textures[i]);
        geometries[HOUSES_OBJ + i]->draw();
    }

    // Pathways
    for (size_t i = 0; i < num_of_pathways; i++)
    {
        glUniform2f(glGetUniformLocation(main_program, "texture_scale"), pathway_scales[i].x * 10, pathway_scales[i].y * 10);
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, objects_buffer, (pathways + i) * 256, sizeof(ObjectUBO));
        glBindTextureUnit(4, pathway_texture);
        geometries[CUBE_OBJ]->draw();
    }

    // Skybox
    glUseProgram(skybox_program);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);
    glActiveTexture(GL_TEXTURE0);

    glBindTextureUnit(4, skybox);
    geometries[CUBE_OBJ]->draw();

    glDepthFunc(GL_LESS);
}

void Application::render_ui() { 
    const float unit = ImGui::GetFontSize();
    if (ui && !controls && !settings) {
        ImGui::Begin("Pause", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::SetWindowSize(ImVec2(16 * unit, 10 * unit));
        ImGui::SetWindowPos(ImVec2(width / 2 - 8.f * unit, height / 3.f));

        ImGui::Text("            ");
        ImGui::SameLine();
        ImGui::Text("PAUSE");
    
        ImGui::Separator();
        ImGui::Text("");

        ImGui::Text("            ");
        ImGui::SameLine();
        if (ImGui::Button("Back")) {
            engine->play2D(click_source);
            ui = false;
            show_cursor = false;
        }

        ImGui::Text("          ");
        ImGui::SameLine();
        if (ImGui::Button("Controls")) {
            engine->play2D(click_source);
            controls = true;
        }

        ImGui::Text("          ");
        ImGui::SameLine();
        if (ImGui::Button("Settings")) {
            engine->play2D(click_source);
            settings = true;
        }

        ImGui::Text("            ");
        ImGui::SameLine();
        if (ImGui::Button("Exit")) {
            engine->play2D(click_source);
            _sleep(500);
            exit(0);
        }

        ImGui::End();
    } else if (controls) {
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::SetWindowSize(ImVec2(16 * unit, 15 * unit));
        ImGui::SetWindowPos(ImVec2(width / 2 - 8.f * unit, height / 3.f));

        ImGui::Text("          ");
        ImGui::SameLine();
        ImGui::Text("CONTROLS");

        ImGui::Separator();
        ImGui::Text("");

        ImGui::Text("           Movement: WASD");
        ImGui::Text("      Toggle sprint: Left shift");
        ImGui::Text("          Pause car: Space");
        ImGui::Text("      Switch camera: P");
        ImGui::Text("       Night vision: N");
        ImGui::Text("Toggle toon shading: T");
        ImGui::Text("         Toggle fog: F");
        ImGui::Text("");

        ImGui::Text("            ");
        ImGui::SameLine();
        if (ImGui::Button("Back")) {
            engine->play2D(click_source);
            controls = false;
        }

        ImGui::End();
    } else if (settings) {
        ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::SetWindowSize(ImVec2(16 * unit, 10 * unit));
        ImGui::SetWindowPos(ImVec2(width / 2 - 8.f * unit, height / 3.f));

        ImGui::Text("          ");
        ImGui::SameLine();
        ImGui::Text("SETTINGS");

        ImGui::Separator();
        ImGui::Text("");

        if (ImGui::SliderInt("Trees", &num_of_trees, 1, 1500, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            engine->play2D(slider_source);
        }
        if (ImGui::SliderInt("Toon levels", &toon_levels, 4, 20, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            engine->play2D(slider_source);
        }

        ImGui::Text("");
        ImGui::Text("            ");
        ImGui::SameLine();
        if (ImGui::Button("Back")) {
            engine->play2D(click_source);
            settings = false;
        }

        ImGui::End();
    }
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------

void Application::on_resize(int width, int height) {
    // Calls the default implementation to set the class variables.
    PV112Application::on_resize(width, height);
}

void Application::on_mouse_move(double x, double y) {
    if (p_view && !show_cursor) {
        double *yaw = &plr.cam.yaw;
        double *pitch = &plr.cam.pitch;
        if (first_mouse)
        {
            plr.lastX = x;
            plr.lastY = y;
            first_mouse = false;
        }
        double xoffset = x - plr.lastX;
        double yoffset = plr.lastY - y; 
        plr.lastX = x;
        plr.lastY = y;

        double sensitivity = 0.2f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        *yaw   += xoffset;
        *pitch += yoffset;

        if(*pitch > 89.0f)
            *pitch = 89.0f;
        if(*pitch < -89.0f)
            *pitch = -89.0f;

        glm::vec3 direction = glm::vec3(
            cos(glm::radians(*yaw)) * cos(glm::radians(*pitch)),
            sin(glm::radians(*pitch)),
            sin(glm::radians(*yaw)) * cos(glm::radians(*pitch))
        );
        plr.cam.camera_front = glm::normalize(direction);
    } else {
        camera.on_mouse_move(x, y);
    }
}
void Application::on_mouse_button(int button, int action, int mods) {
    if (p_view) {

    } else {
        camera.on_mouse_button(button, action, mods);
    }
}
void Application::on_key_pressed(int key, int scancode, int action, int mods) {
    // Calls default implementation that invokes compile_shaders when 'R key is hit.
    PV112Application::on_key_pressed(key, scancode, action, mods);
    if (key == GLFW_KEY_ESCAPE && action != GLFW_RELEASE) {
        show_cursor = !show_cursor;
        ui = !ui;
        controls = false;
    }
    if (key == GLFW_KEY_LEFT_SHIFT && action != GLFW_RELEASE) {
        plr.sprint = !plr.sprint;
    }
    if (key == GLFW_KEY_F && action != GLFW_RELEASE) {
        fog = !fog;
    }
    if (key == GLFW_KEY_N && action != GLFW_RELEASE) {
        night_vision = !night_vision;
    }
    if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE) {
        car_pause = (car_pause == 1) ? 0 : 1;
    }
    if (key == GLFW_KEY_P && action != GLFW_RELEASE) {
        p_view = !p_view;
    }
    if (key == GLFW_KEY_T && action != GLFW_RELEASE) {
        toon_shading = !toon_shading;
    }
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            plr.wasd.w = true;
        } else {
            plr.wasd.w = false;
        }
    }
    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            plr.wasd.a = true;
        } else {
            plr.wasd.a = false;
        }
    }
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            plr.wasd.s = true;
        } else {
            plr.wasd.s = false;
        }
    }
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            plr.wasd.d = true;
        } else {
            plr.wasd.d = false;
        }
    }
}

// ----------------------------------------------------------------------------
// Help Functions
// ----------------------------------------------------------------------------

float Application::rand_float(float low, float high) {
    return low + static_cast<float>(rand()) / ( static_cast<float>(RAND_MAX/(high - low)));
}

GLuint Application::load_cubemap(std::vector<std::string> faces)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    for (size_t i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load((images_path / "skybox" / faces[i]).generic_string().data(), &width, &height, &nrChannels, 0);
            
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
} 
