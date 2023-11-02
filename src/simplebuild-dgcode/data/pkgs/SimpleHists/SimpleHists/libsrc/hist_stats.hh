
// The simple approach to one-pass calculation of RMS used for example in ROOT
// (using sumW, sumWX and sumWX2 variables) is highly numerically unstable, since
// one in the end calculates RMS by sqrt(bignum^2-otherbignum^2).
//
// Here we instead define a different variable:
//
// T = sumWX2 - (sumWX)^2/sumW  => RMS = sqrt(T/sumW)
//
// One can (carefully) derive formulas for how T should be updated when
// respectively filling and merging histograms. These formulas are numerically
// stable, and are represented in the implementation below.
//
// Below we keep T in the variable called "rms_state", but for reference we also
// provide the simple ROOT-style approach by changing the first define below to
// 1 (***FOR TESTING ONLY***). In that case, rms_state corresponds to the usual
// sumWX2, and one can expect significant levels of incorrectness in the RMS
// estimations when distribution have true rms << |mean|.
//
// Thomas Kittelmann, 2014.

#define SIMPLEHISTS_ROOT_STYLE_RMS 0

#include <cmath>
#include <stdexcept>

namespace SimpleHists {

  //used while filling:
  inline void update_stats_on_fill(double& sumw, double& sumwx, double& rms_state, const double& val)
  {
#if SIMPLEHISTS_ROOT_STYLE_RMS
    rms_state += val*val;
#else
    const double d1=sumw*val-sumwx;
    const double d2=sumw*(1+sumw);
    rms_state += (d2?d1*d1/d2:0);
#endif
    ++sumw;
    sumwx += val;
  }

  inline void update_stats_on_fillN(double& sumw, double& sumwx, double& rms_state, const unsigned long& N, const double& val)
  {
#if SIMPLEHISTS_ROOT_STYLE_RMS
    rms_state += N * val*val;
#else
    const double d1=sumw*val-sumwx;
    const double d2=sumw*(N+sumw);
    rms_state += (d2?N*d1*d1/d2:0);
#endif
    sumw += N;
    sumwx += N * val;
  }

  inline void update_stats_on_fill(double& sumw, double& sumwx, double& rms_state, const double& val, const double& weight)
  {
#if SIMPLEHISTS_ROOT_STYLE_RMS
    rms_state += weight*val*val;
#else
    const double d1=sumw*val-sumwx;
    const double d2=sumw*(weight+sumw);
    rms_state += (d2?d1*d1*weight/d2:0);
#endif
    sumw += weight;
    sumwx += val*weight;
  }

  inline double calc_covxy(const double& sumw, const double& covstate)
  {
    if (!sumw)
      throw std::runtime_error("covariance not well defined in empty histograms");
    return covstate/sumw;
  }

  inline void update_covxy_on_fill(const double& old_sumw, const double& old_sumwx, const double& old_sumwy,
                                   double& covstate, const double& valx,const double& valy, const double& weight)
  {
    if (!old_sumw)
      return;
    double new_sumwx = old_sumwx + valx * weight;
    double new_sumw = old_sumw + weight;
    covstate += weight * (valx - new_sumwx/new_sumw) * (valy - old_sumwy/old_sumw);//looks assymetric but is not
  }


  inline void update_covxy_on_fill(const double& old_sumw, const double& old_sumwx, const double& old_sumwy,
                                   double& covstate, const double& valx,const double& valy)
  {
    if (!old_sumw)
      return;
    double new_sumwx = old_sumwx + valx;
    double new_sumw = old_sumw + 1;
    covstate += (valx - new_sumwx/new_sumw) * (valy - old_sumwy/old_sumw);//looks assymetric but is not
  }


  inline void merge_covxy(double& covstate, const double& sumw, const double& sumwx, const double& sumwy,
                          const double& other_covstate, const double& other_sumw, const double& other_sumwx, const double& other_sumwy)
  {
    if (other_sumw) {
      if (sumw) {
        //both hists had something
        covstate += other_covstate
          + ( (sumwx/sumw-other_sumwx/other_sumw)
              * (sumwy/sumw-other_sumwy/other_sumw)
              * (sumw*other_sumw)/(sumw+other_sumw) );
      } else {
        //we didn't have anything yet, so can simply copy in the value
        covstate = other_covstate;
      }
    } else {
      //nothing to merge
      assert(other_sumw==0&&other_sumwx==0&&other_sumwy==0);
    }
  }

  inline void merge_stats(double& sumw, double& sumwx, double& rms_state,
                          const double& other_sumw, const double& other_sumwx, const double& other_rms_state)
  {
    if (other_sumw) {
      if (sumw) {
        //both hists had something
#if SIMPLEHISTS_ROOT_STYLE_RMS
        rms_state += other_rms_state;
#else
        const double w1(sumw);//s1
        const double w2(other_sumw);//s2
        const double wx1(sumwx);//b1
        const double wx2(other_sumwx);//b2
        const double k(w2*wx1-w1*wx2);
        assert((w1+w2)>0);
        rms_state += other_rms_state + k*k/(w1*w2*(w1+w2));
#endif
        sumw += other_sumw;
        sumwx += other_sumwx;
      } else {
        //we didn't have anything yet, so can simply copy in stats
        rms_state = other_rms_state;
        sumw = other_sumw;
        sumwx = other_sumwx;
      }
    } else {
      //nothing to merge
      assert(other_sumw==0&&other_sumwx==0&&other_rms_state==0);
    }
  }

#if SIMPLEHISTS_ROOT_STYLE_RMS
  inline double calc_rms(const double& sumw, const double& sumwx, const double& rms_state)
  {
    if (!sumw)
      throw std::runtime_error("rms not well defined in empty histograms");

    double rms2 = rms_state/sumw - (sumwx*sumwx)/(sumw*sumw);
    return sqrt(rms2<0?-rms2:rms2);
  }
  inline double calc_rms2(const double& sumw, const double& sumwx, const double& rms_state)
  {
    if (!sumw)
      throw std::runtime_error("rms not well defined in empty histograms");

    double rms2 = rms_state/sumw - (sumwx*sumwx)/(sumw*sumw);
    return rms2;
  }
#else
  inline double calc_rms(const double& sumw, const double& rms_state)
  {
    if (!sumw)
      throw std::runtime_error("rms not well defined in empty histograms");

    double rms2=rms_state/sumw;
    assert(rms2>=0||std::isnan(rms2));
    return sqrt(rms2);
  }
  inline double calc_rms2(const double& sumw, const double& rms_state)
  {
    if (!sumw)
      throw std::runtime_error("rms not well defined in empty histograms");

    double rms2=rms_state/sumw;
    assert(rms2>=0);
    return rms2;
  }
#endif

  inline double calc_mean(const double& sumw, const double& sumwx)
  {
    if (!sumw)
      throw std::runtime_error("mean not well defined in empty histograms");
    return sumwx/sumw;
  }

  inline void update_maxmin_filled(float& minfilled,
                                   float& maxfilled,
                                   const double& val)
  {
    if (maxfilled<minfilled) {
      //first fill
      minfilled = maxfilled = val;
    } else {
      //subsequent fill
      minfilled = std::min<double>(val,minfilled);
      maxfilled = std::max<double>(val,maxfilled);
      // if (val<minfilled) minfilled = val;
      // if (val>maxfilled) maxfilled = val;
    }
  }

  inline void merge_maxmin(float& minfilled, float& maxfilled,
                           const float& other_minfilled, const float& other_maxfilled)
  {
    const bool def(minfilled<=maxfilled);
    const bool o_def(other_minfilled<=other_maxfilled);
    if (!def&&o_def) {
      //We had no fills, but the other did. Copy over.
      minfilled = other_minfilled;
      maxfilled = other_maxfilled;
    } else if (def&&o_def) {
      //Both histograms had fills. Expand min..max range as necessary.
      if (other_minfilled<minfilled) minfilled = other_minfilled;
      if (other_maxfilled>maxfilled) maxfilled = other_maxfilled;
    }
  }

}
