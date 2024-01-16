#include "Utils/ProgressiveHash.hh"
#include <cassert>
#include "PMurHash.hh"

ProgressiveHash::~ProgressiveHash()
{
  static_assert(sizeof(std::uint32_t)==sizeof(MH_UINT32));
}

void ProgressiveHash::addData(const char* data, unsigned len)
{
  m_data_length += len;
  PMurHash32_Process(&m_hash_state, &m_hash_carry, data, len);
}

std::uint32_t ProgressiveHash::getHash() const
{
  return PMurHash32_Result(m_hash_state,m_hash_carry, m_data_length);
}
