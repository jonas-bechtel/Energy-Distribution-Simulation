#pragma once
class CrossSection;

class PlasmaRateCoefficient
{
public:
	PlasmaRateCoefficient(std::string name);

	std::string GetLabel();

	void Convolve(const CrossSection& cs);
	void ConvolveFromErrorIterationArray(const CrossSection& cs);

	bool IsPlotted() const { return plotted; }
	void SetPlotted(bool plot) { plotted = plot; }
	void Plot(bool showMarkers) const;

	void Save(std::string foldername) const;
	void Load(std::filesystem::path file);

	static void ShowConvolutionParamterInputs();

private:
	std::vector<double> temperatures;
	std::vector<double> values;
	std::vector<double> errors;

	std::string label = "plasma rate";
	std::filesystem::path crossSectionFile;

	bool plotted = false;

	static inline int numberValues = 1000;
	static inline double startTemperature = 10.0; 
	static inline double endTemperature = 50000.0;
};


class PlasmaRateCoefficientFolder
{
public:
	PlasmaRateCoefficientFolder(std::string foldername);
	PlasmaRateCoefficientFolder(const PlasmaRateCoefficientFolder& other) = delete;
	PlasmaRateCoefficientFolder& operator=(const PlasmaRateCoefficientFolder& other) = delete;

	PlasmaRateCoefficientFolder(PlasmaRateCoefficientFolder&& other) = default;
	PlasmaRateCoefficientFolder& operator=(PlasmaRateCoefficientFolder&& other) = default;

	void AddPlasmaRateCoefficient(PlasmaRateCoefficient& cs);
	void RemovePlasmaRateCoefficient(int index);
	std::vector<PlasmaRateCoefficient>& GetPlasmaRateCoefficients();
	std::string GetName() { return m_foldername; }

	void ShowSelectablesOfPlasmaRateCoefficients();

	void Load(const std::filesystem::path& folder);

private:
	void SetAllPlotted(bool plot);

private:
	std::vector<PlasmaRateCoefficient> m_plasmaRateCoefficients;
	int m_currentPlasmaRateCoefficientIndex = -1;
	std::string m_foldername;

	friend class PlasmaRateCoefficientManager;
};