
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

#include <cairo/cairo.h>

#include "SDL.h"

#include <vector>
#include <iostream>
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

constexpr int gWINDOW_WEIGHT = 1000;
constexpr int gWINDOW_HEIGHT = 800;

constexpr int gFPS = 60;
constexpr Uint32 gFPS_TIME = 1000 / gFPS;

constexpr int unitScale = 2*50;

void ClearCairo(cairo_t *cr)
{
    // 清理
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0, 0.4, 0, 1);
    cairo_paint(cr);

    // 设置画布刻度
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgba(cr, 0, 0, 0, 1);
    for (int i = 0; i < gWINDOW_HEIGHT / unitScale; i++) {
        cairo_move_to(cr, 0, unitScale * i);
        cairo_line_to(cr, gWINDOW_WEIGHT, unitScale * i);
    }
    for (int i = 0; i < gWINDOW_WEIGHT / unitScale; i++) {
        cairo_move_to(cr, unitScale * i, 0);
        cairo_line_to(cr, unitScale * i, gWINDOW_HEIGHT);
    }
    cairo_stroke(cr);

    cairo_restore(cr);
}

void DrawLineWithCairo(cairo_t *cr, int x, int y) 
{
    // 绘制
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1, 0.2, 1, 0.7);
    cairo_set_line_width(cr, 10);
    cairo_move_to(cr, gWINDOW_WEIGHT / 2, gWINDOW_HEIGHT / 2);
    cairo_line_to(cr, x, y);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void DrawRectWithCairo(cairo_t *cr, int x, int y) 
{
    // 绘制
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.5, 0.2, 1, 0.7);
    cairo_set_line_width(cr, 4);
    cairo_rectangle(cr, x, y, 100, 50);
    cairo_stroke(cr);
    cairo_restore(cr);
}

std::vector<char> raster_direct(int width, int height,
                                void (*draw)(SkCanvas*)) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    size_t rowBytes = info.minRowBytes();
    size_t size = info.computeByteSize(rowBytes);
    std::vector<char> pixelMemory(size);  // allocate memory
    sk_sp<SkSurface> surface =
            SkSurface::MakeRasterDirect(
                    info, &pixelMemory[0], rowBytes);
    SkCanvas* canvas = surface->getCanvas();
    draw(canvas);
    return pixelMemory;
}

void example(SkCanvas* canvas) {
    const SkScalar scale = 256.0f;
    const SkScalar R = 0.45f * scale;
    const SkScalar TAU = 6.2831853f;
    SkPath path;
    for (int i = 0; i < 5; ++i) {
        SkScalar theta = 2 * i * TAU / 5;
        if (i == 0) {
            path.moveTo(R * cos(theta), R * sin(theta));
        } else {
            path.lineTo(R * cos(theta), R * sin(theta));
        }
    }
    path.close();
    SkPaint p;
    p.setAntiAlias(true);
    canvas->clear(SK_ColorWHITE);
    canvas->translate(0.5f * scale, 0.5f * scale);
    canvas->drawPath(path, p);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    // 初始化SDL
    ret = SDL_Init(SDL_INIT_VIDEO);
    if (ret != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n",SDL_GetError());
        return -1;
    }
    // 创建一个 SDL 窗口
    SDL_Window *screen = SDL_CreateWindow("Hello SDL",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gWINDOW_WEIGHT, gWINDOW_HEIGHT, 0);
    if (screen == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't Create SDL Window: %s\n",SDL_GetError());
        SDL_Quit();
        return -1;
    }
    // 加载一张图片
    SDL_Surface *surfaceIcon = SDL_LoadBMP("./hello_SDL2.bmp");
    if (surfaceIcon == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't SDL_LoadBMP: %s\n",SDL_GetError());
        SDL_DestroyWindow(screen);
        SDL_Quit();
        return -1;
    }
    // 设置应用图标
    SDL_SetWindowIcon(screen, surfaceIcon);
    // 释放 surfaceIcon
    SDL_FreeSurface(surfaceIcon);
    // 创建 render
    SDL_Renderer *render = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
    if (render == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't SDL_CreateRenderer: %s\n",SDL_GetError());
        SDL_DestroyWindow(screen);
        SDL_Quit();
        return -1;
    }

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, gWINDOW_WEIGHT, gWINDOW_HEIGHT);
    cairo_t *cr = cairo_create(surface);
    if (surface == nullptr || cr == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Call cairo_image_surface_get_data error.");
    }

    std::vector<char> skiaPixels = raster_direct(gWINDOW_WEIGHT, gWINDOW_HEIGHT, example);

    SDL_Texture *texture;
    int partTextureW = gWINDOW_WEIGHT;
    int partTextureH = gWINDOW_HEIGHT;
    int32_t *pixels = nullptr;
    SDL_Rect rect = {.x=0, .y=0, .w=gWINDOW_WEIGHT, .h=gWINDOW_HEIGHT};
    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, gWINDOW_WEIGHT, gWINDOW_HEIGHT);
    // SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    // SDL_UpdateTexture(texture, &rect, cairoData, 1200); // 采用更高效的lock模式

    int clickX = 0;
    int clickY = 0;
    int keyboardEvent = 0;
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        std::chrono::time_point start = std::chrono::high_resolution_clock::now();
        Uint32 startMs = SDL_GetTicks();
        /* deal with event */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            if (event.type == SDL_MOUSEMOTION) {
                clickX = event.motion.x > partTextureW ? 0 : event.motion.x;
                clickY = event.motion.y > partTextureH ? 0 : event.motion.y;
            }
        }
        /* do your job */
        // 清屏
        SDL_SetRenderDrawColor(render, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear(render);

        // draw
        ClearCairo(cr);
        DrawLineWithCairo(cr, clickX, clickY);
        DrawRectWithCairo(cr, clickX, clickY);
        unsigned char *cairoData = cairo_image_surface_get_data(surface);
        if (cairoData == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Call cairo_image_surface_get_data error.");
            return -1;
        }

        // 将pixels切换到texture
        int pitch= 0;
        // 读取显存中的像素信息到 pixels
        SDL_LockTexture(texture, nullptr, reinterpret_cast<void **>(&pixels), &pitch);
        // 绘制像素
        memcpy(pixels, cairoData, pitch * rect.h);
        // memcpy(pixels, skiaPixels.data(), pitch * rect.h);
        // 将内存中的 pixels 更新到显存
        SDL_UnlockTexture(texture);

        SDL_Point point = {0, 0};
        // 将已经渲染好的texture，渲染到窗口
        SDL_RenderCopyEx(render, texture, nullptr, nullptr, 0, &point, SDL_FLIP_NONE);
        // 显示
        SDL_RenderPresent(render);

        // 调整帧率
        Uint32 endMs = SDL_GetTicks();
        Uint32 consumeTime = endMs - startMs;
        SDL_Delay(consumeTime >= gFPS_TIME ? 0 : (gFPS_TIME - consumeTime)); // 调整帧率

        std::chrono::time_point end = std::chrono::high_resolution_clock::now();
        int fps = std::chrono::seconds(1) / (end - start);
        // 修改应用标题
        SDL_SetWindowTitle(screen, (std::string("x,y : ") + std::to_string(clickX) + std::string(",") + std::to_string(clickY)).c_str());
    }
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    // 销毁 texture
    SDL_DestroyTexture(texture);
    // 销毁 render
    SDL_DestroyRenderer(render);
    // 销毁 SDL 窗口
    SDL_DestroyWindow(screen);
    // SDL 退出
    SDL_Quit();

    return 0;
}