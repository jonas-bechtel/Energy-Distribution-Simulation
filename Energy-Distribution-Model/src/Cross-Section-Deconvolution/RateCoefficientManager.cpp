#include "pch.h"
#include "RateCoefficientManager.h"
#include "CrossSectionManager.h"
#include <FileUtils.h>
#include <ImGuiUtils.h>

RateCoefficientFolder& RateCoefficientManager::AddRateCoefficientFolder(RateCoefficientFolder folder)
{
	s_folderNameList.push_back(folder.m_foldername);
	s_selectedFolderIndex = s_folderNameList.size() - 1;

	s_rateCoefficientFolders.emplace_back(std::move(folder));
	s_currentRateCoefficientFolderIndex = s_rateCoefficientFolders.size() - 1;

	ResetAllRateCoefficientIndeces();

	return s_rateCoefficientFolders.back();
}

void RateCoefficientManager::RemoveRateCoefficientFolder(int index)
{
	s_folderNameList.erase(s_folderNameList.begin() + index);

	s_rateCoefficientFolders.erase(s_rateCoefficientFolders.begin() + index);
	s_currentRateCoefficientFolderIndex = std::min(s_currentRateCoefficientFolderIndex, (int)s_rateCoefficientFolders.size() - 1);

	s_selectedFolderIndex = std::min(s_selectedFolderIndex, (int)s_folderNameList.size() - 1);
}

std::vector<RateCoefficientFolder>& RateCoefficientManager::GetFolders()
{
	return s_rateCoefficientFolders;
}

bool RateCoefficientManager::IsFolderSelected()
{
	return s_currentRateCoefficientFolderIndex > -1;
}

std::vector<std::reference_wrapper<RateCoefficient>> RateCoefficientManager::GetSelectedRateCoefficients()
{
	std::vector<std::reference_wrapper<RateCoefficient>> result;

	// if a folder is selected return references to all contained rc
	if (s_currentRateCoefficientFolderIndex > -1)
	{
		for (RateCoefficient& rc : s_rateCoefficientFolders.at(s_currentRateCoefficientFolderIndex).m_rateCoefficients)
		{
			result.emplace_back(rc);
		}
		return result;
	}

	// else return a reference to the single rc that is selected
	for (RateCoefficientFolder& folder : s_rateCoefficientFolders)
	{
		if (folder.m_currentRateCoefficientIndex > -1)
		{
			result.emplace_back(folder.m_rateCoefficients.at(folder.m_currentRateCoefficientIndex));
			return result;
		}
	}

	return result;
}

std::string RateCoefficientManager::GetNameOfSelectedRateCoefficient()
{
	if (s_currentRateCoefficientFolderIndex > -1)
	{
		return s_rateCoefficientFolders.at(s_currentRateCoefficientFolderIndex).m_foldername;
	}
	else
	{
		for (RateCoefficientFolder& folder : s_rateCoefficientFolders)
		{
			if (folder.m_currentRateCoefficientIndex > -1)
			{
				return folder.m_rateCoefficients.at(folder.m_currentRateCoefficientIndex).GetLabel();
			}
		}
		return "no selected rc";
	}
}

RateCoefficientFolder* RateCoefficientManager::GetFolderByName(std::string& name)
{
	auto it = std::find_if(s_rateCoefficientFolders.begin(), s_rateCoefficientFolders.end(),
		[&](RateCoefficientFolder& folder)
		{
			return folder.GetName() == name;
		});

	if (it == s_rateCoefficientFolders.end())
		return nullptr;

	return &(*it);
}

void RateCoefficientManager::ShowFolderList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("rate coefficient folders", ImVec2(100, 100), flags))
	{
		ImGui::Text("rate coefficient folders");
		if (ImGui::BeginListBox("##rclist", ImVec2(-1, 200)))
		{
			ImGui::PushStyleColor(ImGuiCol_Header, s_inputTextColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, s_hoveredTextColor);

			for (size_t i = 0; i < s_rateCoefficientFolders.size(); i++)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen;

				RateCoefficientFolder& folder = s_rateCoefficientFolders.at(i);
				if (s_currentRateCoefficientFolderIndex == i)
				{
					node_flags |= ImGuiTreeNodeFlags_Selected;
				}
				ImGui::PushID(i);
				bool nodeOpen = ImGui::TreeNodeEx(folder.m_foldername.c_str(), node_flags);
				if (ImGui::IsItemClicked())
				{
					s_currentRateCoefficientFolderIndex = i;
					ResetAllRateCoefficientIndeces();
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
				if (ImGui::SmallButton("x"))
				{
					RemoveRateCoefficientFolder(i);
				}

				if (nodeOpen)
				{
					folder.ShowSelectablesOfRateCoefficients();
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::PopStyleColor(2);
			ImGui::EndListBox();
		}
		ShowLoadButtons();
	}
	ImGui::EndChild();
}

void RateCoefficientManager::ShowLoadButtons()
{
	if (ImGui::Button("load measured rc"))
	{
		std::vector<std::filesystem::path> files = FileUtils::SelectFiles(FileUtils::GetMeasuredRateCoefficientFolder(), { "*.dat" });
		if (!files.empty())
		{
			std::string foldername = files.at(0).parent_path().filename().string();

			RateCoefficientFolder* folder = RateCoefficientManager::GetFolderByName(foldername);
			if (!folder)
			{
				folder = &AddRateCoefficientFolder(RateCoefficientFolder(foldername));
			}

			for (std::filesystem::path& file : files)
			{
				RateCoefficient rc(file.filename().string());
				rc.Load(file);
				folder->AddRateCoefficient(rc);
			}
		}
	}
	if (ImGui::Button("load convolved rc"))
	{
		std::vector<std::filesystem::path> files = FileUtils::SelectFiles(FileUtils::GetRateCoefficientFolder(), { "*.dat" });
		if (!files.empty())
		{
			std::string foldername = files.at(0).parent_path().filename().string();

			RateCoefficientFolder* folder = RateCoefficientManager::GetFolderByName(foldername);
			if (!folder)
			{
				folder = &AddRateCoefficientFolder(RateCoefficientFolder(foldername));
			}

			for (std::filesystem::path& file : files)
			{
				RateCoefficient rc(file.filename().string());
				rc.Load(file);
				folder->AddRateCoefficient(rc);
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("load rc folder"))
	{
		std::filesystem::path folder = FileUtils::SelectFolder(FileUtils::GetRateCoefficientFolder());
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
			RateCoefficientFolder newFolder(folder.filename().string());
			newFolder.Load(folder);
			AddRateCoefficientFolder(std::move(newFolder));
		}

	}
	ImGui::SameLine();
	if (ImGui::Button("new rc folder"))
	{
		// dont allow duplicate folder names
		if (IsFolderInList(s_newFolderNameInput))
		{
			std::cout << "foldername already exists" << std::endl;
			// TODO make popup/modal window
		}
		else
		{
			RateCoefficientFolder newFolder(s_newFolderNameInput);
			AddRateCoefficientFolder(std::move(newFolder));
		}

	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGuiUtils::InputText("new folder name", &s_newFolderNameInput);
}

void RateCoefficientManager::ShowConvolutionControls()
{
	ImGui::SeparatorText("input");
	ImGui::Text("energy distribution set: "); ImGui::SameLine();
	ImGui::TextColored(s_inputTextColor, "%s", EnergyDistributionWindow::GetCurrentSet().GetLabel().c_str());

	ImGui::Text("target cross section: "); ImGui::SameLine();
	ImGui::TextColored(s_inputTextColor, "%s", CrossSectionManager::GetNameOfSelectedCrossSection().c_str());

	ImGui::SeparatorText("output");

	ImGui::BeginDisabled(s_selectedFolderIndex < 0 || s_newRateCoefficientNameInput.empty());
	if (ImGui::Button("Convolve Rate Coefficient"))
	{
		OnButtonConvolve();
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
	ImGui::BeginDisabled(CrossSectionManager::IsFolderSelected());
	ImGuiUtils::InputText("filename", &s_newRateCoefficientNameInput);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100.0f);
	ImGuiUtils::InputText("_ext", &s_newRateCoefficientNameExtensionInput);

	//ShowSizeMismatchPopup();
	//ShowMissingDataPopup();
}

void RateCoefficientManager::AddRateCoefficientToSelectedFolder(RateCoefficient& rc)
{
	RateCoefficientFolder& SelectedRcFolder = s_rateCoefficientFolders.at(s_selectedFolderIndex);
	rc.Save(SelectedRcFolder.m_foldername);
	SelectedRcFolder.AddRateCoefficient(rc);
}

void RateCoefficientManager::ResetAllRateCoefficientIndeces()
{
	for (RateCoefficientFolder& folder : s_rateCoefficientFolders)
	{
		folder.m_currentRateCoefficientIndex = -1;
	}
}

bool RateCoefficientManager::IsFolderInList(std::string& foldername)
{
	return std::find(s_folderNameList.begin(), s_folderNameList.end(), foldername) != s_folderNameList.end();
}

void RateCoefficientManager::OnButtonConvolve()
{
	std::vector<EnergyDistributionSet>& setList = EnergyDistributionWindow::GetSetList();
	auto selectedCrossSections = CrossSectionManager::GetSelectedCrossSections();
	
	if (setList.empty() || selectedCrossSections.empty())
	{
		ImGui::OpenPopup("missing data");
		std::cout << "no energy distribution set or cross section selected\n";
		return;
	}
	
	EnergyDistributionSet& currentSet = EnergyDistributionWindow::GetCurrentSet();

	for (auto csRef : selectedCrossSections)
	{
		CrossSection& cs = csRef.get();

		//if (rc.GetSize() != currentSet.GetSize())
		//{
		//	ImGui::OpenPopup("size mismatch");
		//	std::cout << "sizes of rate coefficients and energy distributions dont match: " +
		//		std::to_string(rc.GetSize()) + " != " + std::to_string(currentSet.GetSize()) << std::endl;
		//	return;
		//}
		std::string newName;
		if (CrossSectionManager::IsFolderSelected())
		{
			newName = cs.GetLabel() + "_" + s_newRateCoefficientNameExtensionInput;
		}
		else
		{
			newName = s_newRateCoefficientNameInput + "_" + s_newRateCoefficientNameExtensionInput;
		}
		RateCoefficient rc(newName);
		rc.Convolve(cs, currentSet);
		AddRateCoefficientToSelectedFolder(rc);
	}
}
