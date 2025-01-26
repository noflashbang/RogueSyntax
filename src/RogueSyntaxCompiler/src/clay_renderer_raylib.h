#pragma once
#include "clay.h"
#include <raylib.h>
#include <raymath.h>
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

// Macros
#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) { .r = (unsigned char)roundf(color.r), .g = (unsigned char)roundf(color.g), .b = (unsigned char)roundf(color.b), .a = (unsigned char)roundf(color.a) }

// Structs
typedef struct
{
    uint32_t fontId;
    Font font;
} Raylib_Font;



typedef enum
{
    CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL
} CustomLayoutElementType;

typedef struct
{
    Model model;
    float scale;
    Vector3 position;
    Matrix rotation;
} CustomLayoutElement_3DModel;

typedef struct
{
    CustomLayoutElementType type;
    union {
        CustomLayoutElement_3DModel model;
    };
} CustomLayoutElement;

class Clay_RayLib_Render
{
public:
    static Raylib_Font Raylib_fonts[10];
    static Camera Raylib_camera;

    // Function declarations
    static Ray GetScreenToWorldPointWithZDistance(Vector2 position, Camera camera, int screenWidth, int screenHeight, float zDistance);
    static Clay_Dimensions Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData);
    static void Clay_Raylib_Initialize(int width, int height, const char* title, unsigned int flags);
    static void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands);
};

