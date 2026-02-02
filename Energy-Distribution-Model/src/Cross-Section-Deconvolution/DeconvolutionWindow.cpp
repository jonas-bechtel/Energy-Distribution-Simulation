#include "pch.h"

#include "DeconvolutionWindow.h"
#include "BoltzmannDistribution.h"
#include "EnergyDistributionSet.h"
#include "EnergyDistributionWindow.h"
#include "CrossSectionManager.h"
#include "RateCoefficientManager.h"
#include "RateCoefficient.h"
#include "PlasmaRateCoefficient.h"
#include "PlasmaRateCoefficientManager.h"
#include "CrossSection.h"

#include "FileUtils.h"
#include <Constants.h>

namespace DeconvolutionWindow
{
	// plot parameters
	static bool logX = true;
	static bool logY = true;
	static bool showMarkers = false;
	static bool plotAsHist = false;
	static bool showSubfunctions = false;
	 
	static bool showBoltzmannConvolutionWindow = false;
	
	static bool showPlasmaRateWindow = false;
	
	void Init()
	{
		BoltzmannDistribution::Update(CrossSection("bla"));

		CrossSectionManager::AddCrossSectionFolder(CrossSectionFolder("test"));
		RateCoefficientManager::AddRateCoefficientFolder(RateCoefficientFolder("measured rcs"));

		// temp: load default rate coefficient
		RateCoefficient rc("mbrc");
		rc.Load(FileUtils::GetMeasuredRateCoefficientFolder() / "amb-Ed_IeXXX_t0.5-14.0_pN0gt0_c00.dat");
		RateCoefficientManager::GetFolders().at(0).AddRateCoefficient(rc);
	}

	void ShowWindow()
	{
		if (ImGui::Begin("Deconvolution Window"))
		{
			ImGui::BeginGroup();
			EnergyDistributionWindow::ShowSetList();
			RateCoefficientManager::ShowFolderList();
			CrossSectionManager::ShowFolderList();
			PlasmaRateCoefficientManager::ShowFolderList();
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			ShowSettings();
			ShowPlots();
			ImGui::EndGroup();
			
			//CrossSection& currentCrosssection = GetCurrentCS();
			BoltzmannDistribution::ShowWindow(showBoltzmannConvolutionWindow, CrossSection("bla"));

			CrossSectionManager::ShowFittinAndBinningSettings();

			ShowPlasmaRateWindow();
		}
		ImGui::End();
	}

	void ShowSettings()
	{
		ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
		if (ImGui::BeginChild("(De)convolveSettings", ImVec2(100.0f, 0.0f), flags))
		{
			if (ImGui::BeginTabBar("controls"))
			{
				if (ImGui::BeginTabItem("Deconvolution"))
				{
					CrossSectionManager::ShowDeconvolutionControls();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Convolution"))
				{
					RateCoefficientManager::ShowConvolutionControls();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::EndChild();
	}

	void ShowPlots()
	{
		if (ImPlot::BeginPlot("rate coefficient"))
		{
			ImPlot::SetupAxis(ImAxis_X1, "detuning energy [eV]");
			ImPlot::SetupAxis(ImAxis_Y1, "rate coefficient");
			if (logX) ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
			ImPlot::SetupLegend(ImPlotLocation_NorthEast);

			int i = 0;
			for (RateCoefficientFolder& folder : RateCoefficientManager::GetFolders())
			{
				for (const RateCoefficient& rc : folder.GetRateCoefficients())
				{
					ImGui::PushID(i++);
					if (showSubfunctions)
					{
						rc.PlotSubfunctions();
					}
					rc.Plot(showMarkers);
					ImGui::PopID();
				}
			}
			ImPlot::EndPlot();
		}

		ImGui::Checkbox("log X", &logX);
		ImGui::SameLine();
		ImGui::Checkbox("log Y", &logY);
		ImGui::SameLine();
		ImGui::Checkbox("show markers", &showMarkers);
		ImGui::SameLine();
		ImGui::Checkbox("plot as hist", &plotAsHist);
		ImGui::SameLine();
		ImGui::Checkbox("show f_pl", &showBoltzmannConvolutionWindow);
		ImGui::SameLine();
		ImGui::Checkbox("show subfunctions", &showSubfunctions);
		ImGui::SameLine();
		ImGui::Checkbox("show plasma rates", &showPlasmaRateWindow);

		if (ImPlot::BeginPlot("cross section"))
		{
			ImPlot::SetupAxis(ImAxis_X1, "collision energy [eV]");
			ImPlot::SetupAxis(ImAxis_Y1, "sigma(E)");
			if (logX) ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
			ImPlot::SetupLegend(ImPlotLocation_NorthEast);
			int i = 0;
			for (CrossSectionFolder& folder : CrossSectionManager::GetFolders())
			{
				for (const CrossSection& cs : folder.GetCrossSections())
				{
					ImGui::PushID(i++);
					cs.Plot(showMarkers, plotAsHist);
					ImGui::PopID();
				}
			}

			if (showBoltzmannConvolutionWindow)
			{
				BoltzmannDistribution::Plot(showMarkers);
			}

			ImPlot::EndPlot();
		}
	}

	void ShowPlasmaRateWindow()
	{
		if (!showPlasmaRateWindow)
		{
			return;
		}
		if (ImGui::Begin("plasma rate window", &showPlasmaRateWindow, ImGuiWindowFlags_NoDocking))
		{
			if (ImPlot::BeginPlot("plasma rates"))
			{
				ImPlot::SetupAxis(ImAxis_X1, "Temperature [K]");
				ImPlot::SetupAxis(ImAxis_Y1, "plasma rate");
				if (logX) ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
				if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
				ImPlot::SetupLegend(ImPlotLocation_NorthEast);
				int i = 0;
				for (PlasmaRateCoefficientFolder& folder : PlasmaRateCoefficientManager::GetFolders())
				{
					for (const PlasmaRateCoefficient& prc : folder.GetPlasmaRateCoefficients())
					{
						ImGui::PushID(i++);
						prc.Plot(showMarkers);
						ImGui::PopID();
					}
				}
				

				ImPlot::EndPlot();
			}

			PlasmaRateCoefficient::ShowConvolutionParamterInputs();
		}
		ImGui::End();
	}
}

