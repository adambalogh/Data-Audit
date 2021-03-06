#pragma once

#include <memory>
#include <vector>

#include "openssl/bn.h"

#include "audit/util.h"
#include "audit/client/upload/block_tagger.h"
#include "audit/client/upload/file.h"
#include "audit/client/upload/storage.h"

namespace audit {
namespace upload {

// This client class should be used to tag and upload files.
class Client {
 public:
  typedef HMACPRF PrfType;

  Client(std::unique_ptr<FileStorage> file_storage)
      : storage_(std::move(file_storage)) {}

  Stats Upload(const File& file, const TaggingParameters& params,
               ProgressBar::CallbackType callback);

 private:
  void Compress(const File& file) const;

  Storage storage_;
};
}
}
