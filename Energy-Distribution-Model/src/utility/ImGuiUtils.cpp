#include "pch.h"
#include "ImGuiUtils.h"
#include <Application.h>

void ImGuiUtils::TextTooltip(std::string text)
{
	if (!Application::GetSettings().tooltipsDisabled && ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 15.0f);
		ImGui::Text(text.c_str());
		//ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.0f), text.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ImGuiUtils::ErrorPopup(std::string title, std::string errorMessage)
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("%s", errorMessage.c_str());
		ImGui::SetItemDefaultFocus();

		// Width of the button you want
		float buttonWidth = 120.0f;

		// Calculate horizontal center position inside the current window
		float availableWidth = ImGui::GetContentRegionAvail().x;
		float cursorX = (availableWidth - buttonWidth) * 0.5f;

		// Move cursor to the center position
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + cursorX);

		if (ImGui::Button("Ok", ImVec2(buttonWidth, 0.0f))) 
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}
