
namespace SimpleHists {
  inline bool floatCompatible(const double& val1, const double& val2)
  {
    const double eps = 1.0e-6;
    return (fabs(val1-val2)/(1.0+std::max(fabs(val1),fabs(val2)))<eps) || ((val1!=val1)&&(val2!=val2));
  }
}
