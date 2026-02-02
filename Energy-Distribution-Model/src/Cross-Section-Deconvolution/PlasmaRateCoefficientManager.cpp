#include "pch.h"
#include "PlasmaRateCoefficientManager.h"
#include <FileUtils.h>

void PlasmaRateCoefficientManager::AddPlasmaRateCoefficientToFolder(PlasmaRateCoefficient prc, std::string& folderName)
{
	PlasmaRateCoefficientFolder* folder = GetFolderByName(folderName);
	if (!folder)
	{
		folder = &AddPlasmaRateCoefficientFolder(PlasmaRateCoefficientFolder(folderName));
	}
	prc.Save(folder->GetName());
	folder->AddPlasmaRateCoefficient(prc);
}

PlasmaRateCoefficientFolder& PlasmaRateCoefficientManager::AddPlasmaRateCoefficientFolder(PlasmaRateCoefficientFolder folder)
{
	s_folderNameList.push_back(folder.m_foldername);
	s_selectedFolderIndex = s_folderNameList.size() - 1;

	s_plasmaRateCoefficientFolders.emplace_back(std::move(folder));
	s_currentPlasmaRateCoefficientFolderIndex = s_plasmaRateCoefficientFolders.size() - 1;

	ResetAllPlasmaRateCoefficientIndeces();

	return s_plasmaRateCoefficientFolders.back();
}

void PlasmaRateCoefficientManager::RemovePlasmaRateCoefficientFolder(int index)
{
	s_folderNameList.erase(s_folderNameList.begin() + index);

	s_plasmaRateCoefficientFolders.erase(s_plasmaRateCoefficientFolders.begin() + index);
	s_currentPlasmaRateCoefficientFolderIndex = std::min(s_currentPlasmaRateCoefficientFolderIndex, (int)s_plasmaRateCoefficientFolders.size() - 1);
	
	s_selectedFolderIndex = std::min(s_selectedFolderIndex, (int)s_folderNameList.size() - 1);
}

std::vector<PlasmaRateCoefficientFolder>& PlasmaRateCoefficientManager::GetFolders()
{
	return s_plasmaRateCoefficientFolders;
}

PlasmaRateCoefficientFolder* PlasmaRateCoefficientManager::GetFolderByName(std::string& name)
{
	auto it = std::find_if(s_plasmaRateCoefficientFolders.begin(), s_plasmaRateCoefficientFolders.end(),
		[&](PlasmaRateCoefficientFolder& folder)
		{
			return folder.GetName() == name;
		});

	if (it == s_plasmaRateCoefficientFolders.end())
		return nullptr;

	return &(*it);
}

void PlasmaRateCoefficientManager::ShowFolderList()
{
	ImGuiChildFlags flags = ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
	if (ImGui::BeginChild("plasma rate coefficient folders", ImVec2(100, 100), flags))
	{
		ImGui::Text("plasma rate coefficient folders");
		if (ImGui::BeginListBox("##prclist", ImVec2(-1, 200)))
		{
			for (size_t i = 0; i < s_plasmaRateCoefficientFolders.size(); i++)
			{
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
					ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_DefaultOpen;

				PlasmaRateCoefficientFolder& folder = s_plasmaRateCoefficientFolders.at(i);
				if (s_currentPlasmaRateCoefficientFolderIndex == i)
				{
					node_flags |= ImGuiTreeNodeFlags_Selected;
				}
				ImGui::PushID(i);
				bool nodeOpen = ImGui::TreeNodeEx(folder.m_foldername.c_str(), node_flags);
				if (ImGui::IsItemClicked())
				{
					s_currentPlasmaRateCoefficientFolderIndex = i;
					ResetAllPlasmaRateCoefficientIndeces();
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
					RemovePlasmaRateCoefficientFolder(i);
				}

				if (nodeOpen)
				{
					folder.ShowSelectablesOfPlasmaRateCoefficients();
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		ShowLoadButtons();
	}
	ImGui::EndChild();
}

void PlasmaRateCoefficientManager::ShowLoadButtons()
{
	if (ImGui::Button("load prc"))
	{
		std::vector<std::filesystem::path> files = FileUtils::SelectFiles(FileUtils::GetPlasmaRateFolder(), { "*.dat" });
		if (!files.empty())
		{
			std::string foldername = files.at(0).parent_path().filename().string();

			PlasmaRateCoefficientFolder* folder = PlasmaRateCoefficientManager::GetFolderByName(foldername);
			if (!folder)
			{
				folder = &AddPlasmaRateCoefficientFolder(PlasmaRateCoefficientFolder(foldername));
			}

			for (std::filesystem::path& file : files)
			{
				PlasmaRateCoefficient prc(file.filename().string());
				prc.Load(file);
				folder->AddPlasmaRateCoefficient(prc);
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("load prc folder"))
	{
		std::filesystem::path folder = FileUtils::SelectFolder(FileUtils::GetPlasmaRateFolder());
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
			PlasmaRateCoefficientFolder newFolder(folder.filename().string());
			newFolder.Load(folder);
			AddPlasmaRateCoefficientFolder(std::move(newFolder));
		}
	}
}

void PlasmaRateCoefficientManager::ResetAllPlasmaRateCoefficientIndeces()
{
	for (PlasmaRateCoefficientFolder& folder : s_plasmaRateCoefficientFolders)
	{
		folder.m_currentPlasmaRateCoefficientIndex = -1;
	}
}

bool PlasmaRateCoefficientManager::IsFolderInList(const std::string& foldername)
{
	return std::find(s_folderNameList.begin(), s_folderNameList.end(), foldername) != s_folderNameList.end();
}
