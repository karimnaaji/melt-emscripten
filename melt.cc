#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "imgui.h"
#include "imgui_font.h"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"
#define MELT_IMPLEMENTATION
#include "melt/melt.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <sstream>
#include <map>

#include "bunny_obj.h"
#include "column_obj.h"
#include "cube_obj.h"
#include "sphere_obj.h"
#include "teapot_obj.h"
#include "suzanne_obj.h"

static bool show_quit_dialog = false;

static MeltParams melt_params;
static MeltResult melt_result;

static sg_pass_action pass_action;
static sg_pipeline pipeline_0;
static sg_pipeline pipeline_0_depth;
static sg_pipeline pipeline_1;
static sg_pipeline pipeline_1_depth;
static sg_bindings bindings_0;
static sg_bindings bindings_1;
static sg_buffer position_buffer;
static sg_buffer occluder_position_buffer;
static sg_buffer occluder_index_buffer;

static glm::mat4 view;
static glm::mat4 position;

static uint32_t model_vertex_count;
static uint32_t occluder_vertex_count;

static bool depth_test_enabled = false;

typedef struct 
{
    glm::mat4 mvp;
} vs_uniform_params;

typedef struct 
{
    float alpha;
} fs_uniform_params;

typedef struct
{
    glm::vec3 scale;
    glm::vec3 translation;
    float voxel_resolution;
    float fill_percentage;
} model_config;

static std::map<const char*, model_config> model_configs =
{
    {"bunny.obj", {glm::vec3(2.0f), glm::vec3(0.0f, -0.5f, 0.0f), 0.12f, 0.9f}},
    {"column.obj", {glm::vec3(0.35f), glm::vec3(0.0f, -6.7f, 0.0f), 0.2f, 0.85f}},
    {"cube.obj", {glm::vec3(1.2f), glm::vec3(0.0f), 0.15f, 1.0f}},
    {"sphere.obj", {glm::vec3(1.5f), glm::vec3(0.0f), 0.15f, 0.8f}},
    {"suzanne.obj", {glm::vec3(1.5f), glm::vec3(0.0f), 0.15f, 0.8f}},
    //{"teapot.obj", {glm::vec3(1.0f, glm::vec3(0.0f), 0.25f, 1.0f}},
};

static const char* obj_models[] = 
{ 
    "bunny.obj", 
    "column.obj", 
    "cube.obj",
    "sphere.obj",
    "suzanne.obj",
    //"teapot.obj",
};
static int obj_model_index = 0;

static void setup_imgui_style()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowMinSize     = ImVec2(280, 5000);
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
}

static void init_melt_params()
{
    melt_params.debug.voxelScale = 0.8f;
    melt_params.voxelSize = 0.25f;
    melt_params.fillPercentage = 1.0f;
    melt_params.debug.flags |= MeltDebugTypeShowResult;
    melt_params.debug.extentIndex = -1;
    melt_params.boxTypeFlags = MeltOccluderBoxTypeRegular;
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
    
    model_vertex_count = vertex_count;

    return true;
}

static void generate_occluder()
{
    MeltGenerateOccluder(melt_params, melt_result);

    if (melt_result.debugMesh.vertices.size() == 0)
        return;

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
    
    occluder_vertex_count = melt_result.debugMesh.indices.size();
}

static void init(void) 
{
    sg_desc desc = {};
    desc.mtl_device = sapp_metal_get_device();
    desc.mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor;
    desc.mtl_drawable_cb = sapp_metal_get_drawable;
    desc.d3d11_device = sapp_d3d11_get_device();
    desc.d3d11_device_context = sapp_d3d11_get_device_context();
    desc.d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view;
    desc.d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view;
    desc.gl_force_gles2 = sapp_gles2();
    sg_setup(&desc);

    simgui_desc_t simgui_desc = {};
    simgui_desc.no_default_font = true;
    simgui_desc.dpi_scale = sapp_dpi_scale();
    simgui_setup(&simgui_desc);

    auto& io = ImGui::GetIO();
    ImFontConfig fontCfg;
    fontCfg.FontDataOwnedByAtlas = false;
    fontCfg.OversampleH = 2;
    fontCfg.OversampleV = 2;
    fontCfg.RasterizerMultiply = 1.5f;
    io.Fonts->AddFontFromMemoryTTF(dump_font, sizeof(dump_font), 13.0f, &fontCfg);

    setup_imgui_style();

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

    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].val[0] = 0.64f;
    pass_action.colors[0].val[1] = 0.76f;
    pass_action.colors[0].val[2] = 0.91f;
    pass_action.colors[0].val[3] = 1.0f;

#if defined(SOKOL_GLCORE33)
    const char vertex_source[] = 
        "#version 330\n"
        "precision highp float;\n"
        "layout(location=0) in vec3 position;\n"
        "layout(location=1) in vec3 color;\n"
        "uniform mat4 mvp;\n"
        "out vec3 f_color;\n"
        "void main(void) {\n"
        "    gl_Position = mvp * vec4(position, 1.0);\n"
        "    f_color = color;\n"
        "}\n";
    const char fragment_source[] =
        "#version 330\n"
        "precision highp float;\n"
        "in vec3 f_color;\n"
        "out vec4 color;\n"
        "uniform float alpha;\n"
        "void main(void) {\n"
        "    color = vec4(f_color, alpha);\n"
        "}\n";
#elif defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
    const char vertex_source[] = 
        "precision highp float;\n"
        "attribute vec3 position;\n"
        "attribute vec3 color;\n"
        "uniform mat4 mvp;\n"
        "varying vec3 f_color;\n"
        "void main(void) {\n"
        "    gl_Position = mvp * vec4(position, 1.0);\n"
        "    f_color = color;\n"
        "}\n";
    const char fragment_source[] =
        "precision highp float;\n"
        "varying vec3 f_color;\n"
        "uniform float alpha;\n"
        "void main(void) {\n"
        "    gl_FragColor = vec4(f_color, alpha);\n"
        "}\n";
#endif

    sg_shader_desc shader_desc =  {
        .attrs[0].name = "position",
        .attrs[1].name = "color",
        .fs.uniform_blocks[0] = {
            .size = sizeof(fs_uniform_params),
            .uniforms = { [0] = { .name = "alpha", .type = SG_UNIFORMTYPE_FLOAT } }
        },
        .fs.source = fragment_source,
        .vs.uniform_blocks[0] =  {
            .size = sizeof(vs_uniform_params),
            .uniforms =  { [0] = { .name = "mvp", .type = SG_UNIFORMTYPE_MAT4 } }
        },
        .vs.source = vertex_source,
    };
    sg_shader program = sg_make_shader(&shader_desc);
    
    sg_pipeline_desc pip_desc = {
        .layout =  {
            .buffers[0].stride = sizeof(glm::vec3) * 2,
            .attrs = {
                [0].offset = 0,
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].offset = sizeof(glm::vec3),
                [1].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .shader = program,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_ALWAYS,
            .depth_write_enabled = false
        },
        .blend =  {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        },
    };
    pipeline_0 = sg_make_pipeline(&pip_desc);
    pip_desc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pip_desc.blend.enabled = false;
    pip_desc.depth_stencil.depth_write_enabled = true;
    pipeline_0_depth = sg_make_pipeline(&pip_desc);
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_ALWAYS;
    pip_desc.blend.enabled = true;
    pipeline_1 = sg_make_pipeline(&pip_desc);
    pip_desc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pip_desc.blend.enabled = false;
    pip_desc.depth_stencil.depth_write_enabled = true;
    pipeline_1_depth = sg_make_pipeline(&pip_desc);

    init_melt_params();

    const char* model_name = obj_models[0];
    load_model_mesh(obj_models[0]);
    melt_params.fillPercentage = model_configs[model_name].fill_percentage;
    melt_params.voxelSize = model_configs[model_name].voxel_resolution;
    generate_occluder();
}

static void simgui_frame()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGuiWindowFlags options = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Fixed Overlay", nullptr, ImVec2(0,0), 0.3f, options);

    static bool box_type_diagonals = false;
    static bool box_type_top = false;
    static bool box_type_bottom = false;
    static bool box_type_sides = false;
    static bool box_type_regular = true;

    if (ImGui::Combo("Obj model", &obj_model_index, obj_models, IM_ARRAYSIZE(obj_models)))
    {   
        const char* model_name = obj_models[obj_model_index];
        load_model_mesh(model_name);
        melt_params.fillPercentage = model_configs[model_name].fill_percentage;
        melt_params.voxelSize = model_configs[model_name].voxel_resolution;
        generate_occluder();
    }

    ImGui::DragFloat("Voxel Size", &melt_params.voxelSize, 0.005f, 0.12f, 0.25f);
    ImGui::DragFloat("Fill Percentage", &melt_params.fillPercentage, 0.01f, 0.0f, 1.0f);

    ImGui::Checkbox("BoxTypeDiagonals", &box_type_diagonals);
    ImGui::Checkbox("BoxTypeTop", &box_type_top);
    ImGui::Checkbox("BoxTypeBottom", &box_type_bottom);
    ImGui::Checkbox("BoxTypeSides", &box_type_sides);
    ImGui::Checkbox("BoxTypeRegular", &box_type_regular);
    
    melt_params.boxTypeFlags = MeltOccluderBoxTypeNone;

    if (box_type_diagonals) melt_params.boxTypeFlags |= MeltOccluderBoxTypeDiagonals;
    if (box_type_top) melt_params.boxTypeFlags |= MeltOccluderBoxTypeTop;
    if (box_type_bottom) melt_params.boxTypeFlags |= MeltOccluderBoxTypeBottom;
    if (box_type_sides) melt_params.boxTypeFlags |= MeltOccluderBoxTypeSides;
    if (box_type_regular) melt_params.boxTypeFlags = MeltOccluderBoxTypeRegular;

    ImGui::Checkbox("Depth Test", &depth_test_enabled);

    if (ImGui::Button("Generate") && melt_params.boxTypeFlags != MeltOccluderBoxTypeNone)
    {
        generate_occluder();
    }

    ImGui::End();
}

static void frame(void) 
{
    const int width = sapp_width();
    const int height = sapp_height();

    simgui_new_frame(width, height, 1.0 / 60.0);
    simgui_frame();
    sg_begin_default_pass(&pass_action, width, height);

    if (bindings_0.vertex_buffers[0].id != SG_INVALID_ID && 
        bindings_1.vertex_buffers[0].id != SG_INVALID_ID)
    {
        const char* model_name = obj_models[obj_model_index];
        glm::mat4 projection = glm::perspective(glm::radians(55.0f), float(width) / height, 0.01f, 100.0f);
        view = glm::lookAt(glm::vec3(0.0, 1.5f, 6.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        static float ry = 0.0f;
        ry += 0.01f;
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), model_configs[model_name].scale);
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), model_configs[model_name].translation);
        glm::mat4 rym = glm::rotate(glm::mat4(1.0f), ry, glm::vec3(0.0f, 1.0f, 0.0f)) * scale * translate;

        vs_uniform_params vs_uniforms;
        vs_uniforms.mvp = projection * view * rym;
        fs_uniform_params fs_uniforms;
        fs_uniforms.alpha = 0.3f;

        if (depth_test_enabled)
            sg_apply_pipeline(pipeline_0_depth);
        else
            sg_apply_pipeline(pipeline_0);
        sg_apply_bindings(&bindings_0);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_uniforms, sizeof(vs_uniform_params));
        sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &fs_uniforms, sizeof(fs_uniform_params));

        sg_draw(0, model_vertex_count, 1);

        if (depth_test_enabled)
            sg_apply_pipeline(pipeline_1_depth);
        else
            sg_apply_pipeline(pipeline_1);
        sg_apply_bindings(&bindings_1);
        sg_draw(0, occluder_vertex_count, 1);
    }

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) 
{
    simgui_shutdown();
    sg_destroy_buffer(position_buffer);
    sg_destroy_buffer(occluder_position_buffer);
    sg_destroy_buffer(occluder_index_buffer);
    sg_shutdown();
}

static void input(const sapp_event* event) 
{
    if (event->type == SAPP_EVENTTYPE_QUIT_REQUESTED)
    {
        show_quit_dialog = true;
        sapp_cancel_quit();
    }

    simgui_handle_event(event);
}

sapp_desc sokol_main(int argc, char* argv[]) 
{
    sapp_desc desc = {};
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
