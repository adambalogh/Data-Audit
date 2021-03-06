#include "audit/server/file_list_handler.h"

#include <string>

#include "folly/io/IOBuf.h"
#include "proxygen/httpserver/ResponseBuilder.h"
#include "nlohmann/json.hpp"

#include "audit/common.h"
#include "audit/providers/local_disk/file_list_source.h"
#include "audit/proto/cpor.pb.h"

using json = nlohmann::json;

using namespace proxygen;

using folly::IOBuf;

namespace audit {
namespace server {

void FileListHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
}

void FileListHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {}

void FileListHandler::onEOM() noexcept {
    auto list = source_.GetFiles();

    proto::FileList file_list;
    for (auto& file : list) {
        proto::File proto_file;
        proto_file.set_name(file.name);
        proto_file.set_size(file.size);
        *file_list.add_files() = proto_file;
    }

    auto bin = file_list.SerializeAsString();
    auto response_body = IOBuf::copyBuffer(bin);

    ResponseBuilder(downstream_)
        .status(200, "OK")
        .body(std::move(response_body))
        .sendWithEOM();
}

void FileListHandler::onUpgrade(UpgradeProtocol protocol) noexcept {}

void FileListHandler::requestComplete() noexcept { delete this; }

void FileListHandler::onError(ProxygenError err) noexcept { delete this; }
}
}
