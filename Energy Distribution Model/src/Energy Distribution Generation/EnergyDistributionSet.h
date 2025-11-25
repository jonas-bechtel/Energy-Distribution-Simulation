#pragma once
#include "EnergyDistribution.h"

struct SetInformation
{
	std::vector<int> indeces;
	std::vector<double> centerLabEnergy;
	std::vector<double> detuningEnergy;
	std::vector<double> fitDetuningEnergy;
	std::vector<double> fitLongitudinalTemperature;
	std::vector<double> fitTransverseTemperature;
	std::vector<double> fitScalingFactor;
	std::vector<double> fitFWHM;
	std::vector<double> FWHM;
	std::vector<double> mainPeakPositions;
	std::vector<double> distanceLeftFWHM;
	std::vector<double> distanceRightFWHM;

	bool plot = false;

public:
	void AddDistributionValues(const EnergyDistribution& dist);
	void RemoveDistributionValues(int index);

	void PlotFitEd(std::string setLabel);
	void PlotFitlongkT(std::string setLabel);
	void PlotFitTranskT(std::string setLabel);
	void PlotFitScalingFactor(std::string setLabel);
	void PlotFitFWHM(std::string setLabel);
	void PlotFWHM(std::string setLabel);
	void PlotMainPeakPosition(std::string setLabel);
	void PlotDistanceLeftFWHM(std::string setLabel);
	void PlotDistanceRightFWHM(std::string setLabel);

	void Save(std::filesystem::path folder);
};

class EnergyDistributionSet
{
public:
	EnergyDistributionSet();
	EnergyDistributionSet(const EnergyDistributionSet& other) = delete;
	EnergyDistributionSet& operator=(const EnergyDistributionSet& other) = delete;

	EnergyDistributionSet(EnergyDistributionSet&& other) = default;
	EnergyDistributionSet& operator=(EnergyDistributionSet&& other) = default;

	void AddDistribution(EnergyDistribution&& distribution);
	void RemoveDistribution(int index);

	std::string GetLabel();
	EnergyDistribution* FindByEd(double detuningEnergy) const;

	void SetFolder(std::filesystem::path path);
	void SetSubfolder(std::filesystem::path path);
	void SetAllPlotted(bool plotted);
	void SetAllShowNormalised(bool showNormalised);

	std::filesystem::path GetFolder();
	std::filesystem::path GetSubfolder();
	const std::vector<EnergyDistribution>& GetDistributions() const;
	SetInformation& GetInfo();
	int GetSize() const { return distributions.size(); }

	void CalculatePsisFromBinning(TH1D* crossSection);

	void ShowList();

	void SaveSamples() const;
	void SaveHists() const;
	void Load(std::filesystem::path& folder);

private:
	std::vector<EnergyDistribution> distributions;
	SetInformation info;

	std::unordered_map<double, EnergyDistribution*> EdToDistMap;

	std::filesystem::path folder = "Test";
	std::filesystem::path subFolder = "subfolder";

	friend class CrossSection;
	friend class RateCoefficient;
};

