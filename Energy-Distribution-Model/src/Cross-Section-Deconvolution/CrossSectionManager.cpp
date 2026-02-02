#include "pch.h"
#include "CrossSectionManager.h"
#include "FileUtils.h"
#include "ImGuiUtils.h"
#include "DeconvolutionWindow.h"
#include "RateCoefficientManager.h"
#include "CrossSection.h"
#include "RateCoefficient.h"

CrossSectionFolder& CrossSectionManager::AddCrossSectionFolder(CrossSectionFolder folder)
{
	s_folderNameList.push_back(folder.m_foldername);
	s_selectedFolderIndex = s_folderNameList.size() - 1;

	s_crossSectionFolders.emplace_back(std::move(folder));
	s_currentCrossSectionFolderIndex = s_crossSectionFolders.size() - 1;

	ResetAllCrossSectionIndeces();

	return s_crossSectionFolders.back();
}

void CrossSectionManager::RemoveCrossSectionFolder(int index)
{
	s_folderNameList.erase(s_folderNameList.begin() + index);

	s_crossSectionFolders.erase(s_crossSectionFolders.begin() + index);
	s_currentCrossSectionFolderIndex = std::min(s_currentCrossSectionFolderIndex, (int)s_crossSectionFolders.size() - 1);

	s_selectedFolderIndex = std::min(s_selectedFolderIndex, (int)s_folderNameList.size() - 1);
}

std::vector<CrossSectionFolder>& CrossSectionManager::GetFolders()
{
	return s_crossSectionFolders;
}

bool CrossSectionManager::IsFolderSelected()
{
	return s_currentCrossSectionFolderIndex > -1;
}

std::vector<std::reference_wrapper<CrossSection>> CrossSectionManager::GetSelectedCrossSections()
{
	std::vector<std::reference_wrapper<CrossSection>> result;

	// if a folder is selected return references to all contained cs
	if (s_currentCrossSectionFolderIndex > -1)
	{
		for (CrossSection& cs : s_crossSectionFolders.at(s_currentCrossSectionFolderIndex).m_crossSections)
		{
			result.emplace_back(cs);
		}
		return result;
	}

	// else return a reference to the single cs that is selected
	for (CrossSectionFolder& folder : s_crossSectionFolders)
	{
		if (folder.m_currentCrossSectionIndex > -1)
		{
			result.emplace_back(folder.m_crossSections.at(folder.m_currentCrossSectionIndex));
			return result;
		}
	}

	return result;
}

std::string CrossSectionManager::GetNameOfSelectedCrossSection()
{
	if (s_currentCrossSectionFolderIndex > -1)
	{
		return s_crossSectionFolders.at(s_currentCrossSectionFolderIndex).m_foldername;
	}
	else
	{
		for (CrossSectionFolder& folder : s_crossSectionFolders)
		{
			if (folder.m_currentCrossSectionIndex > -1)
			{
				return folder.m_crossSections.at(folder.m_currentCrossSectionIndex).GetLabel();
			}
		}
		return "no selected cs";
	}
}

CrossSectionFolder* CrossSectionManager::GetFolderByName(std::string& name)
{
	auto it = std::find_if(s_crossSectionFolders.begin(), s_crossSectionFolders.end(),
		[&](CrossSectionFolder& folder)
		{
			return folder.GetName() == name;
		});

	if (it == s_crossSectionFolders.end())
		return nullptr;

	return &(*it);
}

void CrossSectionManager::ShowFolderList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("cross section folders", ImVec2(100, 100), flags))
	{
		ImGui::Text("cross section folders");
		if (ImGui::BeginListBox("##cslist", ImVec2(-1, 200)))
		{
			ImGui::PushStyleColor(ImGuiCol_Header, s_inputTextColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, s_hoveredTextColor);

			for (size_t i = 0; i < s_crossSectionFolders.size(); i++)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen;

				CrossSectionFolder& folder = s_crossSectionFolders.at(i);
				if (s_currentCrossSectionFolderIndex == i)
				{
					node_flags |= ImGuiTreeNodeFlags_Selected;
				}
				ImGui::PushID(i);
				bool nodeOpen = ImGui::TreeNodeEx(folder.m_foldername.c_str(), node_flags);
				if (ImGui::IsItemClicked())
				{
					s_currentCrossSectionFolderIndex = i;
					ResetAllCrossSectionIndeces();
				}
				if (ImGui::BeginPopupContextItem("plot all/none cs"))
				{
					if (ImGui::Button("plot all"))
					{
						folder.SetAllPlotted(true);
					}
					if (ImGui::Button("plot none"))
					{
						folder.SetAllPlotted(false);
					}

					ImGui::EndPopup();
				}
				ImGui::SameLine();
				ImGui::BeginDisabled(folder.GetCrossSections().empty());
				if (ImGui::SmallButton("-> plasma rate"))
				{
					// TODO
				}
				ImGui::EndDisabled();

				ImGui::SameLine();
				if (ImGui::SmallButton("x"))
				{
					RemoveCrossSectionFolder(i);
				}

				if (nodeOpen)
				{
					folder.ShowSelectablesOfCrossSections();
					ImGui::TreePop();
				}
				ImGui::PopID();
				ImGui::Separator();
			}
			ImGui::PopStyleColor(2);
			ImGui::EndListBox();
		}
		ShowLoadButtons();

		//ShowMissingDataPopup();
	}
	ImGui::EndChild();
}

void CrossSectionManager::ShowLoadButtons()
{

	if (ImGui::Button("load cs"))
	{
		std::vector<std::filesystem::path> files = FileUtils::SelectFiles(FileUtils::GetCrossSectionFolder(), { "*.dat" });
		if (!files.empty())
		{
			std::string foldername = files.at(0).parent_path().filename().string();

			CrossSectionFolder* folder = CrossSectionManager::GetFolderByName(foldername);
			if (!folder)
			{
				folder = &AddCrossSectionFolder(CrossSectionFolder(foldername));
			}

			for (std::filesystem::path& file : files)
			{
				CrossSection cs(file.filename().string());
				cs.Load(file);
				folder->AddCrossSection(cs);
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("load cs folder"))
	{
		std::filesystem::path folder = FileUtils::SelectFolder(FileUtils::GetCrossSectionFolder());
		if (folder.empty())
		{
			std::cout << "cancel loading folder" << std::endl;
		}
		else if (IsFolderInList(folder.filename().string()))
		{
			std::cout << "folder already loaded" << std::endl;
			// TODO make popup/modal window
		}
		else
		{
			CrossSectionFolder newFolder(folder.filename().string());
			newFolder.Load(folder);
			AddCrossSectionFolder(std::move(newFolder));
		}

	}
	ImGui::SameLine();
	if (ImGui::Button("new cs folder"))
	{
		// dont allow duplicate folder names
		if (IsFolderInList(s_newFolderNameInput))
		{
			std::cout << "foldername already exists" << std::endl;
			// TODO make popup/modal window
		}
		else
		{
			CrossSectionFolder newFolder(s_newFolderNameInput);
			AddCrossSectionFolder(std::move(newFolder));
		}

	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGuiUtils::InputText("new folder name", &s_newFolderNameInput);

}

void CrossSectionManager::ShowDeconvolutionControls()
{
	ImGui::SeparatorText("input");
	ImGui::Text("energy distribution set: "); ImGui::SameLine();
	ImGui::TextColored(s_inputTextColor, "%s", EnergyDistributionWindow::GetCurrentSet().GetLabel().c_str());

	ImGui::Text("target rate coefficient: "); ImGui::SameLine();
	ImGui::TextColored(s_inputTextColor, "%s", RateCoefficientManager::GetNameOfSelectedRateCoefficient().c_str());

	ImGui::Checkbox("show binning settings", &s_showBinningSettingsWindow);
	ImGui::SameLine();
	ImGui::Checkbox("show Fit settings", &s_showFitSettingsWindow);

	ImGui::SeparatorText("output");

	ImGui::BeginDisabled(s_selectedFolderIndex < 0 || s_newCrossSectionNameInput.empty());
	if (ImGui::Button("Deconvolve Cross Section"))
	{
		OnButtonDeconvolve();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();

	const char* preview = s_selectedFolderIndex >= 0 ? s_folderNameList[s_selectedFolderIndex].c_str() : "-";
	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::BeginCombo("folder", preview))
	{
		for (int i = 0; i < s_folderNameList.size(); ++i)
		{
			bool isSelected = (s_selectedFolderIndex == i);
			if (ImGui::Selectable(s_folderNameList[i].c_str(), isSelected))
			{
				s_selectedFolderIndex = i;
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150.0f);
	ImGui::BeginDisabled(RateCoefficientManager::IsFolderSelected());
	ImGuiUtils::InputText("filename", &s_newCrossSectionNameInput);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGuiUtils::InputText("_ext", &s_newCrossSectionNameExtensionInput);

	ImGui::BeginDisabled(s_selectedFolderIndex < 0);
	if (ImGui::Button("create 1/E cs"))
	{
		std::ostringstream oss;
		oss << std::scientific << std::setprecision(2) << s_scale;
		std::string scaleString = oss.str();

		CrossSection cs(scaleString + std::string(" over E cs"));
		cs.FillWithOneOverE(s_scale);

		AddCrossSectionToSelectedFolder(cs);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGui::InputDouble("scale", &s_scale, 0, 0, "%.2e");

	if (ImGui::Button("show initial guess"))
	{
		// TODO
		//if (!rateCoefficientList.empty())
		//{
		//	CrossSection cs;
		//	cs.SetupBinning(binSettings, rateCoefficientList.at(currentRateCoefficientIndex));
		//	cs.SetInitialGuessValues(rateCoefficientList.at(currentRateCoefficientIndex));
		//	cs.SetLabel("initial guess");
		//	AddCrossSectionToList(cs);
		//}
		//else
		//{
		//	ImGui::OpenPopup("missing data");
		//	std::cout << "no rate coefficient selected\n";
		//}
	}

	ShowSizeMismatchPopup();
	ShowMissingDataPopup();
}

void CrossSectionManager::ShowFittinAndBinningSettings()
{
	s_binSettings.ShowWindow(s_showBinningSettingsWindow);
	s_fitSettings.ShowWindow(s_showFitSettingsWindow);
}

void CrossSectionManager::AddCrossSectionToSelectedFolder(CrossSection& cs)
{
	CrossSectionFolder& SelectedCsFolder = s_crossSectionFolders.at(s_selectedFolderIndex);
	cs.Save(SelectedCsFolder.m_foldername);
	SelectedCsFolder.AddCrossSection(cs);
}

void CrossSectionManager::ResetAllCrossSectionIndeces()
{
	for (CrossSectionFolder& folder : s_crossSectionFolders)
	{
		folder.m_currentCrossSectionIndex = -1;
	}
}

bool CrossSectionManager::IsFolderInList(const std::string& foldername)
{
	return std::find(s_folderNameList.begin(), s_folderNameList.end(), foldername) != s_folderNameList.end();
}


void CrossSectionManager::ShowSizeMismatchPopup()
{
	// TODO not ideal to just check first rc needs improvement
	if (!RateCoefficientManager::GetSelectedRateCoefficients().empty())
	{
		EnergyDistributionSet& currentSet = EnergyDistributionWindow::GetCurrentSet();
		RateCoefficient& currentRC = RateCoefficientManager::GetSelectedRateCoefficients().at(0);

		ImGuiUtils::ErrorPopup("size mismatch",
			"Sizes of Rate Coefficients and Energy Distributions dont match:\n"
			"Rate Coefficients:\t\t" + std::to_string(currentRC.GetSize()) +
			"\nEnergy distributions:\t" + std::to_string(currentSet.GetSize()));
	}

	
}

void CrossSectionManager::ShowMissingDataPopup()
{
	ImGuiUtils::ErrorPopup("missing data",
		"no energy distribution set, rate coefficient\n"
		"or cross section was selected");
}

void CrossSectionManager::OnButtonDeconvolve()
{
	std::vector<EnergyDistributionSet>& setList = EnergyDistributionWindow::GetSetList();
	auto selectedRateCoefficients = RateCoefficientManager::GetSelectedRateCoefficients();

	if (setList.empty() || selectedRateCoefficients.empty())
	{
		ImGui::OpenPopup("missing data");
		std::cout << "no energy distribution set or rate coefficient(s) selected\n";
		return;
	}

	EnergyDistributionSet& currentSet = EnergyDistributionWindow::GetCurrentSet();

	for (auto rcRef : selectedRateCoefficients)
	{
		RateCoefficient& rc = rcRef.get();
		if (rc.GetSize() != currentSet.GetSize())
		{
			ImGui::OpenPopup("size mismatch");
			std::cout << "sizes of rate coefficients and energy distributions dont match: " +
				std::to_string(rc.GetSize()) + " != " + std::to_string(currentSet.GetSize()) << std::endl;
			return;
		}
		std::string newName;
		if (RateCoefficientManager::IsFolderSelected())
		{
			newName = rc.GetLabel() + "_" + s_newCrossSectionNameExtensionInput;
		}
		else
		{
			newName = s_newCrossSectionNameInput + "_" + s_newCrossSectionNameExtensionInput;
		}
		CrossSection cs(newName);
		cs.Deconvolve(rc, currentSet, s_fitSettings, s_binSettings);

		AddCrossSectionToSelectedFolder(cs);
	}
}


void FittingOptions::ShowWindow(bool& show)
{
	if (!show)
	{
		return;
	}
	if (ImGui::Begin("Cross Section Fit settings", &show, ImGuiWindowFlags_NoDocking))
	{
		ImGui::Checkbox("ROOT fitting", &ROOT_fit);
		ImGui::SameLine();
		ImGui::Checkbox("SVD fitting", &SVD_fit);
		ImGui::Checkbox("Eigen NNLS fitting", &EigenNNLS_fit);
		ImGui::Checkbox("NNLS ROOT combo", &NNLS_ROOT_fit);

		ImGui::InputInt("iterations", &fit_iterations);
		ImGui::InputInt("max iterations", &maxIterations);
		ImGui::InputDouble("tolerance", &tolerance, 0.0, 0.0, "%.1e");
		//ImGui::InputDouble("learning rate", &learningRate);
		ImGui::Checkbox("fix params", &fixParameters);
		ImGui::InputFloat2("fixed parameter range", fixedParameterRange, "%.4f");
		ImGui::InputInt("error iterations", &errorIterations);
	}
	ImGui::End();
}

void CrossSectionBinningSettings::ShowWindow(bool& show)
{
	if (!show)
	{
		return;
	}
	if (ImGui::Begin("Cross Section Binning settings", &show, ImGuiWindowFlags_NoDocking))
	{
		ImGui::SetNextItemWidth(150.0f);
		ImGui::Combo("binning options", (int*)&scheme, binningOptions, IM_ARRAYSIZE(binningOptions));
		ImGui::PushItemWidth(100.0f);
		ImGui::InputDouble("bin factor", &binFactor, 0.0, 0.0, "%.4f");
		if (scheme == FactorBinning || scheme == PaperFactorMix)
		{
			ImGui::InputInt("number bins", &numberBins);
		}
		if (scheme == PaperFactorMix)
		{
			ImGui::InputDouble("boundary energy", &boundaryEnergy, 0.0, 0.0, "%.4f");
		}
		if (scheme == PaperBinning)
		{
			ImGui::InputInt("max ratio", &maxRatio);
		}
		ImGui::PopItemWidth();
	}
	ImGui::End();
}

