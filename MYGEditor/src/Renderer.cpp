#include "myg/Renderer.h"
#include "myg/Camera.h"
#include "myg/Map.h"
#include "myg/TileInstance.h"
#include "myg/DMICache.h"
#include "myg/DMI.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <cstring>

// OpenGL 3.3 function pointers (loaded via SDL)
static PFNGLCREATESHADERPROC glCreateShader = nullptr;
static PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
static PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
static PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
static PFNGLDELETESHADERPROC glDeleteShader = nullptr;
static PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
static PFNGLATTACHSHADERPROC glAttachShader = nullptr;
static PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
static PFNGLUNIFORM1IPROC glUniform1i = nullptr;
static PFNGLUNIFORM1FPROC glUniform1f = nullptr;
static PFNGLUNIFORM4FPROC glUniform4f = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
static PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
static PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
static PFNGLBUFFERDATAPROC glBufferData = nullptr;
static PFNGLBUFFERSUBDATAPROC glBufferSubData = nullptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = nullptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
static PFNGLACTIVETEXTUREPROC glActiveTextureFunc = nullptr;

static bool LoadGLFunctions() {
    glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
    glUniform4f = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glActiveTextureFunc = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture");

    return glCreateShader && glShaderSource && glCompileShader && glUseProgram && glActiveTextureFunc;
}

namespace myg {

// Vertex shader source
static const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoord;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment shader source
static const char* fragment_shader_source = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;
uniform vec4 colorMod;
uniform float alphaMod;

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    FragColor = texColor * colorMod * vec4(1.0, 1.0, 1.0, alphaMod);
}
)";

Renderer::Renderer()
    : shader_program_(0)
    , vao_(0)
    , vbo_(0)
    , ebo_(0)
    , viewport_width_(800)
    , viewport_height_(600)
    , uniform_projection_(-1)
    , uniform_model_(-1)
    , uniform_texture_(-1)
    , uniform_color_mod_(-1)
    , uniform_alpha_mod_(-1)
{
}

Renderer::~Renderer() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (shader_program_) glDeleteProgram(shader_program_);
}

bool Renderer::Initialize() {
    // Load OpenGL functions
    if (!LoadGLFunctions()) {
        std::cerr << "Failed to load OpenGL functions" << std::endl;
        return false;
    }

    // Compile shaders
    if (!CompileShaders()) {
        std::cerr << "Failed to compile shaders" << std::endl;
        return false;
    }

    // Set up buffers
    SetupBuffers();

    // Get uniform locations
    uniform_projection_ = glGetUniformLocation(shader_program_, "projection");
    uniform_model_ = glGetUniformLocation(shader_program_, "model");
    uniform_texture_ = glGetUniformLocation(shader_program_, "texture1");
    uniform_color_mod_ = glGetUniformLocation(shader_program_, "colorMod");
    uniform_alpha_mod_ = glGetUniformLocation(shader_program_, "alphaMod");

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Update projection matrix
    UpdateProjection();

    return true;
}

void Renderer::SetViewport(int width, int height) {
    viewport_width_ = width;
    viewport_height_ = height;
    glViewport(0, 0, width, height);
    UpdateProjection();
}

void Renderer::BeginFrame() {
    // Clear the screen
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader program
    glUseProgram(shader_program_);
    glBindVertexArray(vao_);
}

void Renderer::EndFrame() {
    // Unbind
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::RenderMap(Map* map, int z_level, const Camera& camera,
                        DMICache* dmi_cache, ::DMCompiler::DMObjectTree* object_tree) {
    if (!map) return;

    // Get visible bounds from camera
    int min_x, min_y, max_x, max_y;
    camera.GetVisibleBounds(min_x, min_y, max_x, max_y);

    // Iterate through visible tiles
    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            TileInstance* tile = map->GetTile(x, y, z_level);
            if (tile) {
                RenderTile(tile, x, y, camera, dmi_cache, object_tree);
            }
        }
    }
}

void Renderer::RenderTile(TileInstance* tile, int x, int y, const Camera& camera,
                         DMICache* dmi_cache, ::DMCompiler::DMObjectTree* object_tree) {
    if (!tile || !dmi_cache || !object_tree) return;

    // Get layer-sorted objects
    auto sorted_objects = tile->GetLayerSorted();

    // Render each object in order
    for (ObjectInstance* obj : sorted_objects) {
        if (!obj) continue;

        // Resolve icon and icon_state variables
        std::string icon = obj->GetVar("icon", object_tree);
        std::string icon_state = obj->GetVar("icon_state", object_tree);

        if (icon.empty()) continue;

        // Get DMI from cache
        DMI* dmi = dmi_cache->GetDMI(icon);
        if (!dmi) continue;

        // Get icon state
        IconState* state = nullptr;
        if (!icon_state.empty()) {
            state = dmi->GetState(icon_state);
        }
        if (!state) {
            state = dmi->GetDefaultState();
        }
        if (!state || state->substates.empty()) continue;

        // For now, just render the first substate (frame 0, dir 0)
        const IconState::IconSubstate& substate = state->substates[0];

        // Calculate source rectangle (in texture coordinates 0-1)
        float tex_width = static_cast<float>(dmi->GetSheetWidth());
        float tex_height = static_cast<float>(dmi->GetSheetHeight());
        float icon_width = static_cast<float>(dmi->GetIconWidth());
        float icon_height = static_cast<float>(dmi->GetIconHeight());

        Rect src;
        src.x = substate.x / tex_width;
        src.y = substate.y / tex_height;
        src.w = icon_width / tex_width;
        src.h = icon_height / tex_height;

        // Calculate destination rectangle (in screen coordinates)
        int screen_x, screen_y;
        camera.WorldToScreen(x, y, screen_x, screen_y);

        float zoom = camera.GetZoom();
        Rect dst;
        dst.x = static_cast<float>(screen_x);
        dst.y = static_cast<float>(screen_y);
        dst.w = zoom;
        dst.h = zoom;

        // Render the sprite
        RenderSprite(dmi->GetTextureID(), src, dst);
    }
}

void Renderer::RenderSprite(GLuint texture, const Rect& src, const Rect& dst,
                           float alpha, const Color& color) {
    // Bind texture
    glActiveTextureFunc(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(uniform_texture_, 0);

    // Set color and alpha uniforms
    glUniform4f(uniform_color_mod_, color.r, color.g, color.b, color.a);
    glUniform1f(uniform_alpha_mod_, alpha);

    // Create model matrix for this sprite
    float model[16] = {
        dst.w, 0.0f,  0.0f, 0.0f,
        0.0f,  dst.h, 0.0f, 0.0f,
        0.0f,  0.0f,  1.0f, 0.0f,
        dst.x, dst.y, 0.0f, 1.0f
    };
    glUniformMatrix4fv(uniform_model_, 1, GL_FALSE, model);

    // Update vertex buffer with texture coordinates
    float vertices[] = {
        // positions   // texture coords
        0.0f, 1.0f,    src.x,         src.y + src.h,  // bottom-left
        1.0f, 1.0f,    src.x + src.w, src.y + src.h,  // bottom-right
        1.0f, 0.0f,    src.x + src.w, src.y,          // top-right
        0.0f, 0.0f,    src.x,         src.y           // top-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Draw the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool Renderer::CompileShaders() {
    // Compile vertex shader
    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
    if (!vertex_shader) {
        return false;
    }

    // Compile fragment shader
    GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (!fragment_shader) {
        glDeleteShader(vertex_shader);
        return false;
    }

    // Link program
    shader_program_ = LinkProgram(vertex_shader, fragment_shader);

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program_ != 0;
}

void Renderer::SetupBuffers() {
    // Create VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Create VBO
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Initial vertex data (will be updated per sprite)
    float vertices[] = {
        // positions   // texture coords
        0.0f, 1.0f,    0.0f, 1.0f,  // bottom-left
        1.0f, 1.0f,    1.0f, 1.0f,  // bottom-right
        1.0f, 0.0f,    1.0f, 0.0f,  // top-right
        0.0f, 0.0f,    0.0f, 0.0f   // top-left
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Create EBO
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

GLuint Renderer::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cerr << "Shader compilation failed: " << info_log << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint Renderer::LinkProgram(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        std::cerr << "Shader program linking failed: " << info_log << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

void Renderer::UpdateProjection() {
    // Create orthographic projection matrix
    // Maps screen coordinates directly (0,0 at top-left, width,height at bottom-right)
    float left = 0.0f;
    float right = static_cast<float>(viewport_width_);
    float bottom = static_cast<float>(viewport_height_);
    float top = 0.0f;
    float near_plane = -1.0f;
    float far_plane = 1.0f;

    float projection[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (far_plane - near_plane), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far_plane + near_plane) / (far_plane - near_plane), 1.0f
    };

    glUseProgram(shader_program_);
    glUniformMatrix4fv(uniform_projection_, 1, GL_FALSE, projection);
}

} // namespace myg
