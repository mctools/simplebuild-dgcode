#include "Core/Types.hh"
#include "zlib.h"
#include "ZLibUtils/Compress.hh"
#include <cassert>
#include <cstdio>
#include <stdexcept>

void ZLibUtils::compressToBuffer(const char* indata,
                                 unsigned indataLength,
                                 std::vector<char>& output,
                                 unsigned& outdataLength)
{
  outdataLength = 0;
  output.clear();
  assert(indataLength<UINT32_MAX);
  //Always embed old original size at first 4 bytes (todo: we could one day
  //compress length=0 data as length=0 vectors, but for now we keep it like
  //this):

  auto outbuf_maxsize = ( indataLength==0 ? 0 : compressBound(indataLength) );
  output.resize( indataLength==0
                 ? sizeof(std::uint32_t)
                 : outbuf_maxsize + 128 + sizeof(std::uint32_t) );//fixme: why + 128??? ??!?!?!?!?!?

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
    output.resize( outdataLength );
    return;
  }

  //something went wrong:
  if (res==Z_MEM_ERROR) {
    printf("ZLibUtils::compressToBuffer ERROR: Z_MEM_ERROR during compression.\n");
  } else if (res==Z_BUF_ERROR) {
    printf("ZLibUtils::compressToBuffer ERROR: Z_BUF_ERROR during compression.\n");
  } else {
    printf("ZLibUtils::compressToBuffer ERROR: Unknown problem during compression.");
  }
  throw std::runtime_error("ZLibUtils::compressToBuffer failed");
}

void ZLibUtils::decompressToBufferNew(const char* indata, unsigned indataLength, std::vector<char>& output)
{
  output.clear();
  if ( indataLength < sizeof(std::uint32_t) ) {
    assert(indataLength == 0 );
    //special case, original buffer was empty, and stored in 0 bytes.
    return;
  }

  const std::uint32_t outdataLength_orig = *(reinterpret_cast<const std::uint32_t*>(indata));
  const unsigned long outdataLength = static_cast<unsigned long>(outdataLength_orig);
  if ( outdataLength_orig == 0 ) {
    assert( indataLength == sizeof(std::uint32_t) );
    return;//original buffer was empty, and used 4 bytes to store that zero length.
  }
  output.resize(outdataLength);//unfortunately we have a needless initialisation here
  if ( output.empty() )
    return;//should never happen, but just to be sure.
  unsigned long outlength = outdataLength;

  const int res = uncompress(reinterpret_cast<unsigned char*>(&(output[0])),
                             &outlength,
                             reinterpret_cast<const unsigned char*>(indata)+sizeof(std::uint32_t),
                             indataLength);
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
  } else if (res==Z_BUF_ERROR) {
    printf("ZLibUtils::decompressToBuffer ERROR: Z_DATA_ERROR during"
           " decompression. Data might be incomplete or corrupted.\n");
  } else {
    printf("ZLibUtils::decompressToBuffer ERROR: Unknown problem during"
           " decompression. Exiting.\n");
  }
  throw std::runtime_error("ZLibUtils::decompressToBuffer failed");
}
