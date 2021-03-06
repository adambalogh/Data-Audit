#include "audit/providers/dropbox/token_source.h"

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "cpprest/uri.h"
#include "cpprest/http_client.h"
#include "nlohmann/json.hpp"

#include "audit/common.h"
#include "audit/providers/dropbox/dropbox_urls.h"

using json = nlohmann::json;

using web::uri;
using web::uri_builder;
using web::http::client::http_client;
using web::http::http_request;

namespace audit {
namespace providers {
namespace dropbox {

const std::string TokenSource::SECRETS_FILE{application_dir + "secrets.json"};

const std::string TokenSource::TOKEN_FILE{application_dir + "token.json"};

TokenSource::TokenSource()
    : client_id_(GetClientId()), client_secret_(GetClientSecret()) {
  GetTokenFromFile();
}

std::string TokenSource::GetValueFromSecret(const std::string& key) {
  std::ifstream secrets_file{SECRETS_FILE};
  if (!secrets_file) {
    throw std::runtime_error(
        "Could not open file containing Dropbox client secrets (" +
        SECRETS_FILE + ")");
  }
  try {
    auto secrets = json::parse(secrets_file);
    return secrets.at(key);
  } catch (std::exception& e) {
    throw std::runtime_error(std::string{"Error parsing secrets file: "} +
                             e.what());
  }
}

std::string TokenSource::GetClientId() {
  return GetValueFromSecret("client_id");
}

std::string TokenSource::GetClientSecret() {
  return GetValueFromSecret("client_secret");
}

bool TokenSource::NeedToAuthorize() { return !has_token_; }

bool TokenSource::GetTokenFromFile() {
  std::ifstream in_file{TOKEN_FILE};
  if (!in_file) {
    return false;
  }
  try {
    auto token_file = json::parse(in_file);
    token_ = token_file.at("token").get<std::string>();
    has_token_ = true;
    return true;
  } catch (...) {
    return false;
  }
}

void TokenSource::SaveTokenToFile() {
  if (!has_token_) {
    throw std::logic_error(
        "The token must be acquired before it can be saved to file");
  }
  json token_file;
  token_file["token"] = token_;
  std::ofstream out_file{TOKEN_FILE};
  if (!out_file) {
    throw std::runtime_error("Could not open token file. (" + TOKEN_FILE + ")");
  }
  out_file << token_file.dump();
}

std::string TokenSource::GetAuthorizeUrl() const {
  uri_builder builder{AUTHORIZE_URL};
  builder.append_query("response_type", "code")
      .append_query("client_id", client_id_);
  auto authorize_url = builder.to_uri();

  return authorize_url.to_string();
}

void TokenSource::ExchangeCodeForToken(const std::string& code) {
  std::stringstream request_body;
  request_body << "code=" << uri::encode_data_string(code)
               << "&grant_type=authorization_code"
               << "&client_id=" << uri::encode_data_string(client_id_)
               << "&client_secret=" << uri::encode_data_string(client_secret_);

  http_request request("POST");
  request.set_request_uri(TOKEN_PATH);
  request.headers().add("Content-Type", "application/x-www-form-urlencoded");
  request.set_body(request_body.str());

  http_client client{AUTH_URL};
  auto http_response = client.request(request).get();
  auto response = json::parse(http_response.extract_string().get());

  // If request was invalid, "error" field will be set
  auto error = response.find("error");
  if (error != response.end()) {
    throw std::runtime_error("Unable to authorize app with Dropbox: " +
                             (*error).get<std::string>() + ". Description: " +
                             response["error_description"].get<std::string>());
  }

  has_token_ = true;
  token_ = response.at("access_token");
  SaveTokenToFile();
}

std::string TokenSource::GetToken() {
  if (!has_token_) {
    throw std::logic_error(
        "You must call ExchangeCodeForToken before you can use GetToken");
  }
  return token_;
}
}
}
}
