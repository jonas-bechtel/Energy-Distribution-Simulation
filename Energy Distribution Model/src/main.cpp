#include "pch.h"

#include "Application.h"

int main(int argc, char** argv) 
{
    TApplication app("app", nullptr, nullptr);

    Application::Init();
    Application::Run();
    Application::Shutdown();
}
