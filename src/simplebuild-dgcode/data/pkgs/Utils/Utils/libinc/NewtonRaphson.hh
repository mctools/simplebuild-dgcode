#ifndef Utils_NewtonRaphson_hh
#define Utils_NewtonRaphson_hh

namespace Utils {

  class NRFunc {
  public:
    virtual void evaluate(const double& x, double& f_of_x, double& fprime_of_x) const = 0;
  };


  //find root of NRFunc in [xlow,xhigh]
  double findNewtonRaphsonRoot(const NRFunc&, const double& xlow, const double& xhigh, const double& xaccuracy = 1.0e-6);

}


#endif
