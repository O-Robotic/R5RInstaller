#include "pch.h"
#include "ui.h"
#include "utils.h"


char* convertDataSize(uint64_t bytes)
{
    uint64_t dataSizes[3]{ 1000 ,1000000,1000000000 };

    if (bytes >= dataSizes[2])
    {
        float convSize = (float)bytes / dataSizes[2];
        size_t size = snprintf(nullptr, 0, "%.2f GB", convSize);
        char* string = (char*)malloc(size + 1);
        sprintf(string, "%.2f GB", convSize);
        return string;
    }

    if (bytes >= dataSizes[1])
    {


        float convSize = (float)bytes / dataSizes[1];
        size_t size = snprintf(nullptr, 0, "%.2f GB", convSize);
        char* string = (char*)malloc(size + 1);
        sprintf(string, "%.2f MB", convSize);
        return string;
    }

    if (bytes >= dataSizes[0])
    {

        float convSize = (float)bytes / dataSizes[0];
        size_t size = snprintf(nullptr, 0, "%.2f KB", convSize);
        char* string = (char*)malloc(size + 1);
        sprintf(string, "%.2f KB", convSize);
        return string;
    }

    size_t size = snprintf(nullptr, 0, "%llu Bytes", bytes);
    char* string = (char*)malloc(size + 1);
    sprintf(string, "%llu Bytes", bytes);
    return string;

}

std::string wstringConv(const std::wstring& inString)
{
    if (inString.empty())
    {
        return "";
    }

    int  size = WideCharToMultiByte(CP_UTF8, 0, &inString.at(0), (int)inString.size(), nullptr, 0, nullptr, nullptr);
    if (size <= 0)
    {
        return "";
    }

    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &inString.at(0), (int)inString.size(), &result.at(0), size, nullptr, nullptr);
    return result;
}

bool formatDir(char* dir, char* removeFromPath)
{
    char* finalSepPtr = nullptr;
    char* seperator = strchr(dir, '/'); //Find directory seperator

    while (seperator != nullptr) //While we are finding new directories to create
    {
        *seperator = '\\';
        finalSepPtr = strchr(seperator, '\\');
        seperator = strchr(seperator + 1, '/');
    }

    if (finalSepPtr && (strcmp(finalSepPtr,"\\") == 0))
    {
        return false;
    }

    if (removeFromPath) //Do we want to remove something from the path output
    {
        //dont ask why
        
        size_t dirLength = strlen(removeFromPath);

        char* a;
        char* b;
        char* c = dir;
        b = c;

        //Make sure we only get the last instance of the dir we want to remove, idk there could be some wierdo with r5_flowstate-r5_flowstate in their file path lmao
        while (a = strstr(c, removeFromPath))
        {
            b = a;
            c += dirLength;
        }

        a = dir;
        a -= dir - b ;
        b += dirLength; //B now has the string - the removeFromPathDir

        //B will have the start of our substring
        while (( *a++ = *b++ ) && *b != (char)"\0")
        {
            continue;
        }
    }


    return true;
}

//size is always 1 nmemb is actual data size
size_t writeToString(char* ptr, size_t size, size_t nmemb, std::string* userdata)
{
    //Add to string
    userdata->append(ptr, size * nmemb);

    //cURL expects the amount of data it wrote as the return value
    return size * nmemb;
}

size_t writeCurlData(void* ptr, size_t size, size_t nnmemb, FILE* userdata)
{
    size_t bytesWritte = fwrite(ptr, size, nnmemb, userdata);
    return bytesWritte;
}

void createNeededDirs(char* pszFilePath)
{

    char workingBuf[MAX_PATH]{ 0 };
    char* seperator = strchr(pszFilePath, '\\'); //Find directory seperator

    char* finalSepPtr = nullptr;

    while (seperator != NULL) //While we are finding new directories to create
    {
        finalSepPtr = strchr(seperator, '\\');
        seperator = strchr(seperator + 1, '\\');
    }


    size_t length = strlen(pszFilePath) - strlen(finalSepPtr) + strlen("\\"); //Get the the length of just the directory path without the file

    memcpy(workingBuf, pszFilePath, length); //copy the data from the string without the file name
    workingBuf[length] = '\0'; //null terminate the copied data

    std::filesystem::create_directories(workingBuf);

    return;

}

bool ReadZipFileFromIndexIntoFile(zip* zip, zip_uint64_t zipFIleIndex, FILE* file)
{
    if (zip && file)
    {
        zip_file_t* zipFile = zip_fopen_index(zip, zipFIleIndex, 0);

        if (zipFile)
        {
            void* buffer = malloc(8192);


            while (zip_uint64_t dataRead = zip_fread(zipFile, buffer, 8192)) //Loop through zip data in max 8k chunks
            {
                fwrite(buffer, dataRead, 1, file);
            }

            free(buffer);

            zip_fclose(zipFile);

        }
        else
        {
            std::cout << "Could not open file from sdk zip" << std::endl;
            return false;
        }
    }

    return false; //If zip or file pointer is null return false
}

bool ExtractZip(std::string filePath, std::string extractPath, COORD* coords, char* pathToChop)
{
    int rc = 0;

    zip* zip = zip_open(filePath.c_str(), 0, &rc);

    if (zip)
    {
        zip_uint64_t numFiles = zip_get_num_entries(zip, 0);

        for (zip_uint64_t i = 0; i < numFiles; i++)
        {
            const char* fileName = zip_get_name(zip, i, 0);

            PrintZipExtractProgress(numFiles, i, coords);

            if (strcmp(fileName, "") != 0)
            {
               
                char filePathCstr[MAX_PATH];
                size_t written = snprintf(filePathCstr, MAX_PATH, "%s%s", extractPath.c_str(), fileName);

                //std::cout << "\n" << filePathCstr << std::endl;

                //if (written >= MAX_PATH)
                //{

                    if (formatDir(filePathCstr, pathToChop))
                    {
                        FILE* file = fopen(filePathCstr, "wb+");

                        if (file) //Check we actually managed to open the file, if we didint its probably because a directory doesnt exist
                        {

                            ReadZipFileFromIndexIntoFile(zip, i, file);
                            fclose(file);
                        }
                        else
                        {
                            createNeededDirs(filePathCstr); //Try to create all the directories 
                            file = fopen(filePathCstr, "wb+"); //Try opening the file again
                            if (file)
                            {
                                ReadZipFileFromIndexIntoFile(zip, i, file);
                                fclose(file);
                            }
                            else //If we cant open a file at this point something has gone very wrong and we should abort the install of the sdk
                            {
                                return false;
                            }
                            
                        }
                    }
               // }
               // else
               // {
               //     std::cout << "The file path of your install folder is too long" << std::endl;
               //     return false;
               // }
            }

        }

        zip_close(zip);

        return true;

    }
    else
    {
        std::cout << "Failed to open the a zip file" << std::endl;
        return false;
    }


}