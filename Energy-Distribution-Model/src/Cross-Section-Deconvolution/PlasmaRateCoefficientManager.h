#pragma once
#include "PlasmaRateCoefficient.h"

class PlasmaRateCoefficientManager
{
public:
	PlasmaRateCoefficientManager() = delete;

	static void AddPlasmaRateCoefficientToFolder(PlasmaRateCoefficient prc, std::string& folderName);
	static PlasmaRateCoefficientFolder& AddPlasmaRateCoefficientFolder(PlasmaRateCoefficientFolder folder);
	static void RemovePlasmaRateCoefficientFolder(int index);
	static std::vector<PlasmaRateCoefficientFolder>& GetFolders();
	//static bool IsFolderSelected();
	//static std::vector<std::reference_wrapper<PlasmaRateCoefficient>> GetSelectedPlasmaRateCoefficients();
	//static std::string GetNameOfSelectedPlasmaRateCoefficient();
	static PlasmaRateCoefficientFolder* GetFolderByName(std::string& name);
	static void ShowFolderList();

private:
	static void ResetAllPlasmaRateCoefficientIndeces();
	static bool IsFolderInList(std::string& foldername);

	static void ShowLoadButtons();

private:
	static inline std::vector<PlasmaRateCoefficientFolder> s_plasmaRateCoefficientFolders;
	static inline int s_currentPlasmaRateCoefficientFolderIndex = -1;

	//static inline std::string s_newFolderNameInput = "";
	static inline std::vector<std::string> s_folderNameList;
	static inline int s_selectedFolderIndex = -1;

	//static inline std::string s_newPlasmaRateCoefficientNameInput = "test";
	//static inline std::string s_newPlasmaRateCoefficientNameExtensionInput = "ext";

	// TODO remove and put in general place
	//static inline const ImVec4 s_inputTextColor = ImVec4(0.6f, 0.2f, 0.1f, 1.0f);
	//static inline const ImVec4 s_hoveredTextColor = ImVec4(0.7f, 0.3f, 0.15, 1.0f);

	friend class PlasmaRateCoefficientFolder;
};

