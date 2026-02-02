#pragma once
#include "CrossSection.h"

class CrossSection;
class CrossSectionFolder;

enum CrossSectionBinningScheme { PaperBinning, FactorBinning, PaperFactorMix };

struct CrossSectionBinningSettings
{
	const char* binningOptions[3] = { "paper binning", "factor binning", "paper/factor mix" };

	int numberBins = 7;
	int maxRatio = 10;
	double boundaryEnergy = 0.1;
	double binFactor = 1.5;

	CrossSectionBinningScheme scheme = CrossSectionBinningScheme::PaperFactorMix;

	void ShowWindow(bool& show);
};

struct FittingOptions
{
	// fit options
	bool ROOT_fit = true;
	bool SVD_fit = false;
	bool EigenNNLS_fit = false;
	bool NNLS_ROOT_fit = false;

	int errorIterations = 1; // number of iterations to calculate errors
	int fit_iterations = 1;
	int maxIterations = 1000;
	double tolerance = 1e-6;
	double learningRate = 1;
	bool fixParameters = false;
	float fixedParameterRange[2] = { 1,100 };

	void ShowWindow(bool& show);
};


class CrossSectionManager
{
public:
	CrossSectionManager() = delete;

	static CrossSectionFolder& AddCrossSectionFolder(CrossSectionFolder folder);
	static void RemoveCrossSectionFolder(int index);
	static std::vector<CrossSectionFolder>& GetFolders();
	static bool IsFolderSelected();
	static std::vector<std::reference_wrapper<CrossSection>> GetSelectedCrossSections();
	static std::string GetNameOfSelectedCrossSection();
	static CrossSectionFolder* GetFolderByName(std::string& name);
	static void ShowFolderList();
	static void ShowLoadButtons();
	static void ShowDeconvolutionControls();
	static void ShowFittinAndBinningSettings();

private:
	static void AddCrossSectionToSelectedFolder(CrossSection& cs);
	static void ResetAllCrossSectionIndeces();
	static bool IsFolderInList(std::string& foldername);

	static void ShowSizeMismatchPopup();
	static void ShowMissingDataPopup();

	static void OnButtonDeconvolve();

private:
	static inline std::vector<CrossSectionFolder> s_crossSectionFolders;
	static inline int s_currentCrossSectionFolderIndex = -1;

	static inline std::string s_newFolderNameInput = "";
	static inline std::vector<std::string> s_folderNameList;
	static inline int s_selectedFolderIndex = -1;

	static inline std::string s_newCrossSectionNameInput = "test";
	static inline std::string s_newCrossSectionNameExtensionInput = "ext";

	static inline CrossSectionBinningSettings s_binSettings;
	static inline FittingOptions s_fitSettings;
	static inline bool s_showBinningSettingsWindow = false;
	static inline bool s_showFitSettingsWindow = false;

	// TODO remove and put in general place
	static inline const ImVec4 s_inputTextColor = ImVec4(0.6f, 0.2f, 0.1f, 1.0f);
	static inline const ImVec4 s_hoveredTextColor = ImVec4(0.7f, 0.3f, 0.15, 1.0f);

	// scaling factor for generating 1/E CS
	static inline double s_scale = 1.0;

	friend class CrossSectionFolder;
};

