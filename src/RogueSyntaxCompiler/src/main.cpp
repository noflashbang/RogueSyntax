#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clayex.h"
#include "UI.h"

#include <raylib.h>
#include <raymath.h>
#include "clay_renderer_raylib.h"

#include <RogueSyntaxCore.h>
#include "RogueSyntaxCompiler.h"

#include <iostream>
#include <cstdlib>

//void* operator new(std::size_t size) {
//    std::cout << "[Global new] Allocating " << size << " bytes\n";
//    return std::malloc(size);
//}
//
//void operator delete(void* ptr) noexcept {
//    std::cout << "[Global delete] Freeing memory\n";
//    std::free(ptr);
//}



void HandleClayErrors(Clay_ErrorData errorData) {
	printf("%s", errorData.errorText.chars);
}

Clay_RenderCommandArray CreateLayout(UI* ui) {
    Clay_BeginLayout();
	ui->DoLayout();
    return Clay_EndLayout();
}

bool debugEnabled = false;

double accumulatedTime = 0.0;
bool cursorBlink = false;

void UpdateDrawFrame(UI* ui)
{
    Vector2 mouseWheelDelta = GetMouseWheelMoveV();
    float mouseWheelX = mouseWheelDelta.x;
    float mouseWheelY = mouseWheelDelta.y;

	accumulatedTime += GetFrameTime();

    //if (IsKeyPressed(KEY_D)) {
    //    debugEnabled = !debugEnabled;
    //    Clay_SetDebugModeEnabled(debugEnabled);
    //}
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
std::string fileName = "";

UI* pUi;
InteractiveCompiler* pConsole;
bool _closeWindow = false;
void OnCommand(std::string cmd);
void OnOpenFile();
void OnOpenFormClosed(const std::string& filename);
void OnSaveFile();
void OnSaveAsFile();
void OnNewFile();
void OnClose();


static std::string GetFileAsString(const std::string& filename)
{
    std::ifstream file(filename);
    char readBuffer[256];
    std::string file_contents;

	memset(readBuffer, 0, sizeof(readBuffer));

	if (file.is_open())
	{
		while (!file.eof())
		{
			file.read(readBuffer, sizeof(readBuffer));
			file_contents.append(readBuffer, file.gcount());
		}
		file.close();
	}
	else
	{
		std::cout << "Could not open file " << filename << std::endl;
	}
    return file_contents;
};

static void SaveFile(const std::string& filename, const std::string& content)
{
	std::ofstream file(filename, std::ios::trunc);
	if (file.is_open())
	{
		file << content;
		file.close();
	}
	else
	{
		std::cout << "Could not open file " << filename << std::endl;
	}
}

int main(int argc, char* argv[])
{
    Clay_SetMaxElementCount(8192 * 6);
    uint64_t totalMemorySize = Clay_MinMemorySize();
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

	config.colors = DEFAULT_PALETTE;

    pConsole = new InteractiveCompiler();
	pUi  = new UI(config);

    pUi->SetOutputPrompt(pConsole->GetPrompt());

	auto conn = pUi->onCmd() += OnCommand;

	pUi->AddMenu("File", { "New", "Open", "Save", "Save As", "Close" });
	auto connOpen = pUi->Subscribe("File", "Open", OnOpenFile);
	auto connSave = pUi->Subscribe("File", "Save", OnSaveFile);
	auto connSaveAs = pUi->Subscribe("File", "Save As", OnSaveAsFile);
	auto connNew = pUi->Subscribe("File", "New", OnNewFile);
	auto connClose = pUi->Subscribe("File", "Close", OnClose);
	auto connOnOpenClosed = pUi->GetOpenForm()->onFormClosed() += OnOpenFormClosed;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (reinitializeClay) {
            Clay_SetMaxElementCount(8192);
            totalMemorySize = Clay_MinMemorySize();
            clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
            Clay_Initialize(clayMemory, { (float)GetScreenWidth(), (float)GetScreenHeight() }, { HandleClayErrors });
            reinitializeClay = false;
        }
        UpdateDrawFrame(pUi);

		if (_closeWindow) 
        {
			CloseWindow();
		}
    }

	delete pUi;
	delete pConsole;

	return 0;
}

void OnOpenFile()
{
	pUi->GetFocusObserver()->SetEventData("OpenForm");
	std::cout << "Open file" << std::endl;
}

void OnOpenFormClosed(const std::string& filename)
{
    std::cout << "Loading file contents" << std::endl;
    pUi->ClearCommand();

	auto fileContents = GetFileAsString(filename);

    pUi->SetEditor(fileContents);
    fileName = filename;
}

void OnSaveFile()
{
	std::cout << "Save file" << std::endl;
	if (fileName.empty())
	{
		OnSaveAsFile();
	}
	else
	{
		std::string code = pUi->GetEditorText();
		SaveFile(fileName, code);
	}
}

void OnSaveAsFile()
{
	std::cout << "Save as file" << std::endl;
    pUi->GetFocusObserver()->SetEventData("SaveForm");
}

void OnNewFile()
{
	std::cout << "New file" << std::endl;
	pUi->ClearCommand();
	pUi->SetEditor("");
	fileName = "";
}

void OnClose()
{
    _closeWindow = true;
}

void OnCommand(std::string cmd)
{
	auto log = std::format("{}{}\n", pConsole->GetPrompt(), cmd);
	pUi->AddOutputText(log);
    pUi->ClearCommand();

    std::stringstream cmdStream(cmd);
    std::string curCmd;

    while (std::getline(cmdStream, curCmd, ' '))
    {
        if (curCmd == "!q")
        {
            pUi->AddOutputText("Exiting...");
            _closeWindow = true;
        }
        else if (curCmd == "!r")
        {
            std::string code = pUi->GetEditorText();
            pUi->AddOutputText(std::format("Compiling {} chars", code.length()));
            auto byteCode = pConsole->Compile(code);
            pUi->AddOutputText(std::format("Running {} bytes", byteCode.Instructions.size()));
            pConsole->Run(byteCode);
            pUi->AddOutputText("Result>");

            auto result = pConsole->GetResult();

            for (const auto& line : result)
            {
                pUi->AddOutputText(line);
            }
        }
        else if (curCmd == "!e")
        {
            pUi->AddOutputText("Error>");
            pUi->AddOutputText(pConsole->Get_RTI_Error());
        }
        else if (curCmd == "!g")
        {
            std::string idx;
            std::getline(cmdStream, idx, ' ');

			if (!idx.empty())
			{
				pUi->AddOutputText("Global[" + idx + "]=");
				pUi->AddOutputText(pConsole->Get_RTI_Global(std::stoi(idx)));
			}
			else
			{
				pUi->AddOutputText("!g must have an index");
			}
        }
        else if (curCmd == "!l")
        {
            std::string frame;
            std::string idx;
            std::getline(cmdStream, frame, ' ');
            std::getline(cmdStream, idx, ' ');

			if (!idx.empty() && !frame.empty())
			{
                pUi->AddOutputText("Local [" + frame + "][" + idx + "]=");
                pUi->AddOutputText(pConsole->Get_RTI_Local(std::stoi(frame), std::stoi(idx)));
			}
            else
            {
				pUi->AddOutputText("!l must have a frame and index");
            }
        }
        else if (curCmd == "!dp")
        {
            std::string code = pUi->GetEditorText();
            pUi->AddOutputText(std::format("Compiling {} chars", code.length()));
            auto byteCode = pConsole->Compile(code);
            pUi->AddOutputText(std::format("Bytecode is {} bytes", byteCode.Instructions.size()));
            pUi->SetInfoText(pConsole->Decompile(byteCode));
		}
		else if (curCmd == "!st")
		{
			pUi->AddOutputText("Stack Trace>");
			pUi->AddOutputText(pConsole->Get_RTI_StackTrace().ToString());
		}
        else if (curCmd == "!br")
        {
			std::string idx;
			std::getline(cmdStream, idx, ' ');
			if (!idx.empty())
			{
				pUi->AddOutputText("Break at ip " + idx);
				//pConsole->BreakAt(std::stoi(idx));
			}
			else
			{
				pUi->AddOutputText("!br must have an index");
			}
        }
        else if (curCmd == "!h")
        {
            pUi->AddOutputText("Commands:");
            pUi->AddOutputText("!q - Quit");
            pUi->AddOutputText("!r - Run");
            pUi->AddOutputText("!dp - Decompile");
			pUi->AddOutputText("!e - Display Error information");
			pUi->AddOutputText("!g # - Global idx#");
			pUi->AddOutputText("!l # # - Local frame# idx#");
            pUi->AddOutputText("!st - Stack trace");
			pUi->AddOutputText("!br # - Break at ip#");
            pUi->AddOutputText("!h - Print this menu");
        }
        else
        {
            pUi->AddOutputText("Unknown command ->" + cmd);
            pUi->AddOutputText("Use !h for help");
        }
    }
}

