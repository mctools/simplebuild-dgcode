#ifndef MeshTests_tempworkarounds2_hh
#define MeshTests_tempworkarounds2_hh

//Various utilities for working around incomplete C++11 support on some old
//platforms (like slc6 with gcc 4.4). This header is destined for removal one
//day. This file is for the unit tests.

#include "Mesh/MeshFiller.hh"

namespace Mesh {

  template<int NDIM, class TStorage>
  inline void doFill(MeshFiller<NDIM,TStorage>& mf, double val, std::initializer_list<double> posil)
  {
    double pos[NDIM];
    assert(posil.size()==NDIM);
    auto it = posil.begin();
    for (int i = 0; i < NDIM; ++i)
      pos[i] = *(it++);
    mf.fill(val,pos);
  }
  template<int NDIM, class TStorage>
  inline void doFill(MeshFiller<NDIM,TStorage>& mf, double val, std::initializer_list<double> posil,std::initializer_list<double> posil2)
  {
    double pos[NDIM];
    double pos2[NDIM];
    assert(posil.size()==NDIM&&posil2.size()==NDIM);
    auto it = posil.begin();
    auto it2 = posil2.begin();
    for (int i = 0; i < NDIM; ++i) {
      pos[i] = *(it++);
      pos2[i] = *(it2++);
    }
    mf.fill(val,pos,pos2);
  }
  template<int NDIM, class TStorage>
  inline double getCellContent(const MeshFiller<NDIM,TStorage>& mf, std::initializer_list<long> cellil)
  {
    long cell[NDIM];
    assert(cellil.size()==NDIM);
    auto it = cellil.begin();
    for (int i = 0; i < NDIM; ++i)
      cell[i] = *(it++);
    return mf.cellContent(cell);
  }
  template<int NDIM, class TStorage>
  inline double contentAt(const MeshFiller<NDIM,TStorage>& mf, std::initializer_list<double> p)
  {
    double pp[NDIM];
    assert(p.size()==NDIM);
    auto it = p.begin();
    for (int i = 0; i < NDIM; ++i)
      pp[i] = *(it++);
    return mf.contentAt(pp);
  }
  template<int NDIM, class TStorage>
  inline double contentAt(const MeshFiller<NDIM,TStorage>& mf,
                          std::initializer_list<double> p1,std::initializer_list<double> p2)
  {
    double pp1[NDIM];
    double pp2[NDIM];
    assert(p1.size()==NDIM);
    assert(p2.size()==NDIM);
    auto it1 = p1.begin();
    for (int i = 0; i < NDIM; ++i)
      pp1[i] = *(it1++);
    auto it2 = p2.begin();
    for (int i = 0; i < NDIM; ++i)
      pp2[i] = *(it2++);
    return mf.contentAt(pp1,pp2);
  }


}
#endif
