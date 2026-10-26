#include "Core/Types.hh"
#include "zlib.h"
#include "ZLibUtils/Compress.hh"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>

void ZLibUtils::compressToBuffer(const char* indata,
                                 unsigned indataLength,
                                 Utils::DynBuffer<char>& output,
                                 unsigned& outdataLength)
{
  outdataLength = 0;
  assert(indataLength<UINT32_MAX);
  //Always embed old original size at first 4 bytes (todo: we could one day
  //compress length=0 data as length=0 vectors, but for now we keep it like
  //this):

  auto outbuf_maxsize = ( indataLength==0 ? 0 : compressBound(indataLength) );
  output.resize_without_init( sizeof(std::uint32_t) + outbuf_maxsize );

  auto out_data = reinterpret_cast<unsigned char*>(output.data());
  auto in_data = reinterpret_cast<const unsigned char*>(indata);

  *(reinterpret_cast<std::uint32_t*>(out_data)) = std::uint32_t(indataLength);

  if (indataLength == 0) {
    //empty buffer, nothing to compress
    outdataLength = sizeof(std::uint32_t);
    return;
  }

  unsigned long outlength = outbuf_maxsize;
  int res = compress( out_data + sizeof(std::uint32_t),
                      &outlength,
                      in_data, indataLength);
  if (res==Z_OK) {
    assert(outlength<UINT_MAX-sizeof(std::uint32_t));
    outdataLength = static_cast<unsigned>(outlength) + sizeof(std::uint32_t);
    assert( outdataLength <= output.size() );
    output.resize_without_init( outdataLength );
    return;
  }

  //something went wrong:
  output.clear();
  if (res==Z_MEM_ERROR) {
    printf("ZLibUtils::compressToBuffer ERROR: Z_MEM_ERROR during compression.\n");
  } else if (res==Z_BUF_ERROR) {
    printf("ZLibUtils::compressToBuffer ERROR: Z_BUF_ERROR during compression.\n");
  } else {
    printf("ZLibUtils::compressToBuffer ERROR: Unknown problem during compression.");
  }
  throw std::runtime_error("ZLibUtils::compressToBuffer failed");
}

namespace {
  //Size of the uncompressed data (stored in the first 4 bytes):
  std::uint32_t uncompressedSize( const char* indata, unsigned indataLength )
  {
    if ( indataLength < sizeof(std::uint32_t) ) {
      assert(indataLength == 0 );
      //special case, original buffer was empty, and stored in 0 bytes.
      return 0;
    }
    std::uint32_t n;
    std::memcpy( &n, indata, sizeof(n) );
    //NB: n=0 means that the original buffer was empty, and used 4 bytes to
    //store that zero length.
    assert( n != 0 || indataLength == sizeof(std::uint32_t) );
    return n;
  }

  void decompressInto( const char* indata, unsigned indataLength,
                       char* output, std::uint32_t outdataLength_orig )
  {
    unsigned long outlength = static_cast<unsigned long>(outdataLength_orig);
    const int res = uncompress(reinterpret_cast<unsigned char*>(output),
                               &outlength,
                               reinterpret_cast<const unsigned char*>(indata)+sizeof(std::uint32_t),
                               indataLength-sizeof(std::uint32_t));
    if (res==Z_OK) {
      assert(outlength<UINT_MAX-sizeof(std::uint32_t));
      if ( outlength != static_cast<unsigned long>(outdataLength_orig) ) {
        throw std::runtime_error("ZLibUtils::decompressToBuffer ERROR:"
                                 " Weird error during decompression.");
      }
      return;
    }
    //something went wrong:
    if (res==Z_MEM_ERROR) {
      printf("ZLibUtils::decompressToBuffer ERROR: Z_MEM_ERROR during"
             " decompression.\n");
    } else if (res==Z_BUF_ERROR) {
      printf("ZLibUtils::decompressToBuffer ERROR: Z_BUF_ERROR during"
             " decompression.\n");
    } else if (res==Z_DATA_ERROR) {
      printf("ZLibUtils::decompressToBuffer ERROR: Z_DATA_ERROR during"
             " decompression. Data might be incomplete or corrupted.\n");
    } else {
      printf("ZLibUtils::decompressToBuffer ERROR: Unknown problem during"
             " decompression. Exiting.\n");
    }
    throw std::runtime_error("ZLibUtils::decompressToBuffer failed");
  }
}

void ZLibUtils::decompressToBufferNew(const char* indata, unsigned indataLength, Utils::DynBuffer<char>& output)
{
  output.clear();
  const std::uint32_t n = uncompressedSize( indata, indataLength );
  if ( n == 0 )
    return;
  output.resize_without_init(n);
  decompressInto( indata, indataLength, output.data(), n );
}

std::size_t ZLibUtils::decompressToFixedBuffer( const char* indata, unsigned indataLength,
                                               char* output, std::size_t capacity )
{
  const std::uint32_t n = uncompressedSize( indata, indataLength );
  if ( n > capacity )
    throw std::length_error("ZLibUtils::decompressToFixedBuffer ERROR: Output buffer is too small.");
  if ( n )
    decompressInto( indata, indataLength, output, n );
  return n;
}
