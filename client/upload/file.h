#pragma once

#include <array>
#include <istream>
#include <vector>

#include "openssl/bn.h"

#include "audit/common.h"
#include "audit/client/prf.h"
#include "audit/proto/cpor.pb.h"

namespace audit {
namespace upload {

// Wrapper class for a single file.
struct File {
  File(std::unique_ptr<std::istream> stream, const std::string& file_name);

  // The stream containing the content of the file
  const std::unique_ptr<std::istream> stream;

  // The name of the file, e.g. text.txt
  const std::string file_name;

  // The size of the file in bytes
  const size_t size;
};

// Holds the parameters used for generating tags.
//
// The time and space complexity of verifying a this file's integrity can be
// adjusted by changing num_sectors and sector_size. Storage overhead on the
// server side can be lowered by increasing num_sectors. The storage overhead on
// the server is equal to (1 / num_sectors), relative to the file's size. On the
// other hand, as we increase num_sectors, the size of data that needs to be
// retrieved for each verification increases linearly. It is up to the user to
// decide which factor is more important.
//
struct TaggingParameters {
  TaggingParameters(int num_sectors, size_t sector_size)
      : num_sectors(num_sectors), sector_size(sector_size) {}

  // Returns the expected size of a single block
  size_t block_size() const { return sector_size * num_sectors; }

  // The number of sectors in a block
  const int num_sectors;

  // The size of a sector in bytes
  const size_t sector_size;
};

// FileContext holds all the objects and values we need to tag and store a file
// and its tags.
//
class FileContext {
 public:
  FileContext(File& file, const TaggingParameters& parameters,
              std::vector<BN_ptr> alphas, BN_ptr p, std::unique_ptr<PRF> prf);

  const File& file() const { return file_; }

  const TaggingParameters& parameters() const { return parameters_; }

  int num_blocks() const { return num_blocks_; }

  const std::vector<BN_ptr>& alphas() const { return alphas_; }

  const BIGNUM* p() const { return p_.get(); }

  const PRF& prf() const { return *prf_; }

  // Returns a fully initialized PrivateFileTag, populated with the members of
  // this object
  proto::PrivateFileTag Proto() const;

 private:
  int CalculateNumBlocks();

  File& file_;

  TaggingParameters parameters_;

  // The number of blocks in a file
  const int num_blocks_{0};

  // List of numbers that are used for generating tags,
  // The size of this list must be equal to the number of sectors
  const std::vector<BN_ptr> alphas_;

  // p_ is a large prime number, its size should be equal to sector_size_
  const BN_ptr p_;

  // A pseudo random function used for generating the tags for this file
  const std::unique_ptr<PRF> prf_;
};
}
}
