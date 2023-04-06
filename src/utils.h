#pragma once
#include "pch.h"

bool formatDir(char* dir, char* removeFromDir);
size_t writeToString(char* ptr, size_t size, size_t nmemb, std::string* userdata);
size_t writeCurlData(void* ptr, size_t size, size_t nnmemb, FILE* userdata);
void createNeededDirs(char* pszFilePath);
bool ExtractZip(std::string filePath, std::string extractPath, COORD* coords, char* pathToChop = nullptr);
std::string wstringConv(const std::wstring& inString);
bool ReadZipFileFromIndexIntoFile(zip* zip, zip_uint64_t zipFIleIndex, FILE* file);
char* convertDataSize(uint64_t bytes);