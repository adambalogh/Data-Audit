#pragma once

#include <array>
#include <istream>
#include <vector>

#include "cryptopp/integer.h"
#include "cryptopp/hmac.h"
#include "cryptopp/sha.h"
#include "cryptopp/aes.h"
#include "openssl/bn.h"

#include "audit/common.h"
#include "audit/proto/cpor.pb.h"

namespace audit {

// A File holds important information about the file and parameters for the
// tagging.
//
// The complexity of verifying a this file's integrity can be adjusted by
// num_sectors and sector_size parameters. Storage overhead on the server side
// can be lowered by increasing num_sectors. The storage overhead on the server
// is equal to (1 / num_sectors), relative to the file's size. On the other
// hand, as we increase num_sectors, the size of data that needs to be
// retrieved for each verification increases linearly. It is up to the user to
// decide which factor is more important.
class File {
 public:
  // Constructs a File.
  //
  // @param file: the file we want to work with
  // @param num_sectors: the number of sectors in a block
  // @param sector_size: the size of a single sector in bytes
  // @param alphas: list of Bignumbers, the size must be equal to num_sectors
  // @param p: a large prime number
  //
  File(std::istream& stream, int num_sectors, size_t sector_size,
       std::vector<BN_ptr> alphas, BN_ptr p);

  std::istream& stream() const { return stream_; }

  int num_blocks() const { return num_blocks_; }
  int num_sectors() const { return num_sectors_; }
  size_t sector_size() const { return sector_size_; }
  size_t block_size() const { return num_sectors_ * sector_size_; }

  const std::vector<BN_ptr>& alphas() const { return alphas_; }
  const BIGNUM* p() const { return p_.get(); }

 private:
  void CalculateNumBlocks();

  // The stream containing the content of the file
  std::istream& stream_;

  // The number of blocks in a file
  int num_blocks_{0};

  // The number of sectors in a block
  int num_sectors_;

  // The size of a sector in bytes
  size_t sector_size_;

  // List of numbers that are used for generating tags,
  // The size of this list must be equal to the number of sectors
  const std::vector<BN_ptr> alphas_;

  // p_ is a large prime number, its size should be equal to sector_size_
  BN_ptr p_;
};
}