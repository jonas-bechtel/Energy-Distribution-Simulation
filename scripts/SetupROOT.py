import os
from pathlib import Path

import Utils
import platform

class RootConfiguration:
    rootVersion = "6.32.04"
    rootZipUrlsWindows = f"https://root.cern/download/root_v{rootVersion}.win64.vc17.zip"
    rootZipUrlsLinux = f"https://root.cern/download/root_v{rootVersion}.Linux-ubuntu24.04-x86_64-gcc13.2.tar.gz"
    rootDirectory = f"./vendor/root_v{rootVersion}"

    @classmethod
    def Validate(cls):
        if (not cls.CheckIfRootInstalled()):
            print(f"root {cls.rootVersion} is not installed.")
            return False

        print(f"Correct root located at {os.path.abspath(cls.rootDirectory)}")
        return True

    @classmethod
    def CheckIfRootInstalled(cls):
        if (not Path(cls.rootDirectory).exists()):
            return cls.InstallRoot()

        return True

    @classmethod
    def InstallRoot(cls):
        permissionGranted = False
        while not permissionGranted:
            reply = str(input("Root not found. Would you like to download root {0:s}? [Y/N]: ".format(cls.rootVersion))).lower().strip()[:1]
            if reply == 'n':
                return False
            permissionGranted = (reply == 'y')

        if platform.system() == "Windows":
            rootPath = f"{cls.rootDirectory}/root-{cls.rootVersion}-windows.zip"
            print("Downloading {0:s} to {1:s}".format(cls.rootZipUrlsWindows, rootPath))
            Utils.DownloadFile(cls.rootZipUrlsWindows, rootPath)

        if platform.system() == "Linux":
            rootPath = f"{cls.rootDirectory}/root-{cls.rootVersion}-linux.tar.gz"
            print("Downloading {0:s} to {1:s}".format(cls.rootZipUrlsLinux, rootPath))
            Utils.DownloadFile(cls.rootZipUrlsLinux, rootPath)

        print("Extracting", rootPath)
        Utils.UnzipFile(rootPath, deleteZipFile=True)
        print(f"Root {cls.rootVersion} has been downloaded to '{cls.rootDirectory}'")

        Utils.MoveDirectoryContent(cls.rootDirectory + "/root", cls.rootDirectory)
        os.rmdir(cls.rootDirectory + "/root")

        return True