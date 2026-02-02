#pragma once

class CrossSection;
class EnergyDistributionSet;

class RateCoefficient
{
public:
	RateCoefficient(std::string name);
	RateCoefficient(const RateCoefficient& other) = delete;
	RateCoefficient& operator=(const RateCoefficient& other) = delete;
	RateCoefficient(RateCoefficient&& other) = default;
	RateCoefficient& operator=(RateCoefficient&& other) = default;

	void VaryGraphValues();
	void ResetGraphValues();

	//void SetLabel(std::string label);
	std::string GetLabel();
	int GetSize() const { return detuningEnergies.size(); }

	void Convolve(const CrossSection& cs, EnergyDistributionSet& set);

	bool IsPlotted() const { return plotted; }
	void SetPlotted(bool plot) { plotted = plot; }
	void Plot(bool showMarkers) const;
	void PlotSubfunctions() const;

	void Clear();
	void Save(std::string foldername) const;
	void Load(const std::filesystem::path& file);

private:
	int GetIndexOfDetuningEnergy(double Ed) const;
	void SortValuesByDetuningEnergy();

private:
	// main data
	TGraphErrors* graph = new TGraphErrors();

	// ordered in asceding detuning energy
	std::vector<double> detuningEnergies;
	std::vector<double> value;
	std::vector<double> error;
	std::vector<std::vector<double>> psiSubfunctions;

	// labelling things
	std::string label = "mbrc";
	std::filesystem::path energyDistriubtionSetFolder;
	std::filesystem::path crossSectionFile;

	bool plotted = false;

	bool measured = true;

	friend class CrossSection;
};

class RateCoefficientFolder 
{
public:
	RateCoefficientFolder(std::string foldername);
	RateCoefficientFolder(const RateCoefficientFolder& other) = delete;
	RateCoefficientFolder& operator=(const RateCoefficientFolder& other) = delete;

	RateCoefficientFolder(RateCoefficientFolder&& other) = default;
	RateCoefficientFolder& operator=(RateCoefficientFolder&& other) = default;

	void AddRateCoefficient(RateCoefficient& cs);
	void RemoveRateCoefficient(int index);
	std::vector<RateCoefficient>& GetRateCoefficients();
	std::string GetName() { return m_foldername; }

	void ShowSelectablesOfRateCoefficients();

	void Load(const std::filesystem::path& folder);

private:
	void SetAllPlotted(bool plot);

private:
	std::vector<RateCoefficient> m_rateCoefficients;
	int m_currentRateCoefficientIndex = -1;
	std::string m_foldername;

	friend class RateCoefficientManager;
};