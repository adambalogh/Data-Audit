#pragma once

#include "audit/client/verify/file_tag_source.h"
#include "audit/providers/dropbox/dropbox_client.h"
#include "audit/providers/dropbox/token_source.h"

#include "cpprest/uri.h"

namespace audit {
namespace providers {
namespace dropbox {

class FileTagSource : public verify::FileTagSource, public DropboxClient {
 public:
  FileTagSource(TokenSourceInterface& token_source)
      : DropboxClient(token_source) {}

  proto::PrivateFileTag GetFileTag(const std::string& file_name) override;
};
}
}
}
