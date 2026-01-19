#include "pch.h"

#include "IonBeam.h"
#include "ElectronBeam.h"
#include "MCMC.h"
#include "EnergyDistribution.h"
#include "Constants.h"

namespace IonBeam
{
	static IonBeamParameter parameters;

	// 3D Hist with main data
	static HistData3D histData;

	//static bool limitZRange = false;
	//static float limitedZRange[2] = { -0.4f, 0.4f };

	static float SliceZ = 0.0f;

	IonBeamParameter GetParameters()
	{
		return parameters;
	}

	void SetParameters(const IonBeamParameter& params)
	{
		parameters = params;
	}

	void Init()
	{
		histData = HistData3D(new TH3D("ion beam", "ion beam", 100, -0.04, 0.04, 100, -0.04, 0.04, 200, -0.7, 0.7));
		UpdateHistData();
	}

	void CreateFromReference(TH3D* reference)
	{
		if (!reference)
		{
			std::cout << "ion beam reference is null" << std::endl;
			return;
		}
		histData = HistData3D((TH3D*)reference->Clone("ion density"));
		histData.GetHist()->SetTitle("ion density");
		
		UpdateHistData();
	}

	void UpdateHistData()
	{
		TH3D* beam = histData.GetHist();

		if (!beam) 
			return;

		beam->Reset();
		int nXBins = beam->GetXaxis()->GetNbins();
		int nYBins = beam->GetYaxis()->GetNbins();
		int nZBins = beam->GetZaxis()->GetNbins();

		for (int i = 1; i <= nXBins; i++) 
		{
			for (int j = 1; j <= nYBins; j++)
			{
				for (int k = 1; k <= nZBins; k++) 
				{
					// Calculate the coordinates for this bin
					double x = beam->GetXaxis()->GetBinCenter(i);
					double y = beam->GetYaxis()->GetBinCenter(j);
					double z = beam->GetZaxis()->GetBinCenter(k);

					double value = GetValue(x, y, z);

					beam->SetBinContent(i, j, k, value);
				}
			}
		}
		histData.UpdateSlice(SliceZ);
	}

	void ShowWindow()
	{
		if (ImGui::Begin("Ion Beam"))
		{
			if (ImGui::BeginChild("settings", ImVec2(300, -1), ImGuiChildFlags_ResizeX))
			{
				ShowParameterControls();
				ImGui::SeparatorText("cooling force specific options");
				ShowCoolingForceParameterControls();
				ImGui::Separator();

				if (ImGui::SliderFloat("slice z", &SliceZ, -0.7f, 0.7f))
				{
					histData.UpdateSlice(SliceZ);
				}

			}
			ImGui::EndChild();
			ImGui::SameLine();
			ShowPlots();

		}
		ImGui::End();
	}

	void ShowPlots()
	{
		if (ImPlot::BeginSubplots("##ion beam subplots", 2, 3, ImVec2(-1, -1), ImPlotSubplotFlags_ShareItems))
		{
			if (ImPlot::BeginPlot("Projection X"))
			{
				histData.PlotProjectionX(0);
				ImPlot::EndPlot();
			}

			if (ImPlot::BeginPlot("Projection Y"))
			{
				histData.PlotProjectionY(0);
				ImPlot::EndPlot();
			}

			if (ImPlot::BeginPlot("Projection Z"))
			{
				histData.PlotProjectionZ(0);
				ImPlot::EndPlot();
			}
			histData.PlotSlice(0);

			ImPlot::EndSubplots();
		}
	}

	void ShowParameterControls()
	{
		if (parameters.ShowControls())
		{
			UpdateHistData();
		}
	}

	void ShowCoolingForceParameterControls()
	{
		//ImGui::BeginGroup();
		//ImGui::PushItemWidth(170.0f);
		//ImGui::BeginDisabled(!limitZRange);
		//ImGui::InputFloat2("##limited range", limitedZRange);
		//ImGui::EndDisabled();
		//ImGui::SameLine();
		//ImGui::Checkbox("limit z range", &limitZRange);
		//ImGui::PopItemWidth();
		//ImGui::EndGroup();
	}

	TVector3 GetDirection()
	{
		float angleX = parameters.angles[0];
		float angleY = parameters.angles[1];

		return TVector3(angleX, angleY, 1).Unit();
	}

	TVector3 GetVelocity()
	{
		return GetDirection() * GetVelocityMagnitude();
	}

	double GetValue(double x, double y, double z)
	{
		if (parameters.sigma[0] == 0 || parameters.sigma[1] == 0)
		{
			return 0;
		}

		// apply shift of ion beam
		x -= parameters.shift[0];
		y -= parameters.shift[1];

		// apply the angles with small angle approximation
		x -= parameters.angles[0] * z;
		y -= parameters.angles[1] * z;

		double value = 0.0;

		double normalisation1 = 1 / (parameters.sigma[0] * parameters.sigma[1] * 2 * TMath::Pi());
		double gauss1 = normalisation1 * exp(-0.5 * ((x * x) / pow(parameters.sigma[0], 2) + (y * y) / pow(parameters.sigma[1], 2)));

		if (parameters.doubleGaussian)
		{
			double amplitudeSum = parameters.amplitude + parameters.amplitude2;

			if (parameters.sigma2[0] == 0 || parameters.sigma2[1] == 0 || amplitudeSum == 0)
			{
				return 0;
			}

			double normalisation2 = 1.0 / (2 * TMath::Pi() * parameters.sigma2[0] * parameters.sigma2[1]);
			double gauss2 = normalisation2 * exp(-0.5 * ((x * x) / pow(parameters.sigma2[0], 2) + (y * y) / pow(parameters.sigma2[1], 2)));

			value = (parameters.amplitude * gauss1 + parameters.amplitude2 * gauss2) / amplitudeSum;
		}
		else
		{
			value = gauss1;
		}

		return value;
	}

	double GetVelocityMagnitude()
	{
		return TMath::Sqrt(2 * ElectronBeam::GetCoolingEnergy() * TMath::Qe() / PhysicalConstants::electronMass);
	}

	float GetSigmaX()
	{
		return parameters.sigma[0];
	}

	float GetSigmaY()
	{
		return parameters.sigma[1];
	}

	TH3D* Get()
	{
		return histData.GetHist();
	}

	std::string GetTags()
	{
		std::string tags = "";
		if (parameters.doubleGaussian) tags += "ion-2gaus, ";
		//if (limitZRange) tags += Form("z samples %.3f - %.3f, ", limitedZRange[0], limitedZRange[1]);
		
		return tags;
	}
}


