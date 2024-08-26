#include "Utils/PackSparseVector.hh"
#include "Utils/DelayedAllocVector.hh"
#include <vector>
#include <climits>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <sstream>

namespace {
  class SimpleRandGen {
    // very simple multiply-with-carry rand gen
    // (http://en.wikipedia.org/wiki/Random_number_generation)
  public:
    SimpleRandGen()
      : m_w(117),/* must not be zero, nor 0x464fffff */
        m_z(11713)/* must not be zero, nor 0x9068ffff */
    {
    }
    ~SimpleRandGen(){}
    std::uint32_t shoot()
    {
      m_w = 18000 * (m_w & 65535) + (m_w >> 16);
      m_z = 36969 * (m_z & 65535) + (m_z >> 16);
      return (m_z << 16) + m_w;  /* 32-bit result */
    }
  private:
    std::uint32_t m_w;
    std::uint32_t m_z;
  };

  class MemoryBuffer {
  public:

    MemoryBuffer() : m_idx(0) {}
    ~MemoryBuffer() {}

    void clear() { m_idx = 0; m_buffer.clear(); }

    void write(unsigned char* buf, unsigned buflen)
    {
      for (unsigned i = 0; i < buflen; ++i)
        m_buffer.push_back(buf[i]);
    }

    void read_rewind() { m_idx = 0; }
    unsigned read(unsigned char* buf, unsigned buflen)
    {
      unsigned i = 0;
      while(i<buflen&&m_idx<m_buffer.size())
        buf[i++] = m_buffer[m_idx++];
      return i;
    }

    std::function<void(unsigned char*, unsigned)> write_function()
    { return std::bind(&MemoryBuffer::write,this,std::placeholders::_1,std::placeholders::_2); }

    std::function<unsigned(unsigned char*, unsigned)> read_function()
    { read_rewind(); return std::bind(&MemoryBuffer::read,this,std::placeholders::_1,std::placeholders::_2); }

    void print()
    {
      printf("Buflen %lu: ", (unsigned long)m_buffer.size());
      for (size_t i = 0; i < m_buffer.size(); ++i) {
        printf("%02x ", (unsigned char)m_buffer[i]);
        if (i%4==3)
          printf(": ");
      }
      printf("\n");
    }

    bool operator==( const MemoryBuffer & o ) const
    { return m_buffer==o.m_buffer; }

  private:
    MemoryBuffer( const MemoryBuffer & );
    MemoryBuffer & operator= ( const MemoryBuffer & );
    std::vector<char> m_buffer;
    size_t m_idx;
  };

  void test(const std::vector<double>& v) {
    MemoryBuffer buf;
    Utils::PackSparseVector::write(v,buf.write_function());
    Utils::DelayedAllocVector<double> v2;
    static int toresize = 0;
    if (toresize++%2)
      v2.resize(v.size());
    Utils::PackSparseVector::read(v2,buf.read_function());
    if (v2.size()!=v.size()) {
      throw std::runtime_error("Unexpected vector length\n");
    }
    for (unsigned i = 0; i < v2.size(); ++i) {
      if (v2[i]!=v[i]) {
        std::ostringstream ss;
        ss << "Unexpected value (expected "<<v.at(i)<<")";
        throw std::runtime_error(ss.str());
      }
    }
  }
}

int main(int,char**) {

  for (unsigned i = 0; i < 10; ++i) {
    std::vector<double> v(10);
    v[i] = 5;
    test(v);
  }
  {
    std::vector<double> v(10);
    v[3] = 5;
    v[4] = 5;
    test(v);
  }
  {
    std::vector<double> v(150);
    for (unsigned i = 5; i < 135; ++i)
      v[i] = i*0.1;
    test(v);
  }

  {
    std::vector<double> v(1000);
    for (unsigned i = 100; i < 500; ++i)
      v[i] = i*0.1;
    test(v);
  }
  {
    std::vector<double> v(1000);
    for (unsigned i = 100; i < 227; ++i)
      v[i] = i*0.1;
    test(v);
  }
  {
    std::vector<double> v(1000);
    for (unsigned i = 100; i < 228; ++i)
      v[i] = i*0.1;
    test(v);
  }
  {
    std::vector<double> v(1000);
    for (unsigned i = 100; i < 229; ++i)
      v[i] = i*0.1;
    test(v);
  }
  {
    std::vector<double> v(228);
    for (unsigned i = 100; i < 228; ++i)
      v[i] = i*0.1;
    test(v);
  }
  {
    std::vector<double> v(229);
    for (unsigned i = 100; i < 228; ++i)
      v[i] = i*0.1;
    test(v);
  }

  //random tests:
  SimpleRandGen r;
  int lengths[] = {1, 5, 20, 200, 999, 2000000};
  int nfills[] = {0, 1, 2, 20, 200, 999};
  for (unsigned jj = 0; jj < 2; ++jj) {
  for (unsigned ilength = 0; ilength < sizeof(lengths)/sizeof(lengths[0]); ++ilength) {
    unsigned length = lengths[ilength];
    for (unsigned infill = 0; infill < sizeof(nfills)/sizeof(nfills[0]); ++infill) {
      unsigned nfill = nfills[infill];
      std::vector<double> v(length);
      for (unsigned ifill = 0; ifill < nfill; ++ifill)
        v[r.shoot()%length] = ifill*0.1 + 0.00000017;
      test(v);
    }
  }
  }
  return 0;
}
