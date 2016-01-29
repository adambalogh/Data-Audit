#include "audit/client/upload/client.h"

#include "openssl/bn.h"

#include "audit/util.h"
#include "audit/client/prf.h"
#include "audit/client/upload/block_tagger.h"
#include "audit/client/upload/block_tag_serializer.h"
#include "audit/client/upload/file.h"
#include "audit/client/upload/storage.h"
#include "audit/proto/cpor.pb.h"

namespace audit {
namespace upload {

Stats Client::Upload(const File& file) {
  TaggingParameters params{10, 128};

  BN_ptr p{BN_new(), ::BN_free};
  BN_generate_prime_ex(p.get(), params.sector_size * 8, false, NULL, NULL,
                       NULL);

  CryptoNumberGenerator random_gen;
  std::vector<BN_ptr> alphas;
  for (int i = 0; i < params.num_sectors; ++i) {
    alphas.push_back(random_gen.GenerateNumber(*p));
  }

  FileContext context{file, params, std::move(alphas), std::move(p),
                      std::unique_ptr<PRF>(new HMACPRF)};

  StatsListener stats_;
  ProgressBarListener progress_{file.size, context.parameters().block_size(),
                                context.num_blocks()};

  StorageListenerChain listener{{&stats_, &progress_}};

  BlockTagger tagger{context};
  BlockTagSerializer serializer{file.file_name};
  while (tagger.HasNext()) {
    serializer.Add(tagger.GetNext());
  }

  auto private_tag = context.Proto();
  *private_tag.mutable_block_tag_map() = serializer.Done();

  storage_->StoreBlockTagFile(file, serializer.FileName(), listener);
  storage_->StoreFileTag(file, private_tag, listener);
  storage_->StoreFile(file, listener);

  return stats_.GetStats();
}
}
}
