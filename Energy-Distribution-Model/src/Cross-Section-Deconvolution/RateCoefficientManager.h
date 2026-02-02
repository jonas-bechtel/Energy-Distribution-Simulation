#pragma once
#include "RateCoefficient.h"

class RateCoefficientManager
{
public:
	RateCoefficientManager() = delete;

	static RateCoefficientFolder& AddRateCoefficientFolder(RateCoefficientFolder folder);
	static void RemoveRateCoefficientFolder(int index);
	static std::vector<RateCoefficientFolder>& GetFolders();
	static bool IsFolderSelected();
	static std::vector<std::reference_wrapper<RateCoefficient>> GetSelectedRateCoefficients();
	static std::string GetNameOfSelectedRateCoefficient();
	static RateCoefficientFolder* GetFolderByName(std::string& name);
	static void ShowFolderList();
	static void ShowLoadButtons();
	static void ShowConvolutionControls();

private:
	static void AddRateCoefficientToSelectedFolder(RateCoefficient& cs);
	static void ResetAllRateCoefficientIndeces();
	static bool IsFolderInList(std::string& foldername);

	//static void ShowSizeMismatchPopup();
	//static void ShowMissingDataPopup();

	static void OnButtonConvolve();

private:
	static inline std::vector<RateCoefficientFolder> s_rateCoefficientFolders;
	static inline int s_currentRateCoefficientFolderIndex = -1;

	static inline std::string s_newFolderNameInput = "";
	static inline std::vector<std::string> s_folderNameList;
	static inline int s_selectedFolderIndex = -1;

	static inline std::string s_newRateCoefficientNameInput = "test";
	static inline std::string s_newRateCoefficientNameExtensionInput = "ext";

	// TODO remove and put in general place
	static inline const ImVec4 s_inputTextColor = ImVec4(0.6f, 0.2f, 0.1f, 1.0f);
	static inline const ImVec4 s_hoveredTextColor = ImVec4(0.7f, 0.3f, 0.15, 1.0f);

	friend class RateCoefficientFolder;
};

