# PowerShell script to set up MYG Editor dependencies

Write-Host "Setting up MYG Editor dependencies..." -ForegroundColor Green

# Check if external directory exists
if (-not (Test-Path "external")) {
    Write-Host "Error: Run this script from the MYGEditor directory" -ForegroundColor Red
    exit 1
}

# Download stb_image.h
Write-Host "`nDownloading stb_image.h..." -ForegroundColor Cyan
$stbPath = "external/stb/stb_image.h"
if (Test-Path $stbPath) {
    Write-Host "  stb_image.h already exists" -ForegroundColor Yellow
} else {
    try {
        Invoke-WebRequest -Uri "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" -OutFile $stbPath
        Write-Host "  Downloaded stb_image.h successfully" -ForegroundColor Green
    } catch {
        Write-Host "  Failed to download stb_image.h: $_" -ForegroundColor Red
    }
}

# Clone ImGui
Write-Host "`nCloning Dear ImGui..." -ForegroundColor Cyan
if (Test-Path "external/imgui") {
    Write-Host "  ImGui directory already exists" -ForegroundColor Yellow
} else {
    try {
        git clone https://github.com/ocornut/imgui.git external/imgui
        Write-Host "  Cloned ImGui successfully" -ForegroundColor Green
    } catch {
        Write-Host "  Failed to clone ImGui: $_" -ForegroundColor Red
        Write-Host "  You can manually clone: git clone https://github.com/ocornut/imgui.git external/imgui" -ForegroundColor Yellow
    }
}

# Check for SDL3
Write-Host "`nChecking for SDL3..." -ForegroundColor Cyan
if (Test-Path "external/SDL3") {
    Write-Host "  SDL3 found in external/SDL3" -ForegroundColor Green
} else {
    Write-Host "  SDL3 not found in external/SDL3" -ForegroundColor Yellow
    Write-Host "  Options:" -ForegroundColor Yellow
    Write-Host "    1. Install SDL3 system-wide" -ForegroundColor Yellow
    Write-Host "    2. Clone SDL3 to external/SDL3: git clone https://github.com/libsdl-org/SDL.git external/SDL3" -ForegroundColor Yellow
    Write-Host "    3. Download SDL3 development libraries and extract to external/SDL3" -ForegroundColor Yellow
}

Write-Host "`nDependency setup complete!" -ForegroundColor Green
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Ensure SDL3 is available (see above)" -ForegroundColor White
Write-Host "  2. Run: mkdir build; cd build; cmake .." -ForegroundColor White
Write-Host "  3. Run: cmake --build . --config Release" -ForegroundColor White
