import os
import subprocess
import platform

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupPremake import PremakeConfiguration as PremakeRequirements
from SetupROOT import RootConfiguration as RootRequirements

os.chdir('./../') # Change from scripts directory to root

premakeInstalled = PremakeRequirements.Validate()
rootInstalled = RootRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

# stop imgui.ini file from beeing tracked
subprocess.call(["git", "update-index", "--skpi-worktree", "*/imgui.ini"])

if (premakeInstalled):
    if platform.system() == "Windows":
        print("\nRunning premake...")
        subprocess.call([os.path.abspath("./scripts/Win-GenProjects.bat"), "nopause"])
        
    if platform.system() == "Linux":
        print("\nRunning premake...")
        subprocess.call([os.path.abspath("./scripts/Linux-GenProjects.sh"), "nopause"])
    print("\nSetup completed!")
else:
    print("We require Premake to generate project files.")