//*****************************************************************************
// Copyright 2018-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#pragma once

#include "aby/kernel/relu_aby.hpp"
#include "abycore/aby/abyparty.h"
#include "abycore/circuit/booleancircuits.h"
#include "abycore/circuit/share.h"
#include "abycore/sharing/sharing.h"
#include "he_type.hpp"
#include "logging/ngraph_he_log.hpp"
#include "ngraph/except.hpp"
#include "ngraph/util.hpp"
#include "seal/he_seal_backend.hpp"

namespace ngraph {
namespace he {
class HESealExecutable;
}
namespace aby {

class ABYExecutor {
 public:
  ABYExecutor(std::string role, std::string mpc_protocol,
              const he::HESealExecutable& he_seal_executable,
              std::string hostname = std::string("localhost"),
              std::size_t port = 7766, uint64_t security_level = 128,
              uint32_t bit_length = 64, uint32_t num_threads = 2,
              std::string mg_algo_str = std::string("MT_OT"),
              uint32_t reserve_num_gates = 65536,
              const std::string& circuit_directiory = "")
      : m_num_threads{num_threads}, m_he_seal_executable{he_seal_executable} {
    std::map<std::string, e_role> role_map{{"server", SERVER},
                                           {"client", CLIENT}};

    auto role_it = role_map.find(ngraph::to_lower(role));
    NGRAPH_CHECK(role_it != role_map.end(), "Unknown role ", role);
    m_role = role_it->second;

    std::map<std::string, e_sharing> protocol_map{{"yao", S_YAO},
                                                  {"gmw", S_BOOL}};

    auto protocol_it = protocol_map.find(ngraph::to_lower(mpc_protocol));
    NGRAPH_CHECK(role_it != role_map.end(), "Unknown role ", role);
    NGRAPH_CHECK(protocol_it != protocol_map.end(), "Unknown mpc_protocol ",
                 mpc_protocol);
    m_aby_gc_protocol = protocol_it->second;

    NGRAPH_CHECK(mg_algo_str == "MT_OT", "Unknown mg_algo_str ", mg_algo_str);
    m_mt_gen_alg = MT_OT;

    NGRAPH_CHECK(security_level == 128, "Unsupported security level ",
                 security_level);
    m_security_level = security_level;

    m_ABYParty = new ABYParty(m_role, hostname, port,
                              get_sec_lvl(m_security_level), bit_length,
                              m_num_threads, m_mt_gen_alg, reserve_num_gates);
    // TODO: connect and base OTs?
    //  m_ABYParty_server->ConnectAndBaseOTs();
    NGRAPH_HE_LOG(1) << "Started ABYParty with role " << role;

    m_random_distribution =
        std::uniform_int_distribution<int64_t>{0, m_rand_max};
  }

  ~ABYExecutor() {
    // TODO: delete ABYParty
  }

  // generate_gc_input_mask(Shape{m_batch_size, element_count});
  // generate_gc_output_mask(Shape{m_batch_size, element_count},
  //                        m_lowest_coeff_modulus / 2);

  ABYParty* get_aby_party() { return m_ABYParty; }

  BooleanCircuit* get_circuit() {
    NGRAPH_HE_LOG(1) << " Starting ABY Sharing on the server";

    std::vector<Sharing*>& sharings = m_ABYParty->GetSharings();
    auto circ = dynamic_cast<BooleanCircuit*>(
        sharings[m_aby_gc_protocol]->GetCircuitBuildRoutine());
    return circ;
  }

  void process_unknown_relu_ciphers_batch(
      const std::vector<he::HEType>& cipher_batch) {
    NGRAPH_HE_LOG(3) << "process_unknown_relu_ciphers_batch ";
  }

 private:
  size_t m_num_threads;
  bool m_mask_gc_inputs;
  bool m_mask_gc_outputs;

  e_role m_role;
  e_sharing m_aby_gc_protocol;
  e_mt_gen_alg m_mt_gen_alg;
  uint32_t m_aby_bitlen;
  uint64_t m_security_level;
  ABYParty* m_ABYParty;

  const he::HESealExecutable& m_he_seal_executable;

  std::default_random_engine m_random_generator;
  int64_t m_rand_max;
  std::uniform_int_distribution<int64_t> m_random_distribution;
  // std::shared_ptr<HETensor> m_gc_input_mask;
  // std::shared_ptr<HETensor> m_gc_output_mask;
};

}  // namespace aby
}  // namespace ngraph