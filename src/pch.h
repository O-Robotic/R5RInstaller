#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define TORRENT_NO_DEPRECATE

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <atlbase.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <filesystem>
#include <string.h>

#include <zip.h>
#include "curl/curl.h"


#include "nlohmann/json.hpp"
#include <ShObjIdl_core.h>


#include "libtorrent/session.hpp"
#include "libtorrent/session_params.hpp"
#include "libtorrent/add_torrent_params.hpp"
#include "libtorrent/torrent_handle.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/torrent_status.hpp"
#include "libtorrent/magnet_uri.hpp"


