#include <cmath>
#include <KinFunctions.h>

// This is from Byukling Kayanti Formula (6.3)
double KinFuncs::Lambda( double x, double y, double z )
{
  return (x - y - z)*(x - y - z) - 4*y*z;
}

//From Byukling Kayanti Formula (5.14) Page 86
double KinFuncs::T_min( double ma_2, double mb_2, double m1_2, double m2_2, double s) // arguments are squares of masses of particles in the reaction a+b->1+2, and s is the square of the total c.m. energy i.e. (a+b)^2
{
  return ma_2 + m1_2 - (1/(2*s))*( (s + ma_2 - mb_2)*(s + m1_2 - m2_2) - sqrt( Lambda(s, ma_2, mb_2)*Lambda(s, m1_2, m2_2) ) );
}

//From Byukling Kayanti Formula (5.14) page 86
double KinFuncs::T_max( double ma_2, double mb_2, double m1_2, double m2_2, double s)
{
  return ma_2 + m1_2 - (1/(2*s))*( (s + ma_2 - mb_2)*(s + m1_2 - m2_2) + sqrt( Lambda(s, ma_2, mb_2)*Lambda(s, m1_2, m2_2) ) );
}

double KinFuncs::Q2_min( double s, double Eb, double M )
{
  // M is the target mass;
  double me = 0.00051;
  double Eg = (s - M*M)/(2*M);
  double E_pr = Eb - Eg;
  double P0 = sqrt(Eb*Eb - me*me);
  double P_pr = sqrt(E_pr*E_pr - me*me);
  double Q2min = 2*(Eb*E_pr - P0*P_pr - me*me);

  return Q2min;
}

double KinFuncs::N_EPA(double Eb, double Eg, double Q2_max)
{
  const double alpha = 1./137.;
  const double PI = 3.14159265358979312;

  double x = Eg/Eb;
  double me = 0.00051;
  double Mp = 0.9383;
  double Q2_min = me*me*x*x/(1 - x);
  return (1/Eb)*alpha/(PI*x)*( (1 - x + x*x/2)*log(Q2_max/Q2_min) - (1 - x));
}

double KinFuncs::N_Brem(double Eg, double Eb, double d, double X0)
{
  // The factor 0.5 is because when one integrates over (l - x)*dx, then you get l^2/2
  return (0.5*d/X0)*(1/Eg)*((4./3.) - (4./3.)*(Eg/Eb) + Eg*Eg/(Eb*Eb));
}