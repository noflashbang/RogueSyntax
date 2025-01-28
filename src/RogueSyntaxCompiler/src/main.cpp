#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clayex.h"
#include "UI.h"

#include <raylib.h>
#include <raymath.h>
#include "clay_renderer_raylib.h"

#include <RogueSyntaxCore.h>
#include "RogueSyntaxCompiler.h"

void HandleClayErrors(Clay_ErrorData errorData) {
	printf("%s", errorData.errorText.chars);
}

Clay_RenderCommandArray CreateLayout(UI& ui) {
    Clay_BeginLayout();
	ui.DoLayout();
    return Clay_EndLayout();
}

bool debugEnabled = false;

double accumulatedTime = 0.0;
bool cursorBlink = false;

void UpdateDrawFrame(UI& ui)
{
    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;

	accumulatedTime += GetFrameTime();

    if (IsKeyPressed(KEY_D)) {
        debugEnabled = !debugEnabled;
        Clay_SetDebugModeEnabled(debugEnabled);
    }
    //----------------------------------------------------------------------------------
    // Handle scroll containers
    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mousePosition, IsMouseButtonDown(0));
    //printf("W: %f H: %f\n", (float)GetRenderWidth(), (float)GetRenderHeight());
    //printf("X: %f Y: %f\n", mousePosition.x, mousePosition.y);
    
    Clay_SetLayoutDimensions({ (float)GetScreenWidth(), (float)GetScreenHeight() });
    
    //Clay_UpdateScrollContainers(true, { mouseWheelX, mouseWheelY }, GetFrameTime());
    // Generate the auto layout for rendering
    //double currentTime = GetTime();
    Clay_RenderCommandArray renderCommands = CreateLayout(ui);
    //printf("layout time: %f microseconds\n", (GetTime() - currentTime) * 1000 * 1000);
    // RENDERING ---------------------------------
    //    currentTime = GetTime();
    BeginDrawing();
    ClearBackground(BLACK);
    Clay_RayLib_Render::Clay_Raylib_Render(renderCommands);
    EndDrawing();
    //    printf("render time: %f ms\n", (GetTime() - currentTime) * 1000);
    //----------------------------------------------------------------------------------
}

bool reinitializeClay = false;

int main(int argc, char *argv[])
{
    Clay_SetMaxElementCount(8192 * 4);
    uint64_t totalMemorySize = Clay_MinMemorySize() * 2;
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(clayMemory, { (float)GetScreenWidth(), (float)GetScreenHeight() }, { HandleClayErrors });
    Clay_SetMeasureTextFunction(Clay_RayLib_Render::Raylib_MeasureText, 0);
    Clay_RayLib_Render::Clay_Raylib_Initialize(1024*2, 1000, "~~Rogue//Syntax~~", FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    Clay_RayLib_Render::Raylib_fonts[FONT_ID_BODY_24] = Raylib_Font {
        .fontId = FONT_ID_BODY_24,
        .font = LoadFontEx("resources/anonymous_pro_bold.ttf", 48, 0, 400),
    };

    SetTextureFilter(Clay_RayLib_Render::Raylib_fonts[FONT_ID_BODY_24].font.texture, TEXTURE_FILTER_BILINEAR);

    Clay_RayLib_Render::Raylib_fonts[FONT_ID_BODY_16] = Raylib_Font{
        .fontId = FONT_ID_BODY_16,
        .font = LoadFontEx("resources/anonymous_pro_bold.ttf", 32, 0, 400),
    };

    SetTextureFilter(Clay_RayLib_Render::Raylib_fonts[FONT_ID_BODY_16].font.texture, TEXTURE_FILTER_BILINEAR);

    Palette DEFAULT_PALETTE = {
    .background = Clay_ColorFromInt(0x212529),
    .text = Clay_ColorFromInt(0xf8f9fa),
    .accent = Clay_ColorFromInt(0x001d3d),
    .accentText = Clay_ColorFromInt(0x003566),
    .foreground = Clay_ColorFromInt(0xffc300),
    .highlight = Clay_ColorFromInt(0xffd60a)
    };

	auto config = UIConfig::MakeDefault();
	UI ui(config);
    
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (reinitializeClay) {
            Clay_SetMaxElementCount(8192);
            totalMemorySize = Clay_MinMemorySize();
            clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
            Clay_Initialize(clayMemory, { (float)GetScreenWidth(), (float)GetScreenHeight() }, { HandleClayErrors });
            reinitializeClay = false;
        }
        UpdateDrawFrame(ui);
    }

	//InteractiveCompiler console;
	//console.Start();
	//console.PrintDecompile();
	//console.Run();
	return 0;
}

