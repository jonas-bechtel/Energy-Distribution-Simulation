#pragma once

class EnergyDistributionSet;
class EnergyDistribution;
class RateCoefficient;
class CrossSectionBinningSettings;
class FittingOptions;

class CrossSection
{
public:
	CrossSection(std::string name);
	CrossSection(const CrossSection& other) = delete;
	CrossSection& operator=(const CrossSection& other) = delete;
	CrossSection(CrossSection&& other) = default;
	CrossSection& operator=(CrossSection&& other) = default;

	TH1D* GetHist();
	std::string GetLabel() const;

	void SetLabel(std::string label);
	void FillWithOneOverE(double scale = 1.0);
	void SetupBinning(const CrossSectionBinningSettings& binSettings, const RateCoefficient& rc);
	void SetInitialGuessValues(const RateCoefficient& rc);
	void Deconvolve(RateCoefficient& rc, EnergyDistributionSet& set, const FittingOptions& fitSettings, const CrossSectionBinningSettings& binSettings);

	void Plot(bool showMarkers, bool plotAsHist) const;
	bool IsPlotted() const;
	void SetPlotted(bool plot);

	void Clear();
	void Save(std::string foldername) const;
	void Load(const std::filesystem::path& filename);

private:
	double ConvolveFit(double Ed, double* csBins, const EnergyDistributionSet& set,
		bool squareCS = true,
		std::unordered_map<double, EnergyDistribution*>* map = nullptr) const;
	void FitWithSVD(const RateCoefficient& rc, const EnergyDistributionSet& set);
	void FitWithROOT(const RateCoefficient& rc, const EnergyDistributionSet& set, const FittingOptions& fitSettings);
	void FitWithEigenNNLS(const RateCoefficient& rc, const EnergyDistributionSet& set, const FittingOptions& fitSettings);

	void ResetNonFixedParameters(const RateCoefficient& rc, const FittingOptions& fitSettings);
private:
	// main data
	TH1D* hist = nullptr;

	std::vector<double> energies;
	std::vector<double> values;
	std::vector<double> errors;
	std::vector<double> binEdges;

	// array with values from error iterations
	std::vector<double> valueArray;

	bool plotted = false;

	// labelling things
	std::string label = "cs";
	std::filesystem::path energyDistriubtionSetFolder;
	std::filesystem::path mergedBeamRateCoefficientFile;

	friend class RateCoefficient;
	friend class PlasmaRateCoefficient ;
};

class CrossSectionFolder
{
public:
	CrossSectionFolder(std::string foldername);
	CrossSectionFolder(const CrossSectionFolder& other) = delete;
	CrossSectionFolder& operator=(const CrossSectionFolder& other) = delete;

	CrossSectionFolder(CrossSectionFolder&& other) = default;
	CrossSectionFolder& operator=(CrossSectionFolder&& other) = default;

	void AddCrossSection(CrossSection& cs);
	void RemoveCrossSection(int index);
	std::vector<CrossSection>& GetCrossSections();

	std::string GetName() { return m_foldername; }

	void ShowSelectablesOfCrossSections();

	void Load(const std::filesystem::path& folder);

private:
	void SetAllPlotted(bool plot);

private:
	std::vector<CrossSection> m_crossSections;
	int m_currentCrossSectionIndex = -1;
	std::string m_foldername;

	friend class CrossSectionManager;
};