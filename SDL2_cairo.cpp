
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

#include <cairo/cairo.h>

#include "SDL.h"

constexpr int gWINDOW_WEIGHT = 1000;
constexpr int gWINDOW_HEIGHT = 800;
constexpr float gfWINDOW_WEIGHT = 1000.0;
constexpr float gfWINDOW_HEIGHT = 800.0;

constexpr int gFPS = 60;
constexpr Uint32 gFPS_TIME = 1000 / gFPS;

constexpr int unitScale = 2*50;

// 渲染计数，作为不精确的定时器
static int renderCount = 0;

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

void DrawFillRectWithCairo(cairo_t *cr, int x, int y) 
{
    // 绘制
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1, 1, 1, 0.7);
    cairo_set_line_width(cr, 4);
    cairo_rectangle(cr, x, y, 100, 50);
    cairo_stroke_preserve (cr);
    cairo_set_source_rgba(cr, 0.5, 0.2, 0.3, 0.7);
    cairo_fill(cr);
    cairo_restore(cr);
}

void DisplayImageWithCairo(cairo_t *cr, int x, int y)
{
    cairo_surface_t *image = cairo_image_surface_create_from_png("./dice.png");
    if (!image) {
        std::cout << "cairo_image_surface_create_from_png Failed!" << std::endl;
        cairo_status_t stat = cairo_status(cr);
        std::cout << "stat: " << cairo_status_to_string(stat) << std::endl;
        return ;
    }
    int imgW = cairo_image_surface_get_width(image);
    int imgH = cairo_image_surface_get_height(image);
    cairo_save(cr);
    cairo_set_line_width(cr, 4);
    cairo_translate (cr, imgW/2, imgH/2);
    cairo_rotate (cr, (static_cast<int>(renderCount/2) % 360) * M_PI/180);
    cairo_translate (cr, -imgW/2, -imgH/2);
    cairo_set_source_rgba(cr, 1, 1, 1, 0.7);
    cairo_rectangle(cr, 0, 0, imgW, imgH);
    cairo_stroke(cr);
    cairo_set_source_surface(cr, image, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);
    cairo_surface_destroy(image);
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
        // DrawLineWithCairo(cr, clickX, clickY);
        // DrawFillRectWithCairo(cr, clickX, clickY);
        DisplayImageWithCairo(cr, clickX, clickY);
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
        // 记录循环次数
        renderCount++;
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