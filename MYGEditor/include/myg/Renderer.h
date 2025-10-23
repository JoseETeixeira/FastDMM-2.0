#pragma once

#include <SDL3/SDL_opengl.h>
#include <string>

namespace myg {

class Camera;
class Map;
class TileInstance;
class DMICache;

} // namespace myg

// Forward declaration - DMCompiler is in global namespace
namespace DMCompiler {
    class DMObjectTree;
}

namespace myg {

/**
 * Struct representing a 2D rectangle
 */
struct Rect {
    float x, y, w, h;
    
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}
};

/**
 * Struct representing an RGBA color
 */
struct Color {
    float r, g, b, a;
    
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
};

/**
 * Renderer class for OpenGL-based sprite and tile rendering
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * Initialize OpenGL resources (shaders, buffers)
     * @return true if initialization succeeded
     */
    bool Initialize();

    /**
     * Set the viewport dimensions
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     */
    void SetViewport(int width, int height);

    /**
     * Begin a new frame (clear buffers, prepare for rendering)
     */
    void BeginFrame();

    /**
     * End the current frame (flush remaining draws)
     */
    void EndFrame();

    /**
     * Render a map at the specified Z-level
     * @param map The map to render
     * @param z_level The Z-level to render
     * @param camera The camera for viewport transformation
     * @param dmi_cache The DMI cache for sprite lookup
     * @param object_tree The object tree for variable resolution
     */
    void RenderMap(Map* map, int z_level, const Camera& camera, 
                   DMICache* dmi_cache, ::DMCompiler::DMObjectTree* object_tree);

    /**
     * Render a single tile
     * @param tile The tile instance to render
     * @param x World X coordinate
     * @param y World Y coordinate
     * @param camera The camera for viewport transformation
     * @param dmi_cache The DMI cache for sprite lookup
     * @param object_tree The object tree for variable resolution
     */
    void RenderTile(TileInstance* tile, int x, int y, const Camera& camera,
                    DMICache* dmi_cache, ::DMCompiler::DMObjectTree* object_tree);

    /**
     * Render a sprite quad
     * @param texture OpenGL texture ID
     * @param src Source rectangle in texture coordinates (0-1)
     * @param dst Destination rectangle in screen coordinates
     * @param alpha Alpha multiplier
     * @param color Color multiplier
     */
    void RenderSprite(GLuint texture, const Rect& src, const Rect& dst,
                     float alpha = 1.0f, const Color& color = Color());

    /**
     * Get viewport width
     */
    int GetViewportWidth() const { return viewport_width_; }

    /**
     * Get viewport height
     */
    int GetViewportHeight() const { return viewport_height_; }

private:
    GLuint shader_program_;
    GLuint vao_, vbo_, ebo_;
    
    int viewport_width_;
    int viewport_height_;

    // Shader uniform locations
    GLint uniform_projection_;
    GLint uniform_model_;
    GLint uniform_texture_;
    GLint uniform_color_mod_;
    GLint uniform_alpha_mod_;

    /**
     * Compile vertex and fragment shaders
     * @return true if compilation succeeded
     */
    bool CompileShaders();

    /**
     * Set up vertex array object and buffers for quad rendering
     */
    void SetupBuffers();

    /**
     * Compile a shader from source
     * @param type Shader type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
     * @param source Shader source code
     * @return Shader ID, or 0 on failure
     */
    GLuint CompileShader(GLenum type, const char* source);

    /**
     * Link shader program from vertex and fragment shaders
     * @param vertex_shader Vertex shader ID
     * @param fragment_shader Fragment shader ID
     * @return Program ID, or 0 on failure
     */
    GLuint LinkProgram(GLuint vertex_shader, GLuint fragment_shader);

    /**
     * Update the projection matrix for the current viewport
     */
    void UpdateProjection();
};

} // namespace myg
