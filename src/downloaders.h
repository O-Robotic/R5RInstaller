#pragma once
#include "pch.h"

inline bool isTorrentFinished = false;

nlohmann::json GetGithubJson(const char* url);

bool DownloadWithCURL(const std::string& url, const std::string& savePath,COORD* coords, char* fileName, curl_off_t DataSize = 0);

bool DownloadTorrent(const std::string& savePath, const char* magnetLink, COORD* coords);