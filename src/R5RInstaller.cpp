// R5RInstaller.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "downloaders.h"
#include "utils.h"
#include "ui.h"

const char* s3FilesMagnet = "";
const char* sdkUrl = "https://api.github.com/repos/Mauler125/r5sdk/releases";
const char* flowStateScripts = "https://github.com/ColombianGuy/r5_flowstate/archive/refs/heads/r5_flowstate.zip";
const char* flowstateRequiredFiles = "https://api.github.com/repos/ColombianGuy/r5_flowstate/releases";

const char* flowstateDLPath = "\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\Flowstate_scripts.zip";
const char* flowstateExtractPath = "\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\platform\\scripts\\";
const char* flowstateReqFilesDLPath = "\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\Flowstate.-.Required.Files.zip";
const char* flowstateReqFilesExtractPath = "\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\";

const char* sdkDLPath = "\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\depot.zip";
const char* sdkExtractPath = "\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\";

bool isInstalling = false;


void StartInstall(void* dataIn)
{

    StartInstallData* data = (StartInstallData*)dataIn;


    const std::string path = wstringConv(data->installPath);


    if(PathIsDirectoryA(path.c_str()))
    {

        std::string temporaryDLPath;
        std::string temporaryExtractPath;

        COORD coords[2];
        bool rc;

        //Check we have enough space
        ULARGE_INTEGER avaliableSize;
        GetDiskFreeSpaceEx(data->installPath.c_str(), &avaliableSize, nullptr, nullptr);

        if ((avaliableSize.QuadPart / (uint64_t)1024 * 1024 * 1024) < 46)
        {
            std::cout << "\nYou do not have enough space avaliable in this location." << std::endl;
            return;
        }

        std::cout << "\nDownloading to: " << path << std::endl;

        //Torrent Download
        GetConsolePosAndSize(coords);
        rc = DownloadTorrent(path, s3FilesMagnet,coords);

        if (!rc)
        {
            std::cout << "\nInstall Failed" << std::endl;
            return;
        }

        //Download the sdk
        nlohmann::json githubManifest = GetGithubJson(sdkUrl)[0]["assets"];

        size_t dataSize = 0;

        std::string githubDownloadUrl;

        for (auto& ittr : githubManifest)
        {
            if (ittr["name"] == "depot.zip")
            {
                githubDownloadUrl = ittr["browser_download_url"];
                dataSize = ittr["size"];
            }
        }

        temporaryDLPath = path + sdkDLPath;
        temporaryExtractPath = path + sdkExtractPath;


        std::cout << std::endl;
        GetConsolePosAndSize(coords);
        rc = DownloadWithCURL(githubDownloadUrl, temporaryDLPath, coords, (char*)"SDK", dataSize);

        if (!rc)
        {
            std::cout << "\nInstall Failed" << std::endl;
            return;
        }



        //Extract the sdk

        std::cout << std::endl;
        GetConsolePosAndSize(coords);
        rc = ExtractZip(temporaryDLPath, temporaryExtractPath ,coords);

        if (!rc)
        {
            std::cout << "\nInstall Failed" << std::endl;
            return;
        }

        if (data->shouldInstallFlowstate)
        {
            temporaryDLPath = path + flowstateDLPath;
            temporaryExtractPath = path + flowstateExtractPath;

            std::cout << std::endl;

            GetConsolePosAndSize(coords);
            rc = DownloadWithCURL(flowStateScripts, temporaryDLPath, coords, (char*)"Flowstate Scripts");

            if (!rc)
            {
                std::cout << "\nInstall Failed" << std::endl;
                return;
            }

            //Extract the flowstate scripts


            temporaryDLPath = path + flowstateDLPath;
            temporaryExtractPath = path + flowstateExtractPath;

            std::cout << std::endl;

            GetConsolePosAndSize(coords);
            rc = ExtractZip(temporaryDLPath, temporaryExtractPath, coords, (char*)"r5_flowstate-r5_flowstate\\"); //We must specify to remove the "r5_flowstate-r5_flowstate" from the extracted file path 
            //because github puts releases into a folder inside the zip

            if (!rc)
            {
                std::cout << "\nInstall Failed" << std::endl;
                return;
            }

            std::cout << std::endl;



            //Download the flowstate required files

            std::string fsGithubDLURL;
            size_t fsGithubDLSize = 0;
            nlohmann::json flowstateJson = GetGithubJson(flowstateRequiredFiles)[0]["assets"];

            for (auto& ittr : flowstateJson)
            {
                if (ittr["name"] == "Flowstate.-.Required.Files.zip")
                {
                    fsGithubDLURL = ittr["browser_download_url"];
                    fsGithubDLSize = ittr["size"];
                }
            }

            temporaryDLPath = path + flowstateReqFilesDLPath;

            GetConsolePosAndSize(coords);
            rc = DownloadWithCURL(fsGithubDLURL, temporaryDLPath, coords, (char*)"Flowstate Required Files", fsGithubDLSize);

            if (!rc)
            {
                std::cout << "\nInstall Failed" << std::endl;
                return;
            }



            //Extract the flowstate required files

            std::cout << std::endl;
            temporaryExtractPath = path + flowstateReqFilesExtractPath;

            GetConsolePosAndSize(coords);
            rc = ExtractZip(temporaryDLPath, temporaryExtractPath, coords);

            if (!rc)
            {
                std::cout << "\nInstall Failed" << std::endl;
                return;
            }
        }
        
        std::cout << "\nInstalation completed\nMake sure you have origin or the ea app open, logged in and with apex in the library before running." << std::endl;
        return;
    }
    else
    {
        std::cout << "\nInvalid install directory, the file path must not contain non-latin characters" << std::endl;
    }
}

void SelectionUI()
{
 
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    CONSOLE_CURSOR_INFO cursorInfo;

    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);


    GetConsoleCursorInfo(consoleHandle, &cursorInfo);

    cursorInfo.bVisible = false;

    SetConsoleCursorInfo(consoleHandle, &cursorInfo);


    const char* logo = R"(+-----------------------------------------------+
|   ___ ___ ___     _              _        _   |
|  | _ \ __| _ \___| |___  __ _ __| |___ __| |	|
|  |   /__ \   / -_) / _ \/ _` / _` / -_) _` |  |
|  |_|_\___/_|_\___|_\___/\__,_\__,_\___\__,_|  |
|                                               |
+-----------------------------------------------+)";


    //Print logo
    SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED);
    std::cout << logo << std::endl;
    SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    //Print insructions
    std::cout << "Arrow keys to navigate, enter to select\n1. Select an install folder by pressing enter on \"Select Install Folder\"\n2. Enable or disable installing flowstate (enabled by default)\n3. Press enter on start install\n" << std::endl;

    GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);

    StartInstallData InstallData;
    
    InstallData.shouldInstallFlowstate = true;

    ConsoleUI* cUI = new ConsoleUI();
    Button* selectFolder = new Button("Select Install Folder", 2, 0, GetFolderFromUser ,&InstallData, 0);

    InstallData.folderSelectButton = selectFolder;

    Toggle* flowstate = new Toggle("Install Flowstate", 2, 1, &InstallData.shouldInstallFlowstate);
    Button* install = new Button("Start Install", 2, 2, StartInstall, &InstallData, FOREGROUND_GREEN);
    Cursor* cursor = new Cursor(">",0, 0, FOREGROUND_GREEN);

    cUI->AddElement(selectFolder);
    cUI->AddElement(install);
    cUI->AddElement(flowstate);
    cUI->AddElement(cursor);

    cUI->DrawElements(consoleInfo.dwCursorPosition);

    while (true)
    {
        CONSOLE_SCREEN_BUFFER_INFO currentPos{ 0 };
        GetConsoleScreenBufferInfo(consoleHandle, &currentPos);

        switch (_getch())
        {
        case(13): //User presses enter   
            cUI->ExecuteInteract(currentPos.dwCursorPosition, cursor);
            break;
        case(72):
                cursor->MoveCursor(-1);
            break;
        case(80):
                cursor->MoveCursor(1);
            break;
        }
    }

    delete cUI;
}

int main()
{
    HWND consoleWindow = GetConsoleWindow();
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ~ENABLE_QUICK_EDIT_MODE);

    SelectionUI();

    system("pause");
  
}