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

#include "he_plaintext.hpp"

#include <cstring>

#include "logging/ngraph_he_log.hpp"
#include "ngraph/check.hpp"
#include "ngraph/except.hpp"

namespace ngraph::runtime::he {

HEPlaintext::HEPlaintext(const std::initializer_list<double>& values)
    : std::vector<double>(values) {}

HEPlaintext::HEPlaintext(const std::vector<double>& values)
    : std::vector<double>(values) {}

HEPlaintext::HEPlaintext(std::vector<double>&& values)
    : std::vector<double>(std::move(values)) {}

HEPlaintext::HEPlaintext(size_t n, double initial_value)
    : std::vector<double>(n, initial_value) {}

HEPlaintext& HEPlaintext::operator=(const HEPlaintext& v) {
  static_cast<std::vector<double>*>(this)->operator=(v);
  return *this;
}

HEPlaintext& HEPlaintext::operator=(HEPlaintext&& v) noexcept {
  static_cast<std::vector<double>*>(this)->operator=(v);
  return *this;
}

void HEPlaintext::write(void* target, const element::Type& element_type) {
  NGRAPH_CHECK(!empty(), "Input has no values");
  NGRAPH_INFO << "Plain writing to type " << element_type;
  size_t count = this->size();
  size_t type_byte_size = element_type.size();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
  switch (element_type.get_type_enum()) {
    case element::Type_t::f32: {
      std::vector<float> float_values{begin(), end()};
      NGRAPH_INFO << "float_values " << float_values.size();
      NGRAPH_INFO << "count " << count;
      NGRAPH_INFO << float_values[0];
      auto type_values_src = static_cast<const void*>(float_values.data());
      std::memcpy(target, type_values_src, type_byte_size * count);
      break;
    }
    case element::Type_t::f64: {
      auto type_values_src = static_cast<const void*>(this->data());
      std::memcpy(target, type_values_src, type_byte_size * count);
      break;
    }
    case element::Type_t::i32: {
      std::vector<int32_t> int32_values(count);
      for (size_t i = 0; i < count; ++i) {
        int32_values[i] = std::round((*this)[i]);
      }
      auto type_values_src = static_cast<const void*>(int32_values.data());
      std::memcpy(target, type_values_src, type_byte_size * count);
      break;
    }
    case element::Type_t::i64: {
      std::vector<int64_t> int64_values(count);
      for (size_t i = 0; i < count; ++i) {
        int64_values[i] = std::round((*this)[i]);
      }
      auto type_values_src = static_cast<const void*>(int64_values.data());
      std::memcpy(target, type_values_src, type_byte_size * count);
      break;
    }
    case element::Type_t::i8:
    case element::Type_t::i16:
    case element::Type_t::u8:
    case element::Type_t::u16:
    case element::Type_t::u32:
    case element::Type_t::u64:
    case element::Type_t::dynamic:
    case element::Type_t::undefined:
    case element::Type_t::bf16:
    case element::Type_t::f16:
    case element::Type_t::boolean:
      NGRAPH_CHECK(false, "Unsupported element type ", element_type);
  }
#pragma clang diagnostic pop
}

std::ostream& operator<<(std::ostream& os, const HEPlaintext& plain) {
  os << "HEPlaintext( ";
  for (const auto& value : plain) {
    os << value << " ";
  }
  os << ")";
  return os;
}
}  // namespace ngraph::runtime::he