/* 
 * File:   KinFunctions.h
 * Author: rafopar
 *
 * Created on January 11, 2020, 4:38 PM
 */

#ifndef KINFUNCTIONS_H
#define KINFUNCTIONS_H

namespace KinFuncs {

    double Lambda(double, double, double);
    double T_min(double, double, double, double, double);
    double T_max(double, double, double, double, double);
    double Q2_min(double s, double Eb, double M);
    double N_EPA(double, double, double );
    double N_Brem(double Eg, double Eb, double d = 5., double X0 = 929.); // 5 default target length used in RG-A, and 929 is the Liquid Hydrogen Rad length
}


#endif /* KINFUNCTIONS_H */

