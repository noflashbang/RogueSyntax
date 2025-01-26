#define CLAY_IMPLEMENTATION
#include "clay.h"
#include <raylib.h>
#include <raymath.h>
#include "clay_renderer_raylib.h"

#include <RogueSyntaxCore.h>
#include "RogueSyntaxCompiler.h"

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;

struct Pallette
{
	Clay_Color background;
    Clay_Color  text;
    Clay_Color  accent;
	Clay_Color  accentText;
    Clay_Color  foreground;
    Clay_Color  highlight;
};

constexpr Clay_Color inline Clay_ColorFromInt(uint32_t value)
{
	return Clay_Color{
		.r = (float)((value >> 16) & 255),
		.g = (float)((value >> 8) & 255),
		.b = (float)((value) & 255),
		.a = (float)(255)
	};
}

Pallette CURRENT_PALLETTE = {
	.background = Clay_ColorFromInt(0x212529),
	.text = Clay_ColorFromInt(0xf8f9fa),
	.accent = Clay_ColorFromInt(0x001d3d),
	.accentText = Clay_ColorFromInt(0x003566),
	.foreground = Clay_ColorFromInt(0xffc300),
	.highlight = Clay_ColorFromInt(0xffd60a)
};

#define RAYLIB_VECTOR2_TO_CLAY_VECTOR2(vector) { .x = vector.x, .y = vector.y }

Clay_LayoutConfig layoutElement = Clay_LayoutConfig{ .padding = {5} };

void HandleClayErrors(Clay_ErrorData errorData) {
	printf("%s", errorData.errorText.chars);
}


Clay_RenderCommandArray CreateLayout() {
    Clay_BeginLayout();

    CLAY(CLAY_ID("Root"),
        CLAY_LAYOUT({  .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, .layoutDirection = CLAY_TOP_TO_BOTTOM, }),
        CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.background }))
    {
        CLAY(CLAY_ID("MenuBar"),
            CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(20)} }),
            CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.accent })) 
        {
            CLAY(
                CLAY_ID("FileButton"),
                CLAY_LAYOUT({ .padding = { 16, 16, 8, 8 } }),
                CLAY_RECTANGLE({
                    .color = { 140, 140, 140, 255 },
                    .cornerRadius = CLAY_CORNER_RADIUS(5)
                    })
            ) {
                CLAY_TEXT(CLAY_STRING("File"), CLAY_TEXT_CONFIG({
                    .textColor = { 255, 255, 255, 255 },
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 16,
                    }));
            }
        }
        CLAY(CLAY_ID("ActionBar"),
            CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(30)} }),
            CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.accent }))
        {
        }
		CLAY(CLAY_ID("MainContent"),
			CLAY_LAYOUT(Clay_LayoutConfig { .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }, .childGap = 16, .layoutDirection = CLAY_LEFT_TO_RIGHT }),
            CLAY_BORDER(Clay_BorderElementConfig { .left = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.background}, .right = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.background}, .top = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.background}, .bottom = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.background}, }))
        {
            CLAY(CLAY_ID("MainWindow"),
                CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_PERCENT(0.66), .height = CLAY_SIZING_GROW(0) }, .childGap = 16, .layoutDirection = CLAY_TOP_TO_BOTTOM, }),
                CLAY_BORDER(Clay_BorderElementConfig{ .left = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.text}, .right = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.text}, .top = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.text}, .bottom = Clay_Border {.width = 2, .color = CURRENT_PALLETTE.text}, }))
            {
                CLAY(CLAY_ID("Editor"),
                    CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_PERCENT(0.75)}}),
                    CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.accentText }))
                {

                }
                CLAY(CLAY_ID("Output"),
                    CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}}),
                    CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.background }))
                {

                }
            }
            CLAY(CLAY_ID("InfoWindow"),
                CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_GROW(0)}}),
                CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.background }))
            {

            }
        }
        CLAY(CLAY_ID("DetailBar"),
            CLAY_LAYOUT({ .sizing = Clay_Sizing {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(20)} }),
            CLAY_RECTANGLE({ .color = CURRENT_PALLETTE.accent })) 
        {
        }
    }
    return Clay_EndLayout();
}

typedef struct
{
    Clay_Vector2 clickOrigin;
    Clay_Vector2 positionOrigin;
    bool mouseDown;
} ScrollbarData;

ScrollbarData scrollbarData = {};

bool debugEnabled = false;

void UpdateDrawFrame(void)
{
    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;

    if (IsKeyPressed(KEY_D)) {
        debugEnabled = !debugEnabled;
        Clay_SetDebugModeEnabled(debugEnabled);
    }
    //----------------------------------------------------------------------------------
    // Handle scroll containers
    Clay_Vector2 mousePosition = RAYLIB_VECTOR2_TO_CLAY_VECTOR2(GetMousePosition());
    Clay_SetPointerState(mousePosition, IsMouseButtonDown(0) && !scrollbarData.mouseDown);
    //printf("W: %f H: %f\n", (float)GetRenderWidth(), (float)GetRenderHeight());
    //printf("X: %f Y: %f\n", mousePosition.x, mousePosition.y);
    
    Clay_SetLayoutDimensions({ (float)GetScreenWidth(), (float)GetScreenHeight() });
    if (!IsMouseButtonDown(0)) 
    {
        scrollbarData.mouseDown = false;
    }

    if (IsMouseButtonDown(0) && !scrollbarData.mouseDown && Clay_PointerOver(Clay__HashString(CLAY_STRING("ScrollBar"), 0, 0))) {
        Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay__HashString(CLAY_STRING("MainContent"), 0, 0));
        scrollbarData.clickOrigin = mousePosition;
        scrollbarData.positionOrigin = *scrollContainerData.scrollPosition;
        scrollbarData.mouseDown = true;
    }
    else if (scrollbarData.mouseDown) 
    {
        Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay__HashString(CLAY_STRING("MainContent"), 0, 0));
        if (scrollContainerData.contentDimensions.height > 0)
        {
            Clay_Vector2 ratio = {
                scrollContainerData.contentDimensions.width / scrollContainerData.scrollContainerDimensions.width,
                scrollContainerData.contentDimensions.height / scrollContainerData.scrollContainerDimensions.height,
            };

            if (scrollContainerData.config.vertical) {
                scrollContainerData.scrollPosition->y = scrollbarData.positionOrigin.y + (scrollbarData.clickOrigin.y - mousePosition.y) * ratio.y;
            }
            if (scrollContainerData.config.horizontal) {
                scrollContainerData.scrollPosition->x = scrollbarData.positionOrigin.x + (scrollbarData.clickOrigin.x - mousePosition.x) * ratio.x;
            }
        }
    }

    Clay_UpdateScrollContainers(true, { mouseWheelX, mouseWheelY }, GetFrameTime());
    // Generate the auto layout for rendering
    //double currentTime = GetTime();
    Clay_RenderCommandArray renderCommands = CreateLayout();
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
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
    Clay_Initialize(clayMemory, { (float)GetScreenWidth(), (float)GetScreenHeight() }, { HandleClayErrors });
    Clay_SetMeasureTextFunction(Clay_RayLib_Render::Raylib_MeasureText, 0);
    Clay_RayLib_Render::Clay_Raylib_Initialize(1024, 768, "~~Rogue//Syntax~~", FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

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

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (reinitializeClay) {
            Clay_SetMaxElementCount(8192);
            totalMemorySize = Clay_MinMemorySize();
            clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
            Clay_Initialize(clayMemory, { (float)GetScreenWidth(), (float)GetScreenHeight() }, { HandleClayErrors });
            reinitializeClay = false;
        }
        UpdateDrawFrame();
    }

	//InteractiveCompiler console;
	//console.Start();
	//console.PrintDecompile();
	//console.Run();
	return 0;
}

