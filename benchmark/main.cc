#include "benchmark/benchmark.h"

#include "audit/client/verify/client.h"
#include "audit/client/verify/no_server_proof_source.h"
#include "audit/providers/local_disk/fetcher.h"
#include "audit/providers/local_disk/file_storage.h"
#include "audit/providers/local_disk/file_tag_source.h"
#include "audit/client/upload/client.h"
#include "audit/client/upload/storage.h"

#include <assert.h>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

using namespace audit;
using namespace audit::providers;
using audit::upload::Storage;

// TODO build benchmark as Release
std::vector<std::string> files;
std::vector<std::string> file_contents;

upload::Client upload_client{
    std::unique_ptr<upload::FileStorage>{new local_disk::FileStorage}};

const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

char RandomChar() { return alphanum[rand() % (sizeof(alphanum) - 1)]; }

void SetUpFiles() {
  // 1MB, 10MB, 100MB
  for (int i = 0; i < 3; ++i) {
    size_t megabytes = pow(10, i);
    size_t bytes = megabytes * 1000 * 1000;
    std::string str(bytes, 'a');
    std::generate(str.begin(), str.end(), RandomChar);

    files.push_back(std::to_string(megabytes));
    file_contents.push_back(str);
  }
}

static void DeleteFiles() {
  for (auto file : files) {
    std::remove(Storage::GetFilePath(file).c_str());
    std::remove(Storage::GetBlockTagFilePath(file).c_str());
    std::remove(Storage::GetFileTagPath(file).c_str());
  }
}

static void Upload(benchmark::State& state) {
  std::stringstream s{std::move(file_contents[state.range_x()])};

  while (state.KeepRunning()) {
    s.clear();
    upload_client.Upload(upload::File{s, files[state.range_x()]}, [](int) {});
  }
}

static void Verify(benchmark::State& state) {
  verify::Stats stats;
  verify::Client client{
      std::unique_ptr<verify::FileTagSource>(new local_disk::FileTagSource),
      std::unique_ptr<verify::ProofSource>(new verify::NoServerProofSource{
          std::unique_ptr<server::FetcherFactory>{
              new local_disk::FetcherFactory}})};
  while (state.KeepRunning()) {
    assert(client.Verify(files[state.range_x()], 100, stats) == true);
  }
}

// Upload must be executed first, otherwise Verify won't have any files to
// verify
BENCHMARK(Upload)->Arg(0)->Arg(1)->Arg(2);

BENCHMARK(Verify)->Arg(0)->Arg(1)->Arg(2);

int main(int argc, char** argv) {
  srand(time(NULL));
  ::benchmark::Initialize(&argc, argv);
  SetUpFiles();
  ::benchmark::RunSpecifiedBenchmarks();
  DeleteFiles();
}
