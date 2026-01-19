#include "pch.h"

#include "MCMC.h"
#include "ElectronBeam.h"
#include "IonBeam.h"
#include "HistUtils.h"
#include "ImGuiUtils.h"

namespace MCMC
{
	static MCMC_Parameter parameters;

	// 3D Hist with main data
	static TH3D* distribution = nullptr;
	static TH3D* target = nullptr;

	static std::vector<Point3D> chain;
	static float fullZRange[2] = { -0.7f, 0.7f };

	// optional parameter
	//static bool limitZRange = false;
	//static float limitedZRange[2] = { -0.4f, 0.4f };

	// plotting data
	static ROOTCanvas* canvas = nullptr;
	static TCanvas* canvas2 = nullptr;

	static std::vector<HistData3D> plotTargetBeams;
	static std::vector<HistData3D> plotResultBeams;
	static int selectedIndex = -1;
	static float SliceZ = 0.0f;

	// result values
	static float acceptanceRate = 0.0;
	static double totalTime = 0.0;
	static double interpolationTime = 0;

	// threading stuff
	static unsigned const int numThreads = std::thread::hardware_concurrency();
	static std::vector<std::future<float>> futures;

	// RNG stuff
	//std::default_random_engine generator
	static std::vector<RNG_engine> generatorList;
	//std::subtract_with_carry_engine< std::uint_fast64_t, 48, 5, 12> generator; // is slightly faster but maybe less quality
	//std::linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647> generator; // is even faster
	static std::uniform_real_distribution<double> uniformDist = std::uniform_real_distribution<double>(0.0, 1.0);
	static std::normal_distribution<double> normalDistX = std::normal_distribution<double>(0.0, parameters.proposalSigma[0]);
	static std::normal_distribution<double> normalDistY = std::normal_distribution<double>(0.0, parameters.proposalSigma[1]);
	static std::normal_distribution<double> normalDistZ = std::normal_distribution<double>(0.0, parameters.proposalSigma[2]);

	// Autocorrelation stuff
	static bool showAutoCorrelationWindow = false;

	static double means[3] = { 0, 0, 0 };
	static double variances[3] = { 0, 0, 0 };
	static constexpr int maxLag = 100;

	static std::array<double, maxLag> lagValues;

	static std::array<double, maxLag> autocorrX;
	static std::array<double, maxLag> autocorrY;
	static std::array<double, maxLag> autocorrZ;


	void Init()
	{
		canvas = new ROOTCanvas("MCMC canvas", "MCMC canvas", 1200, 500);
		canvas->Divide(2, 1);

		generatorList.resize(numThreads);
		
		for (unsigned int i = 0; i < numThreads; i++)
		{
			generatorList.at(i) = RNG_engine();
		}
	}

	MCMC_Parameter GetParameters()
	{
		return parameters;
	}

	void SetParameters(const MCMC_Parameter& params)
	{
		parameters = params;
	}

	std::vector<Point3D>& GetSamples()
	{
		return chain;
	}

	std::string GetTags()
	{
		std::string tags = "";
		//if(limitZRange) tags += Form("z samples %.3f - %.3f, ", limitedZRange[0], limitedZRange[1]);
	
		return tags;
	}

	void SetTargetDist(TH3D* targetDist)
	{
		delete target;
		target = targetDist;

		if (!target)
		{
			std::cout << "target dist was nullptr" << std::endl;
			return;
		}
		fullZRange[0] = target->GetZaxis()->GetXmin();
		fullZRange[1] = target->GetZaxis()->GetXmax();
	}

	void GenerateSamples()
	{
		if (!target)
		{
			std::cout << "no target histogram was set\n";
			return;
		}

		// use the std of the target distribution as sigmas for proposal functions
		if (parameters.automaticProposalStd)
		{
			parameters.proposalSigma[0] = (float)target->GetStdDev(1);
			parameters.proposalSigma[1] = (float)target->GetStdDev(2);
			parameters.proposalSigma[2] = (float)target->GetStdDev(3);
		}

		normalDistX = std::normal_distribution<double>(0.0, parameters.proposalSigma[0]);
		normalDistY = std::normal_distribution<double>(0.0, parameters.proposalSigma[1]);
		normalDistZ = std::normal_distribution<double>(0.0, parameters.proposalSigma[2]);
		

		delete distribution;
		distribution = (TH3D*)target->Clone("generated Histogram");
		distribution->Reset();
		distribution->SetTitle("generated Histogram");

		chain.clear();
		chain.resize(parameters.numberSamples);
		futures.clear();
		futures.reserve(numThreads);
		interpolationTime = 0;

		if (parameters.changeSeed)
		{
			parameters.seed = ((int)std::time(0));
		}
		generatorList[0].seed(parameters.seed);

		auto t_start = std::chrono::high_resolution_clock::now();

		if (parameters.generateAsync)
		{
			// Launch asynchronous tasks
			for (unsigned int i = 0; i < numThreads; i++)
			{
				int length = parameters.numberSamples / numThreads;
				int offset = i * length;
				RNG_engine& generator = generatorList.at(i);
				generator.seed(parameters.seed + i);

				futures.push_back(std::async(std::launch::async, &GenerateSubchain, length, offset, std::ref(generator)));
			}

			acceptanceRate = 0;

			// Wait for all tasks to complete
			for (auto& future : futures)
			{
				acceptanceRate += future.get();
			}

			acceptanceRate /= numThreads;
		}
		else
		{
			acceptanceRate = GenerateSubchain(parameters.numberSamples, 0, generatorList[0]);
		}

		// Fill graphs
		for (const Point3D& point : chain)
		{
			distribution->Fill(point.x, point.y, point.z);
		}

		auto t_end = std::chrono::high_resolution_clock::now();
		totalTime = std::chrono::duration<double, std::milli>(t_end - t_start).count();
	}

	float GenerateSubchain(int length, int offset, RNG_engine& generator)
	{
		// random start point
		double x = 0; //uniformDist(generator);
		double y = 0; //uniformDist(generator);
		double z = 0.3; //uniformDist(generator);

		Point3D currentPoint(x, y, z);
		double currentValue = target->Interpolate(x, y, z);

		int burnInCounter = 0;
		int lagCounter = 0;
		int acceptedValues = 0;
		int totalIterations = 0;
		int addedValues = 0;

		while (addedValues < length)
		{
			totalIterations++;

			if (GenerateSingleSample(currentPoint, currentValue, generator))
			{
				acceptedValues++;
			};

			if (burnInCounter < parameters.burnIn)
			{
				burnInCounter++;
				continue;
			}

			if (lagCounter == 0)
			{
				chain.at(offset + addedValues) = currentPoint;
				addedValues++;
				lagCounter = parameters.lag;
			}
			else
			{
				lagCounter--;
			}
		}
		return (float)acceptedValues / totalIterations;
	}

	bool GenerateSingleSample(Point3D& current, double& currentValue, RNG_engine& generator)
	{
		// propose new sample
		double x_proposed = current.x + normalDistX(generator);
		double y_proposed = current.y + normalDistY(generator);
		//double z_proposed = current.z + normalDistZ(generator);
		double z_proposed = 0.0;
		GeneralParameter genParams = General::GetParameters();

		if (genParams.limitZRange)
		{
			z_proposed = genParams.limitedZRange[0] + (genParams.limitedZRange[1] - genParams.limitedZRange[0]) * uniformDist(generator);
		}
		else
		{
			z_proposed = fullZRange[0] + (fullZRange[1] - fullZRange[0]) * uniformDist(generator);
		}

		// compute probabilities
		double p_new = HistUtils::GetValueAtPosition(target, { x_proposed,y_proposed, z_proposed }, parameters.useInterpolation);

		// acceptance ratio
		double ratio = p_new / currentValue;

		if (ratio >= 1 || uniformDist(generator) < ratio)
		{
			// Accept the new point
			current.x = x_proposed;
			current.y = y_proposed;
			current.z = z_proposed;
			currentValue = p_new;

			return true;
		}

		return false;
	}

	void SelectedItemChanged()
	{
		HistData3D& newTarget = plotTargetBeams.at(selectedIndex);
		HistData3D& newResult = plotResultBeams.at(selectedIndex);
		newTarget.UpdateSlice(SliceZ);
		newResult.UpdateSlice(SliceZ);

		if (canvas->IsShown())
		{
			newTarget.Plot3D(canvas, 1);
			newResult.Plot3D(canvas, 2);
		}
	}

	void AddMCMCDataToList(HistData3D& target, HistData3D& result)
	{
		plotTargetBeams.push_back(std::move(target));
		plotResultBeams.push_back(std::move(result));

		if (plotTargetBeams.size() == 1)
		{
			selectedIndex = 0;
			SelectedItemChanged();
		}
	}

	void RemoveMCMCDataFromList(int index)
	{
		plotTargetBeams.erase(plotTargetBeams.begin() + index);
		plotResultBeams.erase(plotResultBeams.begin() + index);

		selectedIndex = std::min(selectedIndex, (int)plotTargetBeams.size() - 1);
		if (selectedIndex >= 0)
		{
			SelectedItemChanged();
		}
	}

	void UpdateAutocorrelationData()
	{
		// Compute means and variances
		for (const Point3D& point : chain)
		{
			means[0] += point.x;
			means[1] += point.y;
			means[2] += point.z;
		}
		means[0] /= parameters.numberSamples;
		means[1] /= parameters.numberSamples;
		means[2] /= parameters.numberSamples;

		// Compute variances
		for (const Point3D& point : chain)
		{
			variances[0] += (point.x - means[0]) * (point.x - means[0]);
			variances[1] += (point.y - means[1]) * (point.y - means[1]);
			variances[2] += (point.z - means[2]) * (point.z - means[2]);
		}
		variances[0] /= parameters.numberSamples;
		variances[1] /= parameters.numberSamples;
		variances[2] /= parameters.numberSamples;

		// Compute autocorrelation
		for (int lag = 0; lag < 100; lag++)
		{
			double sumX = 0;
			double sumY = 0;
			double sumZ = 0;

			for (int i = 0; i < parameters.numberSamples - lag; i++)
			{
				Point3D point = chain[i];
				Point3D pointLag = chain[i + lag];
				sumX += (point.x - means[0]) * (pointLag.x - means[0]);
				sumY += (point.y - means[1]) * (pointLag.y - means[1]);
				sumZ += (point.z - means[2]) * (pointLag.z - means[2]);
			}
			autocorrX[lag] = sumX / (variances[0] * (parameters.numberSamples - lag));
			autocorrY[lag] = sumY / (variances[1] * (parameters.numberSamples - lag));
			autocorrZ[lag] = sumZ / (variances[2] * (parameters.numberSamples - lag));

			lagValues[lag] = lag;
		}
	}

	void ShowWindow()
	{
		if (ImGui::Begin("MCMC Window"))
		{
			if (ImGui::BeginChild("left side", ImVec2(300, -1), ImGuiChildFlags_ResizeX))
			{
				ShowList();

				if (ImGui::Button("generate chain") && ElectronBeam::GetSelected())
				{
					//if (!ElectronBeam::GetSelected())
					//	return;

					TH3D* electronBeam = ElectronBeam::GetSelected()->GetHist();

					IonBeam::CreateFromReference(electronBeam);
					TH3D* ionBeam = IonBeam::Get();

					target = (TH3D*)ionBeam->Clone("electron-ion density");
					target->SetTitle("electron-ion density");
					target->Multiply(electronBeam);
					
					GenerateSamples();

					if (distribution && target)
					{
						TH3D* temp1 = (TH3D*)target->Clone("target to look at");
						TH3D* temp2 = (TH3D*)distribution->Clone("generated to look at");

						temp1->SetTitle("target to look at");
						temp2->SetTitle("generated to look at");

						HistData3D targetPlotData(temp1);
						HistData3D resultPlotData(temp2);

						targetPlotData.SetLabel(ElectronBeam::GetSelectedBeamLabel());
						resultPlotData.SetLabel(ElectronBeam::GetSelectedBeamLabel());
						AddMCMCDataToList(targetPlotData, resultPlotData);
					}
					UpdateAutocorrelationData();
				}
				
				ImGui::SetNextItemWidth(200.0f);
				if (ImGui::SliderFloat("slice z", &SliceZ, -0.7f, 0.7f))
				{
					if (selectedIndex >= 0)
					{
						HistData3D& targetbeam = plotTargetBeams.at(selectedIndex);
						HistData3D& resultbeam = plotResultBeams.at(selectedIndex);
						targetbeam.UpdateSlice(SliceZ);
						resultbeam.UpdateSlice(SliceZ);
					}
				}
				
				ImGui::Separator();
				ShowParameterControls();

				ImGui::SeparatorText("output information");
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::LabelText("", "Took %.1f ms total. Interpolation took %.1f ms", totalTime, interpolationTime);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::LabelText("", "Acceptance Rate: %.1f %%", acceptanceRate * 100);

				ImGui::Checkbox("show autocorrelation", &showAutoCorrelationWindow);

				ImGui::Separator();
				canvas->MakeShowHideButton();
				ImGui::SameLine();
				HistData3D::ShowRebinningFactorsInput();
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ShowPlots();
			ShowAutoCorrelationPlots();

		}
		ImGui::End();
		canvas->Render();
	}

	void ShowList()
	{
		if (ImGui::BeginListBox("##mcmc data list", ImVec2(-1, 270.0f)))
		{
			for (int i = 0; i < plotTargetBeams.size(); i++)
			{
				ImGui::PushID(i);
				HistData3D& mcmcData = plotTargetBeams.at(i);

				if (ImGui::Selectable(mcmcData.GetLabel().c_str(), i == selectedIndex, ImGuiSelectableFlags_AllowItemOverlap))
				{
					selectedIndex = i;
					SelectedItemChanged();
				}

				ImGui::SameLine();
				if (ImGui::SmallButton("x"))
				{
					RemoveMCMCDataFromList(i);
				}
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
	}

	void ShowPlots()
	{
		if (ImPlot::BeginSubplots("##mcmc subplots", 2, 3, ImVec2(-1, -1), ImPlotSubplotFlags_ShareItems))
		{
			if (ImPlot::BeginPlot("Projection X"))
			{
				ImPlot::SetupAxes("x", "normalised value");
				for (int i = 0; i < plotTargetBeams.size(); i++)
				{
					plotTargetBeams.at(i).PlotProjectionX(i, ImPlotLineFlags_Segments);
					plotResultBeams.at(i).PlotProjectionX(i);
				}
				ImPlot::EndPlot();
			}

			if (ImPlot::BeginPlot("Projection Y"))
			{
				ImPlot::SetupAxes("y", "normalised value");
				for (int i = 0; i < plotTargetBeams.size(); i++)
				{
					plotTargetBeams.at(i).PlotProjectionY(i, ImPlotLineFlags_Segments);
					plotResultBeams.at(i).PlotProjectionY(i);
				}
				ImPlot::EndPlot();
			}

			if (ImPlot::BeginPlot("Projection Z"))
			{
				ImPlot::SetupAxes("z", "normalised value");
				for (int i = 0; i < plotTargetBeams.size(); i++)
				{
					plotTargetBeams.at(i).PlotProjectionZ(i, ImPlotLineFlags_Segments);
					plotResultBeams.at(i).PlotProjectionZ(i);
				}
				ImPlot::EndPlot();
			}

			if (selectedIndex >= 0)
			{
				plotTargetBeams.at(selectedIndex).PlotSlice(selectedIndex, "Target XY SLice");
				ImGui::SameLine();
				plotResultBeams.at(selectedIndex).PlotSlice(selectedIndex, "Result XY SLice");
			}

			ImPlot::EndSubplots();
		}
	}

	void ShowAutoCorrelationPlots()
	{
		if (showAutoCorrelationWindow && ImGui::Begin("Autocorrelation window", &showAutoCorrelationWindow, ImGuiWindowFlags_NoDocking))
		{
			if (ImPlot::BeginSubplots("##autocorr subplots", 1, 3, ImVec2(-1, -1), ImPlotSubplotFlags_ShareItems))
			{
				if (ImPlot::BeginPlot("Autocorrelation X"))
				{
					ImPlot::PlotLine("##", lagValues.data(), autocorrX.data(), autocorrX.size());
					ImPlot::EndPlot();
				}

				if (ImPlot::BeginPlot("Autocorrelation Y"))
				{
					ImPlot::PlotLine("##", lagValues.data(), autocorrY.data(), autocorrY.size());
					ImPlot::EndPlot();
				}

				if (ImPlot::BeginPlot("Autocorrelation Z"))
				{
					ImPlot::PlotLine("##", lagValues.data(), autocorrZ.data(), autocorrZ.size());
					ImPlot::EndPlot();
				}
				ImPlot::EndSubplots();
			}

			ImGui::End();
		}
	}

	void ShowParameterControls()
	{
		parameters.ShowControls();
	}

}
