#include "audit/client/local_disk_storage.h"

#include <array>
#include <iostream>
#include <fstream>
#include <string>

#include "audit/proto/cpor.pb.h"

namespace audit {

void LocalDiskStorage::StoreBlockTag(const proto::BlockTag& tag) {
  std::ofstream tag_file{
      file_dir_ + "tags" + file_name_ + std::to_string(tag.index()),
      std::ofstream::binary};
  tag.SerializeToOstream(&tag_file);
}

void LocalDiskStorage::StoreFileTag(const proto::PrivateFileTag& file_tag) {
  std::ofstream tag_file{file_dir_ + "file_tag" + file_name_,
                         std::ofstream::binary};
  file_tag.SerializeToOstream(&tag_file);
}

void LocalDiskStorage::StoreFile(std::istream& file) {
  std::ofstream out_file{file_dir_ + file_name_, std::ofstream::binary};
  std::array<char, 1000> buffer;

  while (file.read(buffer.data(), buffer.size()).gcount()) {
    out_file.write(buffer.data(), file.gcount());
  }
}
}