#include "pch.h"

#include "Application.h"

int main(int argc, char** argv) 
{
    TApplication app("app", nullptr, nullptr);

    Application::Init();
    Application::Run();
	std::cout << "Shutting down application..." << std::endl;
    Application::Shutdown();
	std::cout << "Application closed successfully." << std::endl;
   
}
