#include "Core/Types.hh"
#include "zlib.h"
#include "ZLibUtils/Compress.hh"
#include <cassert>
#include <cstdio>
#include <stdexcept>

void ZLibUtils::compressToBuffer(const char* indata, unsigned indataLength, std::vector<char>& output,unsigned& outdataLength)
{
  outdataLength = 0;
  output.clear();
  assert(indataLength<UINT32_MAX);
  output.reserve(indataLength==0?sizeof(std::uint32_t):compressBound(indataLength) + 128 + sizeof(std::uint32_t));
  *(reinterpret_cast<std::uint32_t*>(&output[0])) = std::uint32_t(indataLength);//embed old original size at first 4 bytes
  if (indataLength==0) {
    //empty buffer, nothing to compress
    outdataLength = sizeof(std::uint32_t);
    return;
  }
  unsigned long outlength = output.capacity()-1 - sizeof(std::uint32_t);
  int res = compress(reinterpret_cast<unsigned char*>(&(output[sizeof(std::uint32_t)])),&outlength,
                     reinterpret_cast<const unsigned char*>(indata),indataLength);
  if (res==Z_OK) {
    assert(outlength<UINT_MAX-sizeof(std::uint32_t));
    outdataLength = static_cast<unsigned>(outlength) + sizeof(std::uint32_t);
    return;
  }
  //something went wrong:
  if (res==Z_MEM_ERROR) {
    printf("ZLibUtils::compressToBuffer ERROR: Z_MEM_ERROR during compression. Exiting.\n");
  } else if (res==Z_BUF_ERROR) {
    printf("ZLibUtils::compressToBuffer ERROR: Z_BUF_ERROR during compression. Exiting.\n");
  } else {
    printf("ZLibUtils::compressToBuffer ERROR: Unknown problem during compression. Exiting.\n");
  }
  throw std::runtime_error("ZLibUtils::compressToBuffer failed");
}

void ZLibUtils::decompressToBuffer(const char* indata, unsigned indataLength, std::vector<char>& output,unsigned& outdataLength)
{
  output.clear();
  assert(indataLength>=sizeof(std::uint32_t));
  std::uint32_t outdataLength_orig = *(reinterpret_cast<const std::uint32_t*>(indata));
  outdataLength = outdataLength_orig;
  if (outdataLength_orig==0&&indataLength==sizeof(std::uint32_t))
    {
      return;//original buffer was empty
    }
  output.clear();
  output.reserve(outdataLength);
  unsigned long outlength = outdataLength;
  int res = uncompress(reinterpret_cast<unsigned char*>(&(output[0])),&outlength,
                       reinterpret_cast<const unsigned char*>(indata)+sizeof(std::uint32_t),indataLength);
  if (res==Z_OK) {
    assert(outlength<UINT_MAX-sizeof(std::uint32_t));
    outdataLength = static_cast<unsigned>(outlength);
    if (outdataLength!=outdataLength_orig) {
      printf("ZLibUtils::decompressToBuffer ERROR: Weird error during decompression. Exiting.\n");
      exit(1);
    }
    return;
  }
  //something went wrong:
  if (res==Z_MEM_ERROR) {
    printf("ZLibUtils::decompressToBuffer ERROR: Z_MEM_ERROR during decompression. Exiting.\n");
  } else if (res==Z_BUF_ERROR) {
    printf("ZLibUtils::decompressToBuffer ERROR: Z_BUF_ERROR during decompression. Exiting.\n");
  } else if (res==Z_BUF_ERROR) {
    printf("ZLibUtils::decompressToBuffer ERROR: Z_DATA_ERROR during decompression. Data might be incomplete or corrupted. Exiting.\n");
  } else {
    printf("ZLibUtils::decompressToBuffer ERROR: Unknown problem during decompression. Exiting.\n");
  }
  throw std::runtime_error("ZLibUtils::decompressToBuffer failed");
}
