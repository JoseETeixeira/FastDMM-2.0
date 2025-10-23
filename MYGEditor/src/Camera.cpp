#include "myg/Camera.h"
#include <algorithm>
#include <cmath>

namespace myg {

Camera::Camera()
    : x_(0.0f)
    , y_(0.0f)
    , zoom_(32.0f)  // Default 32 pixels per tile
    , viewport_width_(800)
    , viewport_height_(600)
{
}

void Camera::SetPosition(float x, float y) {
    x_ = x;
    y_ = y;
}

void Camera::SetZoom(float zoom) {
    zoom_ = zoom;
    ClampZoom();
}

void Camera::SetViewportSize(int width, int height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

void Camera::ScreenToWorld(int screen_x, int screen_y, int& world_x, int& world_y) const {
    // Convert screen coordinates to world coordinates
    // Screen (0,0) is top-left, world coordinates are tile-based
    
    // Calculate the world position at the center of the viewport
    float center_world_x = x_;
    float center_world_y = y_;
    
    // Calculate offset from center in screen pixels
    float offset_x = screen_x - (viewport_width_ / 2.0f);
    float offset_y = screen_y - (viewport_height_ / 2.0f);
    
    // Convert pixel offset to world tile offset
    float world_offset_x = offset_x / zoom_;
    float world_offset_y = offset_y / zoom_;
    
    // Calculate final world position
    float world_fx = center_world_x + world_offset_x;
    float world_fy = center_world_y + world_offset_y;
    
    // Convert to integer tile coordinates
    world_x = static_cast<int>(std::floor(world_fx));
    world_y = static_cast<int>(std::floor(world_fy));
}

void Camera::WorldToScreen(int world_x, int world_y, int& screen_x, int& screen_y) const {
    // Convert world tile coordinates to screen coordinates
    
    // Calculate offset from camera position
    float offset_x = world_x - x_;
    float offset_y = world_y - y_;
    
    // Convert to screen pixels
    float pixel_offset_x = offset_x * zoom_;
    float pixel_offset_y = offset_y * zoom_;
    
    // Calculate screen position (centered on viewport)
    screen_x = static_cast<int>((viewport_width_ / 2.0f) + pixel_offset_x);
    screen_y = static_cast<int>((viewport_height_ / 2.0f) + pixel_offset_y);
}

void Camera::GetVisibleBounds(int& min_x, int& min_y, int& max_x, int& max_y) const {
    // Calculate how many tiles are visible in each direction
    float tiles_wide = viewport_width_ / zoom_;
    float tiles_high = viewport_height_ / zoom_;
    
    // Calculate bounds with some padding for partial tiles
    min_x = static_cast<int>(std::floor(x_ - tiles_wide / 2.0f - 1.0f));
    min_y = static_cast<int>(std::floor(y_ - tiles_high / 2.0f - 1.0f));
    max_x = static_cast<int>(std::ceil(x_ + tiles_wide / 2.0f + 1.0f));
    max_y = static_cast<int>(std::ceil(y_ + tiles_high / 2.0f + 1.0f));
}

void Camera::Pan(float dx, float dy) {
    // Convert screen pixel delta to world tile delta
    float world_dx = dx / zoom_;
    float world_dy = dy / zoom_;
    
    // Update camera position
    x_ -= world_dx;  // Negative because dragging right should move camera left
    y_ -= world_dy;
}

void Camera::ZoomAt(float delta, int screen_x, int screen_y) {
    // Get world position under mouse before zoom
    int world_x_before, world_y_before;
    ScreenToWorld(screen_x, screen_y, world_x_before, world_y_before);
    
    // Update zoom
    float old_zoom = zoom_;
    zoom_ += delta;
    ClampZoom();
    
    // If zoom didn't actually change (hit limits), don't adjust position
    if (zoom_ == old_zoom) {
        return;
    }
    
    // Get world position under mouse after zoom
    int world_x_after, world_y_after;
    ScreenToWorld(screen_x, screen_y, world_x_after, world_y_after);
    
    // Adjust camera position to keep the same world point under the mouse
    float world_dx = world_x_after - world_x_before;
    float world_dy = world_y_after - world_y_before;
    
    x_ -= world_dx;
    y_ -= world_dy;
}

void Camera::ClampZoom() {
    zoom_ = std::max(MIN_ZOOM, std::min(MAX_ZOOM, zoom_));
}

} // namespace myg
