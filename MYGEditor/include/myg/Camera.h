#pragma once

namespace myg {

/**
 * Camera class for viewport navigation and coordinate transformation
 */
class Camera {
public:
    Camera();
    ~Camera() = default;

    /**
     * Set camera position in world coordinates
     * @param x World X position
     * @param y World Y position
     */
    void SetPosition(float x, float y);

    /**
     * Get camera X position
     */
    float GetX() const { return x_; }

    /**
     * Get camera Y position
     */
    float GetY() const { return y_; }

    /**
     * Set zoom level (pixels per tile)
     * @param zoom Zoom level (clamped between 8 and 128)
     */
    void SetZoom(float zoom);

    /**
     * Get current zoom level
     */
    float GetZoom() const { return zoom_; }

    /**
     * Set viewport dimensions
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     */
    void SetViewportSize(int width, int height);

    /**
     * Get viewport width
     */
    int GetViewportWidth() const { return viewport_width_; }

    /**
     * Get viewport height
     */
    int GetViewportHeight() const { return viewport_height_; }

    /**
     * Convert screen coordinates to world tile coordinates
     * @param screen_x Screen X coordinate
     * @param screen_y Screen Y coordinate
     * @param world_x Output world X coordinate
     * @param world_y Output world Y coordinate
     */
    void ScreenToWorld(int screen_x, int screen_y, int& world_x, int& world_y) const;

    /**
     * Convert world tile coordinates to screen coordinates
     * @param world_x World X coordinate
     * @param world_y World Y coordinate
     * @param screen_x Output screen X coordinate
     * @param screen_y Output screen Y coordinate
     */
    void WorldToScreen(int world_x, int world_y, int& screen_x, int& screen_y) const;

    /**
     * Get the visible tile bounds for frustum culling
     * @param min_x Output minimum visible X coordinate
     * @param min_y Output minimum visible Y coordinate
     * @param max_x Output maximum visible X coordinate
     * @param max_y Output maximum visible Y coordinate
     */
    void GetVisibleBounds(int& min_x, int& min_y, int& max_x, int& max_y) const;

    /**
     * Pan the camera by a delta amount
     * @param dx Delta X in screen pixels
     * @param dy Delta Y in screen pixels
     */
    void Pan(float dx, float dy);

    /**
     * Zoom the camera centered on a screen position
     * @param delta Zoom delta (positive = zoom in, negative = zoom out)
     * @param screen_x Screen X position to zoom towards
     * @param screen_y Screen Y position to zoom towards
     */
    void ZoomAt(float delta, int screen_x, int screen_y);

private:
    float x_;               // Camera position in world coordinates
    float y_;
    float zoom_;            // Pixels per tile (clamped 8-128)
    int viewport_width_;    // Viewport dimensions in pixels
    int viewport_height_;

    static constexpr float MIN_ZOOM = 8.0f;
    static constexpr float MAX_ZOOM = 128.0f;

    /**
     * Clamp zoom to valid range
     */
    void ClampZoom();
};

} // namespace myg
