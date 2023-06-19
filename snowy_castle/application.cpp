#include "application.hpp"
#include "utils.hpp"
#include <map>

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : PV227Application(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_materials();
    prepare_textures();
    prepare_lights();
    prepare_scene();
    prepare_framebuffers();
}

Application::~Application() {}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    default_unlit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "unlit.frag");
    default_lit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "lit.frag");
    display_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "display_texture.frag");
    accumulate_snow_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "accumulate_snow.frag");
    broom_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "broom.frag");

    snowflakes_program = ShaderProgram();
    snowflakes_program.add_vertex_shader(lecture_shaders_path / "particles/snow.vert");
    snowflakes_program.add_geometry_shader(lecture_shaders_path / "particles/snow.geom");
    snowflakes_program.add_fragment_shader(lecture_shaders_path / "particles/snow.frag");
    snowflakes_program.link();

    snow_program = ShaderProgram();
    snow_program.add_vertex_shader(lecture_shaders_path / "object.vert");
    snow_program.add_tess_control_shader(lecture_shaders_path / "teselation/snow.tesc");
    snow_program.add_tess_evaluation_shader(lecture_shaders_path / "teselation/snow.tese");
    snow_program.add_fragment_shader(lecture_shaders_path / "teselation/snow.frag");
    snow_program.link();

    blur_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "blur.frag");

    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 50.f);

    // Sets the projection matrix for the normal and mirrored cameras.
    projection_matrix =
        glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 0.1f, 5000.0f);
    ortho_proj_matrix = glm::ortho(-13.f, 13.f, -13.f, 13.f, 0.1f, 5000.0f);
    ortho_view_matrix = glm::lookAt(glm::vec3(0.f, 50.f, 0.f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    ortho_camera_ubo.set_view(ortho_view_matrix);
    ortho_camera_ubo.set_projection(ortho_proj_matrix);
    ortho_camera_ubo.update_opengl_data();

    camera_ubo.set_projection(projection_matrix);
    camera_ubo.update_opengl_data();
}

void Application::prepare_materials() {
    glm::vec3 brown_color = glm::vec3(0.5f, 0.25f, 0.0f);
    brown_material_ubo.set_material(PhongMaterialData(brown_color * 0.1, brown_color * 0.9, true, glm::vec3(0.1), 2.0f));
    brown_material_ubo.update_opengl_data();

    glm::vec3 castle_color = glm::vec3(0.95f, 0.75f, 0.55f);
    castle_material_ubo.set_material(PhongMaterialData(castle_color * 0.1, castle_color * 0.9, true, glm::vec3(0.1), 2.0f));
    castle_material_ubo.update_opengl_data();

    snow_material_ubo.set_material(PhongMaterialData(glm::vec3(0.1f), glm::vec3(0.9f), true, glm::vec3(0.1), 2.0f));
    snow_material_ubo.update_opengl_data();
}

void Application::prepare_textures() {
    broom_tex = TextureUtils::load_texture_2d(lecture_textures_path / "wood.jpg");
    TextureUtils::set_texture_2d_parameters(broom_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    ice_albedo_tex = TextureUtils::load_texture_2d(lecture_textures_path / "ice_albedo.png");
    TextureUtils::set_texture_2d_parameters(ice_albedo_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    snow_albedo_tex = TextureUtils::load_texture_2d(lecture_textures_path / "snow_albedo.png");
    TextureUtils::set_texture_2d_parameters(snow_albedo_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    snow_normal_tex = TextureUtils::load_texture_2d(lecture_textures_path / "snow_normal.png");
    TextureUtils::set_texture_2d_parameters(snow_normal_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    snow_height_tex = TextureUtils::load_texture_2d(lecture_textures_path / "snow_height.png");
    TextureUtils::set_texture_2d_parameters(snow_height_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    snow_roughness_tex = TextureUtils::load_texture_2d(lecture_textures_path / "snow_roughness.png");
    TextureUtils::set_texture_2d_parameters(snow_roughness_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

    particle_tex = TextureUtils::load_texture_2d(lecture_textures_path / "star.png");
    TextureUtils::set_texture_2d_parameters(particle_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

void Application::prepare_lights() {
    // The rest is set in the update scene method.
    phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
}

void Application::prepare_framebuffers() {
    // Creates framebuffers for rendering into the mask.
    glCreateFramebuffers(1, &accumulated_snow_fbo[0]);
    glCreateFramebuffers(1, &accumulated_snow_fbo[1]);
    // Creates the framebuffer for orthogonal rendering.
    glCreateFramebuffers(1, &ortho_fbo);

    // Specifies a list of color buffers to be drawn into.
    glNamedFramebufferDrawBuffers(accumulated_snow_fbo[0], 1, FBOUtils::draw_buffers_constants);
    glNamedFramebufferDrawBuffers(accumulated_snow_fbo[1], 1, FBOUtils::draw_buffers_constants);
    glNamedFramebufferDrawBuffers(ortho_fbo, 1, FBOUtils::draw_buffers_constants);

    initialize_fbo_textures();

    // We call separate resize method where we create and attach the textures.
    resize_fullscreen_textures();
}

void Application::resize_fullscreen_textures() {}

void Application::initialize_fbo_textures() {
    // Removes the previously allocated textures (if any).
    glDeleteTextures(1, &accumulated_snow_tex[0]);
    glDeleteTextures(1, &accumulated_snow_tex[1]);
    glDeleteTextures(1, &ortho_tex);
    glDeleteTextures(1, &ortho_depth_tex);

    // Creates new textures for framebuffers and set their basic parameters.
    glCreateTextures(GL_TEXTURE_2D, 1, &accumulated_snow_tex[0]);
    glCreateTextures(GL_TEXTURE_2D, 1, &accumulated_snow_tex[1]);
    glCreateTextures(GL_TEXTURE_2D, 1, &ortho_tex);
    glCreateTextures(GL_TEXTURE_2D, 1, &ortho_depth_tex);

    // Initializes the immutable storage.
    glTextureStorage2D(accumulated_snow_tex[0], 1, GL_R16F, 1024, 1024);
    glTextureStorage2D(accumulated_snow_tex[1], 1, GL_R16F, 1024, 1024);
    glTextureStorage2D(ortho_tex, 1, GL_RGBA8, 1024, 1024);
    glTextureStorage2D(ortho_depth_tex, 1, GL_DEPTH_COMPONENT24, 1024, 1024);


    // Sets the texture parameters.
    TextureUtils::set_texture_2d_parameters(accumulated_snow_tex[0], GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(accumulated_snow_tex[1], GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(ortho_tex, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    TextureUtils::set_texture_2d_parameters(ortho_depth_tex, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

    // Binds textures to framebuffers.
    glNamedFramebufferTexture(accumulated_snow_fbo[0], GL_COLOR_ATTACHMENT0, accumulated_snow_tex[0], 0);
    glNamedFramebufferTexture(accumulated_snow_fbo[1], GL_COLOR_ATTACHMENT0, accumulated_snow_tex[1], 0);
    glNamedFramebufferTexture(ortho_fbo, GL_COLOR_ATTACHMENT0, ortho_tex, 0);
    glNamedFramebufferTexture(ortho_fbo, GL_DEPTH_ATTACHMENT, ortho_depth_tex, 0);

    // Checks status of framebuffers.
    FBOUtils::check_framebuffer_status(accumulated_snow_fbo[0], "Snow buffer #1");
    FBOUtils::check_framebuffer_status(accumulated_snow_fbo[1], "Snow buffer #2");
    FBOUtils::check_framebuffer_status(ortho_fbo, "Ortho buffer");
}

void Application::prepare_scene() {

    Geometry plane = Geometry::from_file(lecture_folder_path / "models/plane.obj");
    snow_terrain_object = SceneObject(
        plane, ModelUBO(scale(translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::mat4(1.0f), glm::vec3(26.f, 1.f, 26.f))),
        snow_material_ubo, snow_albedo_tex);

    outer_terrain_object = SceneObject(
        cube, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.08f, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(13.0f, 0.1f, 13.0f))),
        brown_material_ubo);

    castel_base = SceneObject(
        cube, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.05f, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(3.8f, 0.1f, 3.8f))),
        brown_material_ubo);

    lake_object = SceneObject(
        cube, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.05f, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(12.f, 0.1f, 12.f))),
        blue_material_ubo, ice_albedo_tex);

    Geometry castle = Geometry::from_file(lecture_folder_path / "models/castle.obj");
    castle_object = SceneObject(
        castle, ModelUBO(scale(translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.06f, 0.0f)) * glm::mat4(1.0f), glm::vec3(7.f, 7.f, 7.f))),
        castle_material_ubo);

    // Prepares the broom model - rest is set in the update method.
    Geometry broom = Geometry::from_file(lecture_folder_path / "models/broom.obj");
    broom_object = SceneObject(broom, ModelUBO(), white_material_ubo, broom_tex);

    // The light model is placed based on the value from UI so we update its model matrix later.
    // We are also not adding it to the scene_objects since we render it separately.
    light_object = SceneObject(sphere, ModelUBO(), white_material_ubo);
}

void Application::prepare_snow() {
    // Initialize snowflakes positions.
    std::vector<glm::vec3> snow_initial_positions(current_snow_count);
    for (int i = 0; i < current_snow_count; i++)
        snow_initial_positions[i] = glm::vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 26,
                                              static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 50.0f,
                                              (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 26);

    glCreateBuffers(1, &snow_positions_bo);
    glNamedBufferStorage(snow_positions_bo, sizeof(float) * 3 * current_snow_count, snow_initial_positions.data(), 0);

    glCreateVertexArrays(1, &snow_vao);
    glVertexArrayVertexBuffer(snow_vao, Geometry_Base::DEFAULT_POSITION_LOC, snow_positions_bo, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(snow_vao, Geometry_Base::DEFAULT_POSITION_LOC);
    glVertexArrayAttribFormat(snow_vao, Geometry_Base::DEFAULT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(snow_vao, Geometry_Base::DEFAULT_POSITION_LOC, Geometry_Base::DEFAULT_POSITION_LOC);
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    PV227Application::update(delta);

    // Updates the main camera.
    const glm::vec3 eye_position = camera.get_eye_position();
    camera_ubo.set_view(glm::lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();

    // Computes the light position.
    glm::vec3 light_position =
        glm::vec3(15, 15, 15) * glm::vec3(cosf(gui_light_position / 6.0f) * sinf(gui_light_position), sinf(gui_light_position / 6.0f),
                                          cosf(gui_light_position / 6.0f) * cosf(gui_light_position));

    // Updates the light model visible in the scene.
    light_object.get_model_ubo().set_matrix(glm::translate(glm::mat4(1.0f), light_position) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
    light_object.get_model_ubo().update_opengl_data();

    // Updates the OpenGL buffer storing the information about the light.
    phong_lights_ubo.clear();
    phong_lights_ubo.add(PhongLightData::CreatePointLight(light_position, glm::vec3(0.1f), glm::vec3(0.9f), glm::vec3(1.0f)));
    phong_lights_ubo.update_opengl_data();

    // Recreates particles if the number changed.
    if (current_snow_count != desired_snow_count) {
        current_snow_count = desired_snow_count;
        prepare_snow();
    }

    snow_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)) 
                    * ortho_proj_matrix * ortho_view_matrix;

    render_ortho();
}

void Application::update_broom_location() {
    // Updates the broom location.
    float winZ = 0;
    glReadPixels(broom_center.x, broom_center.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    glm::vec4 clip_space =
        glm::vec4((broom_center.x / width) * 2.0f - 1.0f, (broom_center.y / height) * 2.0f - 1.0f, winZ * 2.0 - 1.0, 1.0f);
    glm::vec4 world_space = glm::inverse(camera.get_view_matrix()) * glm::inverse(projection_matrix) * clip_space;
    glm::vec3 position = glm::vec3(world_space.x / world_space.w, world_space.y / world_space.w + 4.0f, world_space.z / world_space.w);

    broom_object.get_model_ubo().set_matrix(glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(8.f)));
    broom_object.get_model_ubo().update_opengl_data();
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    // Binds the lights for everything.
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    accumulate_snow();
    use_broom();

    if (what_to_display == DISPLAY_FINAL_IMAGE) {
        render_final();
    }
    else if (what_to_display == DISPLAY_SNOW_1) {
        show_texture(accumulated_snow_tex[0]);
    }
    else if (what_to_display == DISPLAY_SNOW_2) {
        show_texture(accumulated_snow_tex[1]);
    }
    else if (what_to_display == DISPLAY_ORTHO) {
        show_texture(ortho_tex);
    }

    // Stops measuring the elapsed time.
    glEndQuery(GL_TIME_ELAPSED);

    // Waits for OpenGL - don't forget OpenGL is asynchronous.
    glFinish();

    // Evaluates the query.
    GLuint64 render_time;
    glGetQueryObjectui64v(render_time_query, GL_QUERY_RESULT, &render_time);
    fps_gpu = 1000.f / (static_cast<float>(render_time) * 1e-6f);
}

void Application::render_final() {
    // Binds and clears the main frame buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    default_lit_program.use();
    glBindTextureUnit(1, ortho_depth_tex);
    glBindTextureUnit(2, accumulated_snow_tex[0]);
    default_lit_program.uniform_matrix("snow_matrix", snow_matrix);
    default_lit_program.uniform("use_snow", show_snow);

    render_object(outer_terrain_object, default_lit_program, false);
    render_object(castel_base, default_lit_program, false);
    render_object(lake_object, default_lit_program, false, 10);
    render_object(castle_object, default_lit_program, false);

    if (show_tessellated_snow) {
        render_tese_snow();
    }

    update_broom_location();

    default_lit_program.uniform("use_snow", false);
    render_object(broom_object, default_lit_program, false);
    render_object(light_object, default_unlit_program, false);

    render_snow();

    // Resets the VAO and the program.
    glBindVertexArray(0);
    glUseProgram(0);
}

void Application::render_ortho() {
    ortho_camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    glBindFramebuffer(GL_FRAMEBUFFER, ortho_fbo);
    glViewport(0, 0, 1024, 1024);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    render_object(outer_terrain_object, default_unlit_program, false);
    render_object(lake_object, default_unlit_program, false, 10);
    render_object(castel_base, default_unlit_program, false);
    render_object(castle_object, default_unlit_program, false);

    // Resets the VAO and the program.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Application::show_texture(GLuint texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    display_texture_program.use();
    glBindTextureUnit(0, texture);

    glBindVertexArray(empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Resets the VAO and the program.
    glBindVertexArray(0);
    glUseProgram(0);
}

void Application::render_snow() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    snowflakes_program.use();
    snowflakes_program.uniform("elapsed_time_s", static_cast<float>(elapsed_time) * 1e-3f);
    glBindTextureUnit(0, particle_tex);
    glBindVertexArray(snow_vao);
    glDrawArrays(GL_POINTS, 0, current_snow_count);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void Application::render_tese_snow() {
    snow_program.use();
    snow_program.uniform_matrix("snow_matrix", snow_matrix);

    glBindTextureUnit(1, ortho_depth_tex);
    glBindTextureUnit(2, accumulated_snow_tex[1]);
    glBindTextureUnit(3, snow_height_tex);
    glBindTextureUnit(4, snow_normal_tex);

    render_object(snow_terrain_object, snow_program, true);
}

void Application::render_object(const SceneObject& object, const ShaderProgram& program, bool render_as_patches,
                                float uv_multiplier) const {
    program.use();

    // Handles the textures.
    program.uniform("has_texture", object.has_texture());
    program.uniform("uv_multiplier", uv_multiplier);
    if (object.has_texture()) {
        glBindTextureUnit(0, object.get_texture());
    } else {
        glBindTextureUnit(0, 0);
    }

    object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
    object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
    object.get_geometry().bind_vao();

    if (render_as_patches) {
        // We must use patches if we want to use tessellation.
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        if (object.get_geometry().draw_elements_count > 0) {
            glDrawElements(GL_PATCHES, object.get_geometry().draw_elements_count, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_PATCHES, 0, object.get_geometry().draw_arrays_count);
        }
    } else {
        // Calls the standard rendering function if we do not require patches.
        object.get_geometry().draw();
    }
}

void Application::clear_accumulated_snow() {
    float clear_color = 0.0;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, accumulated_snow_fbo[0]);
    glClearBufferfv(GL_COLOR, 0, &clear_color);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, accumulated_snow_fbo[1]);
    glClearBufferfv(GL_COLOR, 0, &clear_color);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::accumulate_snow() {
    int accumulation_speed_exp = static_cast<int>(log2(current_snow_count) - 7);
    double acc_speed = 100.0 / accumulation_speed_exp;
    if (last_frame + acc_speed > elapsed_time) return;
    last_frame += acc_speed;

    accumulate_snow_program.use();
    glViewport(0, 0, 1024, 1024);
    glBindVertexArray(empty_vao);

    glBindFramebuffer(GL_FRAMEBUFFER, accumulated_snow_fbo[0]);
    glBindTextureUnit(0, accumulated_snow_tex[1]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, accumulated_snow_fbo[1]);
    glBindTextureUnit(0, accumulated_snow_tex[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    apply_blur();
}

void Application::use_broom() {
    ortho_camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    glBindFramebuffer(GL_FRAMEBUFFER, accumulated_snow_fbo[1]);
    glViewport(0, 0, 1024, 1024);
    glDisable(GL_DEPTH_TEST);

    render_object(broom_object, broom_program, false);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
}

void Application::apply_blur() {
    blur_program.use();
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(empty_vao);

    glBindFramebuffer(GL_FRAMEBUFFER, accumulated_snow_fbo[1]);
    glBindTextureUnit(0, accumulated_snow_tex[0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(20 * unit, 17 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    ImGui::PushItemWidth(150.f);

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::Combo("Display", &what_to_display, DISPLAY_LABELS, IM_ARRAYSIZE(DISPLAY_LABELS));

    ImGui::SliderAngle("Light Position", &gui_light_position, 0);

    if(ImGui::Button("Clear Snow")) {
        clear_accumulated_snow();
    }

    ImGui::Checkbox("Show Snow", &show_snow);

    ImGui::Checkbox("Show Tessellated Snow", &show_tessellated_snow);

    ImGui::Checkbox("Wireframe", &wireframe);

    ImGui::Checkbox("Use PBR", &pbr);

    const char* particle_labels[10] = {"256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536", "131072"};
    int exponent = static_cast<int>(log2(current_snow_count) - 8); // -8 because we start at 256 = 2^8
    if (ImGui::Combo("Particle Count", &exponent, particle_labels, IM_ARRAYSIZE(particle_labels))) {
        desired_snow_count = static_cast<int>(glm::pow(2, exponent + 8)); // +8 because we start at 256 = 2^8
    }

    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height) {
    PV227Application::on_resize(width, height);
    resize_fullscreen_textures();
}

void Application::on_mouse_move(double x, double y) {
    PV227Application::on_mouse_move(x, y);
    broom_center = glm::vec2(static_cast<float>(x), static_cast<float>(height - y));
}