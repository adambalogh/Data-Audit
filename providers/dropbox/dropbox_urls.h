#pragma once

#include <string>

#include "cpprest/uri.h"

namespace audit {
namespace providers {
namespace dropbox {

const web::uri AUTHORIZE_URL{"https://www.dropbox.com/1/oauth2/authorize"};

const web::uri BASE_URL{"https://content.dropboxapi.com"};

const web::uri DOWNLOAD_PATH{"/2/files/download"};

const web::uri LIST_FOLDER_PATH{"/2/files/list_folder"};

const web::uri UPLOAD_PATH{"/2/files/upload"};

const web::uri AUTH_URL{"https://api.dropboxapi.com"};

const web::uri TOKEN_PATH{"/1/oauth2/token"};
}
}
}
