
#include "Utils/DelayedAllocVector.hh"

#include <algorithm>
#include <utility>

int main(int,char**) {

  //test std::swap
  Utils::DelayedAllocVector<double> a(100);
  Utils::DelayedAllocVector<double> b(300);
  a[10]=2.0;
  b[110]=4.0;
  if (a[10]!=2.0||b[110]!=4.0||a[0]!=0.0||b[0]!=0.0)
    return 1;
  std::swap(a,b);
  if (b[10]!=2.0||a[110]!=4.0||b[0]!=0.0||a[0]!=0.0)
    return 1;

  //test block allocations, merge and reset:
  Utils::DelayedAllocVector<double,10> c(101);
  Utils::DelayedAllocVector<double,10> d(101);

  c[5] = 2.0;//block 0
  c[35] = 4.0;//block 3
  c[55] = 8.0;//block 5
  d[35] = 16.0;//block 3
  d[56] = 32.0;//block 5
  d[100] = 64.0;//block 11
  c.merge(d);
  if (d[5]||d[35]||d[55]||d[56]||d[100])
    return 1;
  if (c[5]!=2.0||c[35]!=20.0||c[55]!=8.0||c[56]!=32||c[100]!=64)
    return 1;
  c.reset();
  if (c[5]||c[35]||c[55]||c[56]||c[100])
    return 1;

  //test that we work fine even with "infinite" blocksize:
  Utils::DelayedAllocVector<double,4000000000> e(10);
  e[0] = 1.0;
  if (e[0]!=1.0||e.size()!=10)
    return 1;

  //test that we work fine even with "tiny" blocksize:
  Utils::DelayedAllocVector<double,1> f(10);
  f[0] = 1.0;
  if (f[0]!=1.0||a[1]||f.size()!=10)
    return 1;

  return 0;
}
