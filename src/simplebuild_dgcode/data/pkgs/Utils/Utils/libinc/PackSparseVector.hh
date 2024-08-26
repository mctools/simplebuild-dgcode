#ifndef Utils_PackSparseVector_hh
#define Utils_PackSparseVector_hh

#include <algorithm>//std::min
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <functional>

//Utilities for efficiently reading and writing sparse vectors to bytestreams
//(such as file I/O) with high efficiency.
//
//For reference, the packed data consists of a 64bit unsigned integer with the
//vector length, and the contents of individual entries are indicated by 8bit
//unsigned chars whose values indicate:
//
//          0: All remaining values in vector are zero
//      1-128: next N values in vector are non-zero and follow in the next sizeof(value)*N bytes.
//    129-191: next N-128 values in vector are zero.
//    192-255: next (N-191)*64 values in vector are zero.

namespace Utils {
  namespace PackSparseVector {

    template <class TVector>
    void write(const TVector& v,
               std::function<void(unsigned char* buf, unsigned buflen)> dataAcceptor)
    {
      const std::size_t valsize = sizeof(typename TVector::value_type);
      std::uint64_t n64(v.size());
      dataAcceptor((unsigned char*)&n64,sizeof(n64));
      if (!n64)
        return;//empty
      std::size_t i(0), n(v.size());
      int izero(0);
      int inonzero(0);
      for (;i!=n;++i) {
        if (v[i])
          ++inonzero;
        else
          ++izero;
        if (izero&&v[i]) {
          //write out info regarding null elements
          while (izero>=64) {
            unsigned char k = std::min<int>(izero/64,255-192);
            izero -= 64*k;
            k += 191;
            dataAcceptor(&k,1);
          }
          if (izero) {
            assert(izero<64);
            unsigned char l = izero + 128;
            dataAcceptor(&l,1);
            izero = 0;
          }
          assert(izero==0);
        }
        if (inonzero&&(!v[i]||i+1==n)) {
          //write out info regarding non-null elements
          std::size_t ival = (v[i]?i+1:i)-inonzero;//fixme: correct?
          while (inonzero) {
            unsigned char k = std::min<int>(inonzero,128);
            inonzero -= k;
            dataAcceptor(&k,1);
            //write out k elements, from ival to ival+k-1
            std::size_t iilim = ival+k;
            for (std::size_t ii = ival; ii < iilim; ++ii) {
              typename TVector::value_type val = v[ii];
              assert(val);
              //todo: value-buffer for up to 32(?) values at a time
              dataAcceptor((unsigned char*)&val,valsize);
            }
            ival += k;
          }
        }
      }
      if (izero) {
        //vector ends in one or more null elements:
        assert(!inonzero);
        unsigned char k = 0;
        dataAcceptor(&k,1);
      }
    }

    //Extracts the vector contents in embedded in the bytestream provided by
    //dataProvider and adds them to the content already in the provided vector. If
    //v.size()==0, it will be resized to fit the bytestream contents, otherwise
    //the size must match!
    template <class TVector>
    void read(TVector& v,
              std::function<unsigned(unsigned char* buf, unsigned buflen)> dataProvider)
    {
      typedef typename TVector::value_type TValue;
      const char * errmsg = "Read error - unexpected end of data stream.";
      //First read size of vector:
      std::uint64_t length64;
      unsigned nb = dataProvider((unsigned char*)&length64,sizeof(length64));
      if (nb!=sizeof(length64))
        throw std::runtime_error(errmsg);
      size_t length = length64;
      if (v.size()!=length) {
        if (v.size()==0)
          v.resize(length);
        else
          throw std::runtime_error("Dimension mismatch - vector length in provided data and provided vector object differs");
      }
      if (!v.size())
        return;//done

      const int nvalbuf = 64;
      TValue valbuf[nvalbuf];

      size_t i = 0;
      while(i < length) {
        unsigned char k;
        if (dataProvider((unsigned char*)&k,1)!=1)
          throw std::runtime_error(errmsg);
        if (!k)
          return;//rest of vector is empty.
        if (k<=128) {
          //read k values
          while (k) {
            int toread = std::min<int>(k,nvalbuf);
            k -= toread;
            if (dataProvider((unsigned char*)&valbuf[0],toread*sizeof(valbuf[0]))!=toread*sizeof(valbuf[0]))
              throw std::runtime_error(errmsg);
            for (int j = 0; j < toread; ++j) {
              auto val = *reinterpret_cast<TValue*>(&valbuf[j]);
              v[i++] += val;
            }
          }
        } else {
          //skip some zeroes:
          i += ( k<192 ? size_t(k)-128 : (size_t(k)-191)*64 );
        }
      }

    }
  }
}

#endif
