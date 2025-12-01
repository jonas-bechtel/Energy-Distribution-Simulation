#pragma once

void InitImGui();
void ShutdownImGui();
void BeginImGuiFrame();
void PollEvents(bool& shouldClose);
void RenderImGuiFrame();
