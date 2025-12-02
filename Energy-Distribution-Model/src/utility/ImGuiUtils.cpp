#include "pch.h"
#include "ImGuiUtils.h"
#include <Application.h>

struct InputTextCallback_UserData
{
	std::string* Str;
	ImGuiInputTextCallback  ChainCallback;
	void* ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
		std::string* str = user_data->Str;
		IM_ASSERT(data->Buf == str->c_str());
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
	}
	else if (user_data->ChainCallback)
	{
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	return 0;
}

bool ImGuiUtils::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;
	return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

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
