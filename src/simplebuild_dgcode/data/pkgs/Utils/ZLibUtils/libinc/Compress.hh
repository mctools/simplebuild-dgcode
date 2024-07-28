#ifndef Utils_Compress_hh
#define Utils_Compress_hh

#include "Utils/DynBuffer.hh"

namespace ZLibUtils {
  //Will perform zlib compression on indata and places it in output. Note that
  //for performance reasons, output is clear()'ed internally and memory
  //allocated with reserve(). It will thus have size()==0, and you have to
  //inspect outdataLength to see the resulting amount of uncompressed data. This
  //allows the caller to recycle the output buffer.

  //Note from TK ~10 years later: This is not exactly a great way to do it - and
  //perhaps even UB. Moving to custom DynBuffer.

  void compressToBuffer( const char* indata, unsigned indataLength,
                         Utils::DynBuffer<char>& output,
                         unsigned& outdataLength );
  void decompressToBufferNew( const char* indata, unsigned indataLength,
                              Utils::DynBuffer<char>& output );
}

#endif
