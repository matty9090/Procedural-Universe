#include "UI.hpp"

#include "Services/Log.hpp"

#include <imgui.h>
#include "ImGuiDx11.h"
#include "ImGuiWin32.h"

UI::UI(ID3D11DeviceContext* context, HWND hwnd)
{    
    ID3D11Device* device = nullptr;
    context->GetDevice(&device);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
}

UI::~UI()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void UI::Render()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(260, io.DisplaySize.y));

    ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    ImGui::Text("FPS: %i", (int)FPS);
    ImGui::Separator();
    ImGui::SliderInt("Particles", &Particles, 0, 10000);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void UI::Update(float dt)
{
    ++Frames;
    FPSTimer -= dt;

    if(FPSTimer < 0.0f)
    {
        FPS = static_cast<float>(Frames) / FPSUpdate;
        Frames = 0;
        FPSTimer = FPSUpdate;
    }
}

void UI::Reset()
{

}