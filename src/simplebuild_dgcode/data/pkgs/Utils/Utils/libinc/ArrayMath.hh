#ifndef Utils_ArrayMath_hh
#define Utils_ArrayMath_hh

//Functions which performs mathematical operations on "3-vectors" consisting of
//either pointers to float[3] or double[3] arrays.

#include <cmath>

namespace Utils {

  template <class floatT1, class floatT2>
  inline double dot(const floatT1*v1,const floatT2*v2) { return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]; }

  template <class floatT1>
  inline double phi(const floatT1*v1)//returns in -pi..+pi
  {
    // Could add extra protection here:
    // if (!v1[0]&&!v2[0])
    //   return 0.0;//NaN technically
    return atan2(v1[1],v1[0]);
  }

  template <class floatT1, class floatT2>
  inline double delta_phi(const floatT1*v1,const floatT2*v2)//returns in -pi..+pi
  {
    double phi1 = phi(v1);
    double phi2 = phi(v2);
    double d = phi2-phi1;
    if (d <  - M_PI ) return d + 2 * M_PI;
    if (d >  M_PI ) return d - 2 * M_PI;
    return d;
  }

  template <class floatT1>
  inline double theta(const floatT1*v)
  {
    double r2 = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
    if (!r2)
      return 0;//NaN technically
    double costheta=v[2]/sqrt(r2);
    if (costheta>=1.0) return 0.0;
    if (costheta<=-1.0) return M_PI;
    return acos(costheta);
  }

  template <class floatT1, class floatT2>
  inline double costheta(const floatT1*v1,const floatT2*v2)
  {
    double cth = dot(v1,v2)/sqrt(dot(v1,v1)*dot(v2,v2));
    if (cth>1.0) return 1.0;
    if (cth<-1.0) return -1.0;
    return cth;
  }

  template <class floatT1, class floatT2>
  inline double theta(const floatT1*v1,const floatT2*v2)
  {
    double cth = dot(v1,v2)/sqrt(dot(v1,v1)*dot(v2,v2));
    if (cth>1.0) return 0.0;
    if (cth<-1.0) return M_PI;
    return acos(cth);
  }

  template <class floatT1, class floatT2>
  inline void add(const floatT1*v1,const floatT2*v2, double* result)
  {
    result[0] = v1[0]+v2[0];
    result[1] = v1[1]+v2[1];
    result[2] = v1[2]+v2[2];
  }

  template <class floatT1, class floatT2>
  inline void subtract(const floatT1*v1,const floatT2*v2, double* result)
  {
    result[0] = v1[0]-v2[0];
    result[1] = v1[1]-v2[1];
    result[2] = v1[2]-v2[2];
  }

  template <class floatT1, class floatT2>
  inline double dist(const floatT1*v1,const floatT2*v2)
  {
    double d0 = v1[0]-v2[0];
    double d1 = v1[1]-v2[1];
    double d2 = v1[2]-v2[2];
    return sqrt(d0*d0+d1*d1+d2*d2);
  }

  template <class floatT1>
  inline void mult(const floatT1*v1,double a, double* result)
  {
    result[0] = a*v1[0];
    result[1] = a*v1[1];
    result[2] = a*v1[2];
  }

  template <class floatT1>
  inline void normalise(const floatT1*v, double* result)
  {
    double sq = dot(v,v);
    if (sq>0)
      mult(v,1.0/sqrt(sq),result);
    else
      result[0] = result[1] = result[2] = 0;
  }

}

#endif
