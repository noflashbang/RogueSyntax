#include "clay_renderer_raylib.h"

Raylib_Font Clay_RayLib_Render::Raylib_fonts[10];
Camera Clay_RayLib_Render::Raylib_camera;

Clay_Dimensions Clay_RayLib_Render::Raylib_MeasureText(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData) {
    // Measure string size for Font
    Clay_Dimensions textSize = { 0 };

    float maxTextWidth = 0.0f;
    float lineTextWidth = 0;

    float textHeight = config->fontSize;
    Font fontToUse = Raylib_fonts[config->fontId].font;
    // Font failed to load, likely the fonts are in the wrong place relative to the execution dir
    if (!fontToUse.glyphs) return textSize;

    float scaleFactor = config->fontSize / (float)fontToUse.baseSize;

    for (int i = 0; i < text.length; ++i)
    {
        if (text.chars[i] == '\n') {
            maxTextWidth = fmax(maxTextWidth, lineTextWidth);
            lineTextWidth = 0;
            continue;
        }
        int index = text.chars[i] - 32;
        if (fontToUse.glyphs[index].advanceX != 0) lineTextWidth += fontToUse.glyphs[index].advanceX;
        else lineTextWidth += (fontToUse.recs[index].width + fontToUse.glyphs[index].offsetX);
    }

    maxTextWidth = fmax(maxTextWidth, lineTextWidth);

    textSize.width = maxTextWidth * scaleFactor;
    textSize.height = textHeight;

    return textSize;
}

void Clay_RayLib_Render::Clay_Raylib_Initialize(int width, int height, const char* title, unsigned int flags) {
    SetConfigFlags(flags);
    InitWindow(width, height, title);
    //    EnableEventWaiting();
}

void Clay_RayLib_Render::Clay_Raylib_Render(Clay_RenderCommandArray renderCommands)
{
    char buffer[1024];
    for (int j = 0; j < renderCommands.length; j++)
    {
        Clay_RenderCommand* renderCommand = Clay_RenderCommandArray_Get(&renderCommands, j);
        Clay_BoundingBox boundingBox = renderCommand->boundingBox;
        switch (renderCommand->commandType)
        {
        case CLAY_RENDER_COMMAND_TYPE_TEXT: {
			
			if (renderCommand->text.length >= 1024)
            {
                // Raylib uses standard C strings so isn't compatible with cheap slices, we need to clone the string to append null terminator
                Clay_StringSlice text = renderCommand->text;
                char* cloned = (char*)malloc(text.length + 1);
                memcpy(cloned, text.chars, text.length);

                cloned[text.length] = '\0';
                Font fontToUse = Raylib_fonts[renderCommand->config.textElementConfig->fontId].font;
                DrawTextEx(fontToUse, cloned, { boundingBox.x, boundingBox.y }, (float)renderCommand->config.textElementConfig->fontSize, (float)renderCommand->config.textElementConfig->letterSpacing, CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->config.textElementConfig->textColor));
                free(cloned);
			}
            else
            {
                // Raylib uses standard C strings so isn't compatible with cheap slices, we need to clone the string to append null terminator
                Clay_StringSlice text = renderCommand->text;
                memcpy(buffer, text.chars, text.length);

                buffer[text.length] = '\0';
                Font fontToUse = Raylib_fonts[renderCommand->config.textElementConfig->fontId].font;
                DrawTextEx(fontToUse, buffer, { boundingBox.x, boundingBox.y }, (float)renderCommand->config.textElementConfig->fontSize, (float)renderCommand->config.textElementConfig->letterSpacing, CLAY_COLOR_TO_RAYLIB_COLOR(renderCommand->config.textElementConfig->textColor));
            }
            
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
            Texture2D imageTexture = *(Texture2D*)renderCommand->config.imageElementConfig->imageData;
            DrawTextureEx(
                imageTexture,
                 {
                boundingBox.x, boundingBox.y
            },
                0,
                boundingBox.width / (float)imageTexture.width,
                WHITE);
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
            BeginScissorMode((int)roundf(boundingBox.x), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width), (int)roundf(boundingBox.height));
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
            EndScissorMode();
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            Clay_RectangleElementConfig* config = renderCommand->config.rectangleElementConfig;
            if (config->cornerRadius.topLeft > 0) {
                float radius = (config->cornerRadius.topLeft * 2) / (float)((boundingBox.width > boundingBox.height) ? boundingBox.height : boundingBox.width);
                DrawRectangleRounded( { boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height }, radius, 8, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            else {
                DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, CLAY_COLOR_TO_RAYLIB_COLOR(config->color));
            }
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            Clay_BorderElementConfig* config = renderCommand->config.borderElementConfig;
            // Left border
            if (config->left.width > 0) {
                DrawRectangle((int)roundf(boundingBox.x), (int)roundf(boundingBox.y + config->cornerRadius.topLeft), (int)config->left.width, (int)roundf(boundingBox.height - config->cornerRadius.topLeft - config->cornerRadius.bottomLeft), CLAY_COLOR_TO_RAYLIB_COLOR(config->left.color));
            }
            // Right border
            if (config->right.width > 0) {
                DrawRectangle((int)roundf(boundingBox.x + boundingBox.width - config->right.width), (int)roundf(boundingBox.y + config->cornerRadius.topRight), (int)config->right.width, (int)roundf(boundingBox.height - config->cornerRadius.topRight - config->cornerRadius.bottomRight), CLAY_COLOR_TO_RAYLIB_COLOR(config->right.color));
            }
            // Top border
            if (config->top.width > 0) {
                DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.topLeft), (int)roundf(boundingBox.y), (int)roundf(boundingBox.width - config->cornerRadius.topLeft - config->cornerRadius.topRight), (int)config->top.width, CLAY_COLOR_TO_RAYLIB_COLOR(config->top.color));
            }
            // Bottom border
            if (config->bottom.width > 0) {
                DrawRectangle((int)roundf(boundingBox.x + config->cornerRadius.bottomLeft), (int)roundf(boundingBox.y + boundingBox.height - config->bottom.width), (int)roundf(boundingBox.width - config->cornerRadius.bottomLeft - config->cornerRadius.bottomRight), (int)config->bottom.width, CLAY_COLOR_TO_RAYLIB_COLOR(config->bottom.color));
            }
            if (config->cornerRadius.topLeft > 0) {
                DrawRing( { roundf(boundingBox.x + config->cornerRadius.topLeft), roundf(boundingBox.y + config->cornerRadius.topLeft) }, roundf(config->cornerRadius.topLeft - config->top.width), config->cornerRadius.topLeft, 180, 270, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->top.color));
            }
            if (config->cornerRadius.topRight > 0) {
                DrawRing( { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.topRight), roundf(boundingBox.y + config->cornerRadius.topRight) }, roundf(config->cornerRadius.topRight - config->top.width), config->cornerRadius.topRight, 270, 360, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->top.color));
            }
            if (config->cornerRadius.bottomLeft > 0) {
                DrawRing( { roundf(boundingBox.x + config->cornerRadius.bottomLeft), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomLeft) }, roundf(config->cornerRadius.bottomLeft - config->top.width), config->cornerRadius.bottomLeft, 90, 180, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->bottom.color));
            }
            if (config->cornerRadius.bottomRight > 0) {
                DrawRing( { roundf(boundingBox.x + boundingBox.width - config->cornerRadius.bottomRight), roundf(boundingBox.y + boundingBox.height - config->cornerRadius.bottomRight) }, roundf(config->cornerRadius.bottomRight - config->bottom.width), config->cornerRadius.bottomRight, 0.1, 90, 10, CLAY_COLOR_TO_RAYLIB_COLOR(config->bottom.color));
            }
            break;
        }
        default: {
            printf("Error: unhandled render command.");
            exit(1);
        }
        }
    }
}