#include "gtest/gtest.h"
#include "audit/client/verify/verify_file.h"

#include <sstream>
#include <vector>

#include "openssl/bn.h"

#include "audit/proto/cpor.pb.h"
#include "audit/client/upload/file.h"
#include "audit/client/upload/block_tagger.h"
#include "audit/server/prover.h"
#include "audit/util.h"
#include "audit/common.h"
#include "audit/test_util.h"

using audit::upload::BlockTagger;
using audit::upload::FileContext;
using audit::upload::TaggingParameters;
using audit::upload::File;
using audit::server::Prover;

using namespace audit::verify;

// TODO cleanup
TEST(Verification, Successful) {
  std::string content{"abcdefgh"};
  unsigned int p = 7883;
  std::vector<unsigned int> alphas{342, 53};
  std::vector<unsigned int> blocks{0, 1};
  std::vector<unsigned int> weights{43, 89};

  File file{MakeFile(content), ""};
  TaggingParameters params{2, 1};

  FileContext context{file, params, make_BN_vector(alphas), BN_new_ptr(p),
                      std::unique_ptr<PRF>{new HMACPRF}};

  proto::PrivateFileTag private_file_tag = context.Proto();

  proto::Challenge challenge;
  challenge.set_allocated_file_tag(
      new proto::PublicFileTag(private_file_tag.public_tag()));
  for (int i = 0; i < blocks.size(); ++i) {
    auto item = challenge.add_items();
    item->set_index(blocks[i]);
    item->set_weight(BignumToString(*BN_new_ptr(weights[i])));
  }

  BlockTagger tagger{context};
  std::vector<proto::BlockTag> tags;
  while (tagger.HasNext()) {
    tags.push_back(tagger.GetNext());
  }

  std::stringstream stream(content);
  MemoryFetcher fetcher{challenge.file_tag(), context, tags, stream};
  Prover prover{fetcher, challenge};
  auto proof = prover.Prove();

  EXPECT_TRUE(VerifyFile<HMACPRF>(private_file_tag, challenge, proof));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
