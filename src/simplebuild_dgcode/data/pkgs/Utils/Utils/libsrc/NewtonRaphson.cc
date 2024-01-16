#include "Utils/NewtonRaphson.hh"
#include <cmath>
#include <cstdio>
#include <cassert>

double Utils::findNewtonRaphsonRoot(const NRFunc& funcd, const double& xlow, const double& xhigh, const double& xaccuracy)
{
  //based on small num. rec. example

  const int MAXIT = 100;
  int j;
  double df,dx,dxold,f,fh,fl;
  double temp,xh,xl,rts;

  funcd.evaluate(xlow,fl,df);
  funcd.evaluate(xhigh,fh,df);
  if ((fl > 0.0 && fh > 0.0) || (fl < 0.0 && fh < 0.0)) {
    printf("NewtonRaphson ERROR: Root must be bracketed\n");
    assert(false);
    return 0;
  }
  if (fl == 0.0) return xlow;
  if (fh == 0.0) return xhigh;
  if (fl < 0.0) {
    xl=xlow;
    xh=xhigh;
  } else {
    xh=xlow;
    xl=xhigh;
  }
  rts=0.5*(xlow+xhigh);
  dxold=fabs(xhigh-xlow);
  dx=dxold;
  funcd.evaluate(rts,f,df);
  for (j=1;j<=MAXIT;j++) {
    if ((((rts-xh)*df-f)*((rts-xl)*df-f) >= 0.0)
        || (fabs(2.0*f) > fabs(dxold*df))) {
      dxold=dx;
      dx=0.5*(xh-xl);
      rts=xl+dx;
      if (xl == rts) return rts;
    } else {
      dxold=dx;
      dx=f/df;
      temp=rts;
      rts -= dx;
      if (temp == rts) return rts;
    }
    if (fabs(dx) < xaccuracy) return rts;
    funcd.evaluate(rts,f,df);
    if (f < 0.0)
      xl=rts;
    else
      xh=rts;
  }
  printf("NewtonRaphson ERROR: Maximum number of iterations exceeded\n");
  assert(false);
  return 0.0;
}
