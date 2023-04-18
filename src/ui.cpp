#include "pch.h"
#include "ui.h"
#include "utils.h"
#include "downloaders.h"

void GetFolderFromUser(void* pData)
{

    StartInstallData* data = (StartInstallData*)pData;

    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        CComPtr<IFileOpenDialog> pDialog;
        hr = pDialog.CoCreateInstance(CLSID_FileOpenDialog);

        if (SUCCEEDED(hr))
        {

            FILEOPENDIALOGOPTIONS opt{};
            hr = pDialog->GetOptions(&opt);

            if (SUCCEEDED(hr))
            {

                pDialog->SetOptions(opt | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

                if (SUCCEEDED(pDialog->Show(nullptr)))
                {
                    CComPtr<IShellItem> pShellItem;

                    hr = pDialog->GetResult(&pShellItem);

                    if (SUCCEEDED(hr))
                    {

                        CComHeapPtr<wchar_t> pPath;
                        hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
                        if (SUCCEEDED(hr))
                        {

                            if (pPath.m_pData)
                            {
                                CoUninitialize();
                                data->installPath = pPath.m_pData;
                                std::string newLabel = "Selected Directory: ";
                                newLabel.append(wstringConv(pPath.m_pData));
                                data->folderSelectButton->ChangeText(newLabel);
                                return;                            
                            }
                        }
                    }

                }

            }

        }

    }

    data->installPath = L"";

    //return L"";

}

const char* state(lt::torrent_status::state_t s)
{
    switch (s) {
    case lt::torrent_status::checking_files: return "Checking";
    case lt::torrent_status::downloading_metadata: return "Downloading metadata";
    case lt::torrent_status::downloading: return "Downloading";
    case lt::torrent_status::finished: return "Finished";
    case lt::torrent_status::seeding: return "Seeding";
    case lt::torrent_status::checking_resume_data: return "Checking resume";
    default: return "<>";
    }
}

void PrintZipExtractProgress(zip_uint64_t numFiles, zip_uint64_t numFilesExtracted, COORD* coords)
{
    numFilesExtracted++;

    float percentage = ((float)numFilesExtracted / (float)numFiles * 100);
    
    size_t leaderSize = snprintf(nullptr, 0, "Extracting %.2f%% [", percentage);
    char* leader = (char*)malloc(leaderSize + 1);
    sprintf(leader, "Extracting %.2f%% [", percentage);

    size_t trailerSize = snprintf(nullptr, 0, "] Files extracted %llu of %llu", numFilesExtracted, numFiles);
    char* trailer = (char*)malloc(trailerSize + 1);
    sprintf(trailer, "] Files extracted %llu of %llu", numFilesExtracted, numFiles);

    size_t fillerSize = coords[1].X - leaderSize / sizeof(char) - trailerSize / sizeof(char);
    
    char* filler = PrintFiller(fillerSize, percentage / 100);

    std::cout << '\r' << leader << filler << trailer;
    std::cout.flush();
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coords[0]);

    free(leader);
    free(filler);
    free(trailer);

}

int PrintCurlDownloadStatus(progress* progessData, double dltotal, double dlnow, double ultotal, double ulnow)
{
    size_t downloaded;

    curl_easy_getinfo(progessData->curl, CURLINFO_SIZE_DOWNLOAD_T, &downloaded);

    if (downloaded != 0)
    {

        curl_off_t speed = 0;

        curl_easy_getinfo(progessData->curl, CURLINFO_SPEED_DOWNLOAD_T, &speed);

        //If user doesnt specify data size try and pull it from header
        if (progessData->DataSize == 0)
        {
            curl_easy_getinfo(progessData->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, progessData->DataSize);
        }

        float percentage = (float)downloaded / (float)progessData->DataSize * 100;

        size_t leaderSize;
        char* leader;
        if (progessData->DataSize != 0)
        {

            //Create Leader
            leaderSize = snprintf(nullptr, 0, "Downloading %s %.2f%% [", progessData->fileName, percentage);
            leader = (char*)malloc(leaderSize + 1);
            sprintf(leader, "Downloading %s %.2f%% [", progessData->fileName, percentage);

            char* convertedSize = convertDataSize(downloaded);
            char* convertedSpeed = convertDataSize(speed);

            size_t trailerSize = snprintf(nullptr, 0, "] Total Downloaded %s at %s/s", convertedSize, convertedSpeed);
            char* trailer = (char*)malloc(trailerSize + 1);
            sprintf(trailer, "] Total Downloaded %s at %s/s", convertedSize, convertedSpeed);

            free(convertedSize);
            free(convertedSpeed);


            //Create filler
            size_t fillerSize = progessData->coords[1].X - leaderSize / sizeof(char) - trailerSize / sizeof(char);


            char* filler = PrintFiller(fillerSize, percentage / 100);
            std::cout << '\r' << leader << filler << trailer;


            free(leader);
            free(filler);
            free(trailer);

        }
        else
        {
            char* dot;

            if (progessData->dotslol != 5)
            {
                
                dot = PrintDots(progessData->dotslol, 5);
                progessData->dotslol++;
            }
            else
            {
                dot = PrintDots(progessData->dotslol, 5);
                progessData->dotslol = 0;
            }

            leaderSize = snprintf(nullptr, 0, "Downloading %s%s", progessData->fileName, dot);
            leader = (char*)malloc(leaderSize + 1);
            sprintf(leader, "Downloading %s%s", progessData->fileName, dot);

            std::cout << '\r' << leader;
            
            
            free(dot);
            free(leader);

        }

         std::cout.flush();
         SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), progessData->coords[0]);
    }

    return CURLE_OK;

}

char* PrintDots(int num, int maxDots)
{
    if (num != 0)
    {
        char* dot = (char*)malloc(maxDots + 2);
        //memset(dot, 0, maxDots + 1);
        for (int i = 0; i < num; i++)
        {
            dot[i] = '.';
        }
        dot[num ] = '\0';
        return dot;
    }
    else
    {
        char* dot = (char*)malloc(maxDots + 2);
        dot[maxDots ] = '\0';
        for (int i = 0; i < maxDots; i++)
        {
            dot[i] = ' ';
        }
        
        
        return dot;
    }

}

char* PrintFiller(size_t size, float fillAmount)
{
    if (size > 1)
    {
        char* filler = (char*)malloc(size + 1);
        filler[size] = '\0';
    
        int i = 0;
        char* fillerChar = &filler[0];
        for (; i < std::floor(size * fillAmount);)
        {
            *fillerChar = '>';
            fillerChar++;
            i++;
        }

        for (; i < size;)
        {
            *fillerChar = ' ';
            fillerChar++;
            i++;
        }

        return filler;
    }

    char* filler = (char*)malloc(1);
    *filler = '\0';
    return filler;
}

void GetConsolePosAndSize(COORD* coords)
{

    HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO pConsoleInfo;

    GetConsoleScreenBufferInfo(ConsoleHandle, &pConsoleInfo);

    coords[0] = pConsoleInfo.dwCursorPosition;
    coords[1] = pConsoleInfo.dwSize;

}

void FinalTorrentPrint(COORD* coords)
{

    const char* leader = "Torrent Finished 100% [";
    const char* trailer = "]";


    size_t fillerSize = coords[1].X - strlen(leader) - strlen(trailer);
    
    char* filler = PrintFiller(fillerSize,1);

    std::cout << '\r' << leader << filler << trailer;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coords[0]);

    free(filler);

}

void PrintTorrentStatus(const lt::torrent_status& status, COORD* coords)
{

    float progress1 = (float)(status.progress * 100);

    if (isTorrentFinished)
    {
        progress1 = 100;
    }

    //Create Leader
    size_t leaderSize = snprintf(nullptr, 0, "%s %.2f%% [", state(status.state), progress1);
    char* leader = (char*)malloc(leaderSize + 1);
    sprintf(leader, "%s %.2f%% [", state(status.state), progress1);

    size_t trailerSize = 0;
    char* trailer = nullptr;

    switch (status.state)
    {
    case(lt::torrent_status::downloading):
    {
        char* downloaded = convertDataSize(status.all_time_download);
        char* rate = convertDataSize(status.download_rate);

        trailerSize = snprintf(nullptr, 0, "] Total Downloaded %s at %s/s", downloaded, rate);
        trailer = (char*)malloc(trailerSize + 1);
        sprintf(trailer, "] Total Downloaded %s at %s/s", downloaded, rate);

        free(downloaded);
        free(rate);
        break;
    }
    default:
    {
        trailerSize = snprintf(nullptr, 0, "]");
        trailer = (char*)malloc(trailerSize + 1);
        sprintf(trailer, "]");
        break;
    }
    }

    //Create filler
    size_t fillerSize = coords[1].X - leaderSize / sizeof(char) - trailerSize / sizeof(char);
   
    char* filler = PrintFiller(fillerSize, status.progress);
   
    std::cout << '\r' << leader << filler << trailer;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coords[0]);


    free(leader);
    free(filler);
    free(trailer);
    //std::cout.flush();


}

COORD UIElement::GetActualCoords()
{
    COORD realCoord;
    realCoord.X = position.X + BaseCoords.X;
    realCoord.Y = position.Y + BaseCoords.Y;
    return realCoord;
}

void UIElement::DrawElement()
{

    COORD baseCoords = GetBaseCoords(); //Get base cords of this ui class

    COORD ActualCoords = GetActualCoords();


    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

    GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
    WORD attributes = consoleInfo.wAttributes; // Store current attributes

    SetConsoleCursorPosition(consoleHandle, ActualCoords);

    if (Colour)
    {
        SetConsoleTextAttribute(consoleHandle, Colour);
        std::cout << text << " ";
        SetConsoleTextAttribute(consoleHandle, attributes);
    }
    else
    {
        std::cout << text << " ";
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
    }
};

void ConsoleUI::DrawElements(COORD BaseCords)
{
    BaseCoords = BaseCords;

    for (auto ittr : elements)
    {
        ittr->DrawElement();
    }

}

void ConsoleUI::AddElement(UIElement* element) {
    elements.emplace_back(element);


    //Expand our cursor range when we add a new element
    if (element->GetCoords().Y > cursorValidMove[1].Y)
    {
        cursorValidMove[1].Y = element->GetCoords().Y;
    }

    if (element->GetCoords().Y < cursorValidMove[0].Y)
    {
        cursorValidMove[0].Y = element->GetCoords().Y;
    }
}

COORD UIElement::GetBaseCoords() {
    return BaseCoords;
}

HANDLE UIElement::GetConsoleHandle()
{
    return consoleHandle;
}

void UIElement::ChangeText(std::string& newText)
{
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

    GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);

    SetConsoleCursorPosition(consoleHandle, GetActualCoords());

    DWORD numWritten = 0;
    FillConsoleOutputCharacterA(consoleHandle, ' ', consoleInfo.dwSize.X, GetActualCoords(), &numWritten);

    std::cout << newText;
    std::cout.flush();

    SetConsoleCursorPosition(consoleHandle, consoleInfo.dwCursorPosition);

    text = newText;
}

void ConsoleUI::ExecuteInteract(COORD cursorPos, Cursor* cursor)
{
    for (auto ittr : elements)
    {
        COORD ret = ittr->GetCoords();

        if (ret.Y == cursor->GetCoords().Y) //Is this element on the same line as the cursor
        {
            ittr->Action(); //Call the action function to run the appropriate function for the element
            return;
        }
    }
}

void Toggle::DrawElement()
{

    COORD realCoord = GetActualCoords();

    //HANDLE consoleHandle = GetConsoleHandle();

    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

    GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
    WORD attributes = consoleInfo.wAttributes; // Store current attributes

    SetConsoleCursorPosition(consoleHandle, realCoord);

    if (Colour)
    {

        SetConsoleTextAttribute(consoleHandle, Colour);
        std::cout << text << " ";
        SetConsoleTextAttribute(consoleHandle, attributes);
    }
    else
    {
        std::cout << text << " ";
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
    }

    togglePos = consoleInfo.dwCursorPosition; //This will be the position where the toggle starts


    if (*pToggle)
    {
        std::cout << toggleBorder[0] << toggleElementOn << toggleBorder[1];
    }
    else
    {
        std::cout << toggleBorder[0] << toggleElementOff << toggleBorder[1];
    }

}

void Toggle::ToggleElement()
{
    size_t len = strlen(toggleBorder[0]); //Get the length of the boarder

    COORD cursorPos;
    cursorPos.Y = togglePos.Y;
    cursorPos.X = togglePos.X + len;
    SetConsoleCursorPosition(consoleHandle, cursorPos);

    if (*pToggle) //If its currently on we want to toggle it off
    {
        std::cout << toggleElementOff;
    }
    else
    {
        std::cout << toggleElementOn;
    }

    *pToggle = !*pToggle;
}

void Cursor::MoveCursor(SHORT direction)
{
    if (direction == 1 && position.Y + 1 <= cursorValidMove[1].Y || direction == -1 && position.Y - 1 >= cursorValidMove[0].Y) //Is the movement we are trying to make valid
    {

        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
        SetConsoleTextAttribute(consoleHandle, Colour);

        position.X++;
        SetConsoleCursorPosition(consoleHandle, this->GetActualCoords()); //Move forward to next char
        std::cout << "\b" << " "; //Remove the character and replace with nothing
        position.X--; //Return back to start to print next char

        position.Y += direction; //Do we move up or down
        SetConsoleCursorPosition(consoleHandle, this->GetActualCoords());
        std::cout << text;
        SetConsoleCursorPosition(consoleHandle, this->GetActualCoords());

        SetConsoleTextAttribute(consoleHandle, consoleInfo.wAttributes);
    }
}