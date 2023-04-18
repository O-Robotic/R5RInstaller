#include "pch.h"
#include "utils.h"
#include "ui.h"
#include "downloaders.h"

nlohmann::json GetGithubJson(const char* url)
{

    std::string responseData;
    CURLcode res;
    CURL* pCurl = curl_easy_init();

    if (pCurl)
    {
        struct curl_slist* list = NULL;
        list = curl_slist_append(list, "Content-Type: application/json"); //Expect json output from github api
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "curl/8.0.1"); //Supply user agent or guthub regects the connection
        curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false); //REMOVE FOR RELEASE
        curl_easy_setopt(pCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL); //Force SSL since github uses it 
        curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writeToString); //String append function
        curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &responseData); //string address provided as userp to writer function
        curl_easy_setopt(pCurl, CURLOPT_URL, url);

        res = curl_easy_perform(pCurl); // Perform the actual request

        if (res != CURLE_OK)
        {
            std::cout << "Failed to download github file manifest" << std::endl;
            curl_slist_free_all(list);
            curl_easy_cleanup(pCurl);
            return false;
        }

        curl_slist_free_all(list);
        curl_easy_cleanup(pCurl);

    }
    else
    {
        std::cout << "Failed to init cURL" << std::endl;
    }

    nlohmann::json json = nlohmann::json::parse(responseData);

    return json;

}

bool DownloadWithCURL(const std::string &url, const std::string& savePath, COORD* coords, char* fileName, curl_off_t DataSize)
{

    curl_global_sslset(CURLSSLBACKEND_WOLFSSL, NULL, NULL); //set this to avoid strange ssl error

    CURL* pCurl = curl_easy_init();

    if (pCurl)
    {
        FILE* pFile = fopen(savePath.c_str(), "wb+");

        if (pFile)
        {

            progress progressData{ 0 };

            progressData.DataSize = DataSize;
            progressData.curl = pCurl;
            progressData.coords = coords;
            progressData.fileName = fileName;


            curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1l);
            curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "curl/8.0.1"); //Supply user agent or guthub regects the connection
            curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false); //REMOVE FOR RELEASE
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, writeCurlData); //String append function
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, pFile); //string address provided as userp to writer function
            curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0l);
            curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, PrintCurlDownloadStatus);
            curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, &progressData);
            curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());

            CURLcode res = curl_easy_perform(pCurl);

            if (res != CURLE_OK)
            {
                std::cout << "Failed to download" << savePath << "with curl" << curl_easy_strerror(res) << std::endl;
                curl_easy_cleanup(pCurl);
                fclose(pFile);
                return false;
            }

            curl_easy_cleanup(pCurl);
            fclose(pFile);


            return true;
        }
        else
        {
            std::cout << "Failed to open file to curl download path:" << savePath << std::endl;
            return false;
        }

    }
    else
    {
        std::cout << "Failed to init cURL" << std::endl;
        return false;
    }


}

bool DownloadTorrent(const std::string& savePath, const char* magnetLink, COORD* coords)
{
    lt::settings_pack settings;
    settings.set_int(lt::settings_pack::alert_mask, lt::alert_category::error | lt::alert_category::storage | lt::alert_category::status);
    lt::session torrentSession(settings);

    lt::add_torrent_params torrentParams = lt::parse_magnet_uri(magnetLink);

    torrentParams.save_path = savePath;

    torrentSession.async_add_torrent(std::move(torrentParams));

    lt::torrent_handle torrentHandle;

    //bool isFinishedTorrent = false;
    while (!isTorrentFinished) //Loop while torrent is not finished
    {

        std::vector<lt::alert*> alerts;
        torrentSession.pop_alerts(&alerts);//Get alerts from the running torrent thread

        for (lt::alert const* alert : alerts) //All the alerts that the torrent generated since we last checked
        {

            if (auto AddTorrentAlert = lt::alert_cast<lt::add_torrent_alert>(alert))
            {
                torrentHandle = AddTorrentAlert->handle; //When we get the add torrent alert set the handle back to the added torrent that is running in a different thread
            }

            if (auto TorrentFinishedAlert = lt::alert_cast<lt::torrent_finished_alert>(alert))
            {
                isTorrentFinished = true; //Torrent finished downloading
                FinalTorrentPrint(coords);
            }

            if (auto FileAlert = lt::alert_cast<lt::file_error_alert>(alert))
            {

                std::cout << "\n" << FileAlert->error << std::endl;
                return false;

            }
            
            if (const lt::state_update_alert* stateUpdateAlert = lt::alert_cast<lt::state_update_alert>(alert))
            {
                if (stateUpdateAlert->status.empty())
                {
                    continue;
                }

                const lt::torrent_status& status = stateUpdateAlert->status[0];

                PrintTorrentStatus(status, coords);
            }

        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        torrentSession.post_torrent_updates();
    }
        
    return true;
}