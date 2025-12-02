#pragma once

namespace ImGuiUtils
{
	bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	void TextTooltip(std::string text);
	void ErrorPopup(std::string title, std::string errorMessage);
}
