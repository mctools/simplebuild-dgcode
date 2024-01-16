#ifndef Utils_ProgressiveHash_hh
#define Utils_ProgressiveHash_hh

//The algorithm in this class is based on a fast public domain hash from
//http://code.google.com/p/smhasher/. It is preferred to have a code drop-in
//rather than for instance boost hash algs, since we need to be sure the
//algorithm will not change in the future.
//
//The class wraps the original code to make it easy to do a progressive hash
//calculation.
//
//The code is based on the 32bit version of MurmurHash3. 32bit is all we want
//for a simple low-overhead checksum. This is of course not of cryptographic
//quality!
//
//Primary author (of wrapper class): thomas.kittelmann@ess.eu

#include "Core/Types.hh"

class ProgressiveHash {
public:

  typedef std::uint32_t hashtype;

  ProgressiveHash();
  ~ProgressiveHash();

  void addData(const char*, unsigned len);

  hashtype getHash() const;//Hash of all data added so far

  void reset();//Forgets all added data.

private:
  std::uint32_t m_hash_state;
  std::uint32_t m_hash_carry;
  int m_data_length;
};

#include "Utils/ProgressiveHash.icc"

#endif
