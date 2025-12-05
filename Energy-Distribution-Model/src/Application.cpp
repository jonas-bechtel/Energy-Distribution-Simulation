#include "pch.h"

#include "ImGuiStuff.h"

#include "Application.h"
#include "MCMC.h"
#include "ElectronBeam.h"
#include "IonBeam.h"
#include "LabEnergies.h"
#include "EnergyDistributionWindow.h"
#include "CoolingForceWindow.h"
#include "DeconvolutionWindow.h"


namespace Application
{
    static Settings settings = Settings();

    Settings& GetSettings()
    {
        return settings;
    }
}

void Application::Run()
{
    // Main loop
    bool done = false;
    while (!done)
    {
        PollEvents(done);

        if (done)
            break;

        BeginImGuiFrame();

        ShowWindows();

        RenderImGuiFrame();
    }
}

void Application::Init()
{
    InitImGui();

    MCMC::Init();
    ElectronBeam::Init();
    IonBeam::Init();
    LabEnergy::Init();
    EnergyDistributionWindow::Init();
    CoolingForce::Init();
    DeconvolutionWindow::Init();

    //gStyle->SetOptStat(0);
}
   
void Application::ShowWindows()
{
    ImGui::DockSpaceOverViewport();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("disable tooltips", 0, settings.tooltipsDisabled))
            {
                settings.tooltipsDisabled = !settings.tooltipsDisabled;
            }
            if (ImGui::MenuItem("Show ImGui Demo", 0, settings.showImGuiDemoWindow))
            {
                settings.showImGuiDemoWindow = !settings.showImGuiDemoWindow;
            }
            if (ImGui::MenuItem("Show ImPlot Demo", 0, settings.showImPlotDemoWindow))
            {
                settings.showImPlotDemoWindow = !settings.showImPlotDemoWindow;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Cross Section Deconvolution Window"))
    {
        ImGuiID dockspace_id = ImGui::GetID("cs deconvolution");
        ImGui::DockSpace(dockspace_id);

        DeconvolutionWindow::ShowWindow();

    }
    ImGui::End();

    if (ImGui::Begin("Energy Distribution Generation Window"))
    {
        ImGuiID dockspace_id = ImGui::GetID("e-dist generation");
        ImGui::DockSpace(dockspace_id);

        MCMC::ShowWindow();
        ElectronBeam::ShowWindow();
        IonBeam::ShowWindow();
        LabEnergy::ShowWindow();
        CoolingForce::ShowWindow();
        EnergyDistributionWindow::ShowWindow();

    }
    ImGui::End();

    if(settings.showImGuiDemoWindow)
        ImGui::ShowDemoWindow(&settings.showImGuiDemoWindow);
    if (settings.showImPlotDemoWindow)
        ImPlot::ShowDemoWindow(&settings.showImPlotDemoWindow);

    // Handle ROOT events
    gSystem->ProcessEvents();
}

void Application::Shutdown()
{
    // Close ROOT canvases first to ensure their native windows are destroyed
    // before we tear down ImGui / DirectX resources. This prevents TRootCanvas
    // callbacks/destructors from touching already-destroyed Win32/DX objects.
    if (gROOT && gROOT->GetListOfCanvases())
    {
        TIter next(gROOT->GetListOfCanvases());
        TCanvas* c = nullptr;
        while ((c = static_cast<TCanvas*>(next())))
        {
            // Close removes the TRootCanvas window and should avoid dangling window handles.
            try
            {
                c->Close();
            }
            catch (...)
            {
                // swallow exceptions to continue cleanup
            }
        }

        // Give ROOT a chance to process the window close events synchronously.
        gSystem->ProcessEvents();
    }

	ShutdownImGui();
}




