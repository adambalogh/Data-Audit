#pragma once

#include "cpprest/http_client.h"

#include "audit/client/verify/proof_source.h"
#include "audit/providers/azure/urls.h"

namespace audit {
namespace providers {
namespace azure {

class ProofSource : public ::audit::verify::ProofSource {
 public:
  virtual proto::Proof GetProof(const proto::Challenge& challenge) = 0;

 private:
  web::http::client::http_client client_{BASE_URL};
};
}
}
}
