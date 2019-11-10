#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "imgui.h"
#include "imgui_font.h"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"
#define MELT_IMPLEMENTATION
#include "melt.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <sstream>

#include "bunny_obj.h"
#include "column_obj.h"
#include "cube_obj.h"
#include "sphere_obj.h"
#include "teapot_obj.h"
#include "suzanne_obj.h"

static bool show_test_window = true;
static bool show_another_window = false;
static bool show_quit_dialog = false;
static sg_pass_action pass_action;

static MeltParams melt_params;
static MeltResult melt_result;

static sg_pipeline pipeline;
static sg_bindings bindings_0;
static sg_bindings bindings_1;
static sg_buffer position_buffer;
static sg_buffer occluder_position_buffer;
static sg_buffer occluder_index_buffer;

static glm::mat4 view;
static glm::mat4 position;

typedef struct {
    glm::mat4 mvp;
} uniform_params;

static void setup_imgui_style()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowMinSize     = ImVec2(320, 5000);
    style.FramePadding      = ImVec2(6, 6);
    style.ItemSpacing       = ImVec2(6, 6);
    style.ItemInnerSpacing  = ImVec2(6, 6);
    style.Alpha             = 1.0f;
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 0.0f;
    style.IndentSpacing     = 6.0f;
    style.ItemInnerSpacing  = ImVec2(6, 6);
    style.ColumnsMinSpacing = 50.0f;
    style.GrabMinSize       = 14.0f;
    style.GrabRounding      = 0.0f;
    style.ScrollbarSize     = 12.0f;
    style.ScrollbarRounding = 0.0f;

    style.Colors[ImGuiCol_Text]                  = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.20f, 0.20f, 0.20f, 0.58f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 0.31f, 0.00f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.20f, 0.20f, 0.60f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.47f, 0.47f, 0.47f, 0.21f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.47f, 0.47f, 0.14f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.47f, 0.47f, 0.14f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.86f, 0.86f, 0.86f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
    style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
    ImGuiIO& io = ImGui::GetIO();
}

static void init_melt_params()
{
    melt_params.debug.voxelScale = 0.8f;
    melt_params.voxelSize = 0.25f;
    melt_params.fillPercentage = 1.0f;
    melt_params.debug.flags |= MeltDebugTypeShowResult;
}

static bool load_model_mesh(const char* model_name)
{
    const char* model = nullptr;

    if (strcmp(model_name, "bunny.obj") == 0)
        model = s_bunny_obj;
    else if (strcmp(model_name, "column.obj") == 0)
        model = s_column_obj;
    else if (strcmp(model_name, "cube.obj") == 0)
        model = s_cube_obj;
    else if (strcmp(model_name, "sphere.obj") == 0)
        model = s_sphere_obj;
    else if (strcmp(model_name, "suzanne.obj") == 0)
        model = s_suzanne_obj;
    else if (strcmp(model_name, "teapot.obj") == 0)
        model = s_teapot_obj;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;
    bool obj_parsing_res;
    std::stringstream obj_stream(model);
    tinyobj::MaterialFileReader material_reader("");

    obj_parsing_res = tinyobj::LoadObj(shapes, materials, error, obj_stream, material_reader);

    if (!error.empty() || !obj_parsing_res) 
    {
        return false;
    }

    uint32_t vertex_count = 0;
    for (size_t i = 0; i < shapes.size(); i++) 
        vertex_count += shapes[i].mesh.indices.size();

    int output_index = 0;
    std::vector<glm::vec3> mesh_buffer_data(vertex_count * 2);
    for (size_t i = 0; i < shapes.size(); i++) 
    {
        for (size_t f = 0; f < shapes[i].mesh.indices.size(); f++) 
        {
            auto position = &shapes[i].mesh.positions[shapes[i].mesh.indices[f] * 3];
            mesh_buffer_data[output_index++] = *reinterpret_cast<glm::vec3*>(position);
            mesh_buffer_data[output_index++] = glm::vec3(1.0f, 0.5f, 0.5f);
        }
    }
    
    sg_buffer_desc vbuf_desc;
    memset(&vbuf_desc, 0, sizeof(vbuf_desc));
    vbuf_desc.size = mesh_buffer_data.size() * sizeof(glm::vec3);
    vbuf_desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
    vbuf_desc.usage = SG_USAGE_IMMUTABLE;
    vbuf_desc.label = "position_buffer";
    vbuf_desc.content = mesh_buffer_data.data();
    if (position_buffer.id != SG_INVALID_ID)
    {
        sg_destroy_buffer(position_buffer);
    }
    position_buffer = sg_make_buffer(&vbuf_desc);

    bindings_0 = 
    {
        .vertex_buffers[0] = position_buffer,
    };

    melt_params.mesh.vertices.clear();
    melt_params.mesh.indices.clear();

    for (size_t i = 0; i < shapes.size(); i++) 
    {
        for (size_t f = 0; f < shapes[i].mesh.indices.size(); f++) 
        {
            melt_params.mesh.indices.push_back(shapes[i].mesh.indices[f]);
        }

        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) 
        {
            melt_params.mesh.vertices.emplace_back();
            melt_params.mesh.vertices.back().x = shapes[i].mesh.positions[3 * v + 0];
            melt_params.mesh.vertices.back().y = shapes[i].mesh.positions[3 * v + 1];
            melt_params.mesh.vertices.back().z = shapes[i].mesh.positions[3 * v + 2];
        }
    }

    return true;
}

static void generate_occluder()
{
    MeltGenerateOccluder(melt_params, melt_result);

    sg_buffer_desc vbuf_desc;
    memset(&vbuf_desc, 0, sizeof(vbuf_desc));
    vbuf_desc.size = melt_result.debugMesh.vertices.size() * sizeof(glm::vec3);
    vbuf_desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
    vbuf_desc.usage = SG_USAGE_IMMUTABLE;
    vbuf_desc.label = "occluder_position_buffer";
    vbuf_desc.content = melt_result.debugMesh.vertices.data();
    if (position_buffer.id != SG_INVALID_ID)
    {
        sg_destroy_buffer(occluder_position_buffer);
    }
    occluder_position_buffer = sg_make_buffer(&vbuf_desc);

    sg_buffer_desc ibuf_desc;
    memset(&ibuf_desc, 0, sizeof(ibuf_desc));
    ibuf_desc.size = melt_result.debugMesh.indices.size() * sizeof(uint16_t);
    ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ibuf_desc.usage = SG_USAGE_IMMUTABLE;
    ibuf_desc.label = "occluder_index_buffer";
    ibuf_desc.content = melt_result.debugMesh.indices.data();
    if (occluder_index_buffer.id != SG_INVALID_ID)
    {
        sg_destroy_buffer(occluder_index_buffer);
    }
    occluder_index_buffer = sg_make_buffer(&ibuf_desc);

    bindings_1 = 
    {
        .vertex_buffers[0] = occluder_position_buffer,
        .index_buffer = occluder_index_buffer,
    };
}

static void init(void) {
    // setup sokol-gfx and sokol-time
    sg_desc desc = { };
    desc.mtl_device = sapp_metal_get_device();
    desc.mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor;
    desc.mtl_drawable_cb = sapp_metal_get_drawable;
    desc.d3d11_device = sapp_d3d11_get_device();
    desc.d3d11_device_context = sapp_d3d11_get_device_context();
    desc.d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view;
    desc.d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view;
    desc.gl_force_gles2 = sapp_gles2();
    sg_setup(&desc);

    // setup sokol-imgui, but provide our own font
    simgui_desc_t simgui_desc = { };
    simgui_desc.no_default_font = true;
    simgui_desc.dpi_scale = sapp_dpi_scale();
    simgui_setup(&simgui_desc);

    // configure Dear ImGui with our own embedded font
    auto& io = ImGui::GetIO();
    ImFontConfig fontCfg;
    fontCfg.FontDataOwnedByAtlas = false;
    fontCfg.OversampleH = 2;
    fontCfg.OversampleV = 2;
    fontCfg.RasterizerMultiply = 1.5f;
    io.Fonts->AddFontFromMemoryTTF(dump_font, sizeof(dump_font), 16.0f, &fontCfg);

    setup_imgui_style();

    // create font texture for the custom font
    unsigned char* font_pixels;
    int font_width, font_height;
    io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
    sg_image_desc img_desc = { };
    img_desc.width = font_width;
    img_desc.height = font_height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.min_filter = SG_FILTER_LINEAR;
    img_desc.mag_filter = SG_FILTER_LINEAR;
    img_desc.content.subimage[0][0].ptr = font_pixels;
    img_desc.content.subimage[0][0].size = font_width * font_height * 4;
    io.Fonts->TexID = (ImTextureID)(uintptr_t) sg_make_image(&img_desc).id;

    // initial clear color
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].val[0] = 0.64f;
    pass_action.colors[0].val[1] = 0.76f;
    pass_action.colors[0].val[2] = 0.91f;
    pass_action.colors[0].val[3] = 1.0f;

    const char vertex_source[] = R"END(
        #version 150
        in vec3 position;
        in vec3 color;
        uniform mat4 mvp;
        out vec3 f_color;
        void main(void) {
            gl_Position = mvp * vec4(position, 1.0);
            f_color = color;
        }
    )END";

    const char fragment_source[] = R"END(
        #version 150
        in vec3 f_color;
        out vec4 color;
        uniform float alpha;
        void main(void) {
            color = vec4(f_color, alpha);
        }
    )END";

    sg_shader_desc shader_desc = 
    {
        .vs.uniform_blocks[0] = 
        {
            .size = sizeof(uniform_params),
            .uniforms = 
            { 
                [0] = { .name = "mvp", .type = SG_UNIFORMTYPE_MAT4 }
            }
        },
        .vs.source = vertex_source,
        .fs.source = fragment_source,
    };
    sg_shader program = sg_make_shader(&shader_desc);
    
    sg_pipeline_desc pip_desc = 
    {
        .shader = program,
        .blend = 
        {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        },
        // .index_type = SG_INDEXTYPE_UINT16,
        .layout = 
        {
            .buffers[0].stride = sizeof(glm::vec3) * 2,
            .attrs = {
                [0].offset = 0,
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].offset = sizeof(glm::vec3),
                [1].format = SG_VERTEXFORMAT_FLOAT3,
            }
        }
    };
    pipeline = sg_make_pipeline(&pip_desc);

    init_melt_params();
}

static void simgui_frame()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGuiWindowFlags options = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Fixed Overlay", nullptr, ImVec2(0,0), 0.3f, options);

    static bool depth_test = false;
    static bool box_type_diagonals = false;
    static bool box_type_top = false;
    static bool box_type_bottom = false;
    static bool box_type_sides = false;
    static bool box_type_regular = true;
    const char* obj_models[] = 
    { 
        "bunny.obj", 
        "column.obj", 
        "cube.obj",
        "sphere.obj",
        "suzanne.obj",
        "teapot.obj",
    };
    static int obj_model_index = 0;

    if (ImGui::Combo("Obj model", &obj_model_index, obj_models, IM_ARRAYSIZE(obj_models)))
    {   
        load_model_mesh(obj_models[obj_model_index]);
        generate_occluder();
    }

    ImGui::InputFloat("Voxel Size", &melt_params.voxelSize);
    ImGui::DragFloat("Fill Percentage", &melt_params.fillPercentage, 0.01f, 0.0f, 1.0f);

    ImGui::Checkbox("BoxTypeDiagonals", &box_type_diagonals);
    ImGui::Checkbox("BoxTypeTop", &box_type_top);
    ImGui::Checkbox("BoxTypeBottom", &box_type_bottom);
    ImGui::Checkbox("BoxTypeSides", &box_type_sides);
    ImGui::Checkbox("BoxTypeRegular", &box_type_regular);

    if (box_type_diagonals) melt_params.boxTypeFlags |= MeltOccluderBoxTypeDiagonals;
    if (box_type_top) melt_params.boxTypeFlags |= MeltOccluderBoxTypeTop;
    if (box_type_bottom) melt_params.boxTypeFlags |= MeltOccluderBoxTypeBottom;
    if (box_type_sides) melt_params.boxTypeFlags |= MeltOccluderBoxTypeSides;
    if (box_type_regular) melt_params.boxTypeFlags = MeltOccluderBoxTypeRegular;

    if (ImGui::Button("Generate"))
    {
        generate_occluder();
    }
    ImGui::Checkbox("Depth Test", &depth_test);

    ImGui::End();
}

static void frame(void) {
    const int width = sapp_width();
    const int height = sapp_height();

    simgui_new_frame(width, height, 1.0 / 60.0);
    simgui_frame();
    sg_begin_default_pass(&pass_action, width, height);

    if (bindings_0.vertex_buffers[0].id != SG_INVALID_ID)
    {
        sg_apply_pipeline(pipeline);
        sg_apply_bindings(&bindings_0);
        uniform_params uniforms;
        glm::mat4 projection = glm::perspective(glm::radians(55.0f), float(width) / height, 0.01f, 100.0f);
        glm::mat4 view_proj = projection * view;
        uniforms.mvp = view_proj;
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniforms, sizeof(uniform_params));
        sg_draw(0, 36, 1);
    }

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    simgui_shutdown();
    sg_destroy_buffer(position_buffer);
    sg_destroy_buffer(occluder_position_buffer);
    sg_destroy_buffer(occluder_index_buffer);
    sg_shutdown();
}

static void input(const sapp_event* event) 
{
    static glm::vec3 position(-4.0f, 0.0f, 0.0f);
    static glm::vec3 forward(1.0, 0.0, 0.0f);
    static float rotation[] = { 0.0f, 0.0f };
    static double last_mouse_pos[] = { 0.0, 0.0 };
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    double mouse[2];

    float dpi_scale = sapp_dpi_scale();
    switch (event->type) 
    {
        case SAPP_EVENTTYPE_QUIT_REQUESTED: 
        {
            show_quit_dialog = true;
            sapp_cancel_quit();
        }
        break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        {
            mouse[0] = event->mouse_x / dpi_scale;
            mouse[1] = event->mouse_y / dpi_scale;
            if (event->mouse_button < 3) 
            {
                rotation[0] += (float)(mouse[1] - last_mouse_pos[1]) * 0.005f;
                rotation[1] += (float)(mouse[0] - last_mouse_pos[0]) * 0.005f;
            }
            last_mouse_pos[0] = mouse[0];
            last_mouse_pos[1] = mouse[1];

            float pitch = -rotation[0];
            float yaw = rotation[1];

            if (pitch >= (float) M_PI_2) pitch = (float) M_PI_2;
            if (pitch <= (float)-M_PI_2) pitch = (float)-M_PI_2;

            forward.x = cos(pitch) * cos(yaw);
            forward.y = sin(pitch);
            forward.z = cos(pitch) * sin(yaw);
            forward = glm::normalize(forward);
        }
        break;
        case SAPP_EVENTTYPE_MOUSE_UP:
        {
            last_mouse_pos[0] = mouse[0] = event->mouse_x / dpi_scale;
            last_mouse_pos[1] = mouse[1] = event->mouse_y / dpi_scale;
        }
        break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
        {
            last_mouse_pos[0] = mouse[0] = event->mouse_x / dpi_scale;
            last_mouse_pos[1] = mouse[1] = event->mouse_y / dpi_scale;
        }
        break;
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            float speed = 0.01f;
            if (event->key_code == SAPP_KEYCODE_LEFT_SHIFT)
                speed = 0.1f;
            if (event->key_code == SAPP_KEYCODE_S)
                position -= forward * speed;
            else if (event->key_code == SAPP_KEYCODE_W)
                position += forward * speed;
            else if (event->key_code == SAPP_KEYCODE_A)
                position -= glm::normalize(glm::cross(forward, up)) * speed;
            else if (event->key_code == SAPP_KEYCODE_D)
                position += glm::normalize(glm::cross(forward, up)) * speed;
        }
        break;
    }
    view = glm::lookAt(position, position + forward, up);

    simgui_handle_event(event);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    sapp_desc desc = { };
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.width = 1024;
    desc.height = 768;
    desc.fullscreen = true;
    desc.high_dpi = true;
    desc.html5_ask_leave_site = false;
    desc.ios_keyboard_resizes_canvas = false;
    desc.gl_force_gles2 = true;
    desc.window_title = "Dear ImGui HighDPI (sokol-app)";
    return desc;
}
