#pragma once 
#include "pch.h"

struct StartInstallData
{
    std::wstring installPath;
    bool shouldInstallFlowstate;
};


struct progress
{
    CURL* curl;
    curl_off_t DataSize = 0;
    COORD* coords;
    char* fileName = (char*)"";
    int dotslol;
};

void GetFolderFromUser(void* pData);
void PrintTorrentStatus(const lt::torrent_status& status, COORD* coords);
int PrintCurlDownloadStatus(progress* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
void PrintZipExtractProgress(zip_uint64_t numFiles, zip_uint64_t numFilesExtracted, COORD* coords);
void FinalTorrentPrint(COORD* coords);
void GetConsolePosAndSize(COORD* coords);
char* PrintFiller(size_t size, float fillAmount);
char* PrintDots(int num, int maxDots);

class UIElement;
class Cursor;

class ConsoleUI
{
public:

    ~ConsoleUI()
    {
        for (auto ittr : elements)
        {
            delete ittr;
        }
    }

    ConsoleUI()
    {
        consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
        BaseCoords = consoleInfo.dwCursorPosition;
    }

    void ExecuteInteract(COORD cursorPos, Cursor* cursor);

    void DrawElements(COORD BaseCords);

    void AddElement(UIElement* element);

protected:
    COORD BaseCoords{ 0 }; //This will be the start of the ui section and all elements cords will be relative to this
    std::vector<UIElement*> elements;
    HANDLE consoleHandle;
    inline static COORD cursorValidMove[2]{ 0 , 0 };
};

class UIElement : public ConsoleUI
{
public:

    virtual void Action() {};

    COORD GetActualCoords();

    virtual void DrawElement();

    COORD GetCoords()
    {
        return position;
    }

    COORD GetBaseCoords();
    HANDLE GetConsoleHandle();

protected:
    COORD position{ 0 };      //If this is a toggle it will be the position of the toggle indicator
    WORD Colour = 0;
    const char* text;
};


class Toggle : public UIElement
{
public:
    Toggle(const char* inText, int x, int y, bool* toggle)
    {
        position.X = x;
        position.Y = y;
        pToggle = toggle;
        text = inText;
    }

    void DrawElement();

    void ToggleElement();

    void Action()
    {
        ToggleElement();
    }


private:
    const char* text;
    COORD togglePos{ 0 };
    const char* toggleElementOn = "*";
    const char* toggleElementOff = " ";
    const char* toggleBoarder[2]{ "<",">" };
    bool* pToggle;
};

class Button : public UIElement
{

public:
    Button(const char* textin, int x, int y, std::function<void(void*)> CompletionFunction, void* pData1, WORD colourIn = 0)
    {
        completionFunction = CompletionFunction;
        pData = pData1;

        position.X = x;
        position.Y = y;

        Colour = colourIn;
        text = textin;
    }

    void Action() //Virtual function that calls the relevent internal action function
    {
        ButtonCallFunction();
    }

    void ButtonCallFunction()
    {
        completionFunction(pData);
    }

private:
    std::function<void(void*)> completionFunction{ 0 };
    void* callback = nullptr;
    void* pData;
};


//Cursor UIElement
class Cursor : public UIElement
{
public:
    Cursor(const char* cursor, int x, int y, WORD colour = 0)
    {
        position.X = x;
        position.Y = y;
        Colour = colour;
        text = cursor;
    }

    void MoveCursor(int direction);

};
