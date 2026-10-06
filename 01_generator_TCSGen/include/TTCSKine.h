/* 
 * File:   TTCSKine.h
 * Author: rafopar
 *
 * Created on January 11, 2020, 1:49 PM
 */

#ifndef TTCSKINE_H
#define TTCSKINE_H

#include <TLorentzVector.h>

class TTCSKine {
public:
    TTCSKine(double m = 0.938, double E = 10.6);
    TTCSKine(TLorentzVector&, TLorentzVector&, TLorentzVector&, double m = 0.938, double E = 5.76);
    /*TTCSKine(const TTCSKine& orig);*/

    double GetPhi_cm() const {return phi_cm;};
    double GetTheta_cm() const {return theta_cm;};
    double Get_tM() const {return tM;};                 // Mandelshtam t
    double GetMinv() const {return Minv;};              // Invariant Mass of lepton pairs
    double GetMM2() const {return MM2;};
    double GetEg() const {return Egamma;};              // Energy of incpming photon
    double GetEq_prime() const {return Eq_prime;};      // Energy of the timelike photon
    double GetMis_mom() const {return mis_mom;};        // Missing Momentum
    double GetPx_mis() const {return px_mis;};          // X xomponent of Missing momentum
    double GetPy_mis() const {return py_mis;};          // Y xomponent of Missing momentum
    double GetPz_mis() const {return pz_mis;};          // Z xomponent of Missing momentum
    double GetQ2() const {return Q2;};                  // Q2 of the quasi-real photon
    double Get_L() const {return L;};                   // Kinematic factor L
    double Get_L0() const {return L0;};                 // Kinematic factor L0
    void SetLemLepLp(TLorentzVector&, TLorentzVector&, TLorentzVector&);

    void Define_kinematic();


    virtual ~TTCSKine();
private:

    TLorentzVector Lp;
    TLorentzVector Lp_cm;
    TLorentzVector Lp1;
    TLorentzVector Lp1_cm;
    TLorentzVector Lem;
    TLorentzVector Lem_cm;
    TLorentzVector Lep;
    TLorentzVector Lep_cm;
    TLorentzVector Lg;
    TLorentzVector Lg_cm;
    TLorentzVector Lemep;
    TLorentzVector Lemep_cm;
    TLorentzVector Lgemep;
    TLorentzVector Lgemep_cm;
    TLorentzVector Lem_eep_cm;
    TLorentzVector Lep_eep_cm;
    TLorentzVector Lp1_eep_cm;
    TLorentzVector Lcm;
    TLorentzVector Lbeam;

    TLorentzVector L_mis;

    double Eq_prime;
    double mprot;
    double Egamma;
    double Eb;
    double Minv, tM;
    double MM2;
    double mis_mom;
    double px_mis;
    double py_mis;
    double pz_mis;
    double L, L0;
    double Q2; // Q2 of the quasi-real photon
    static constexpr double radian = 57.2957795130823229;
    static constexpr double PI = 3.14159265358979312;

    //  Lp.SetPxPyPzE(0., 0., 0., mprot);
    //  Lbeam.SetPxPyPzE(0, 0, Eb, Eb);

    TVector3 TV3_em, TV3_ep, TV3_p, TV3_p1;
    TVector3 TV3_emep_crs, TV3_pp1_crs; //cross products of (em X ep) and (p X p1)

    double phi_cm, theta_cm;


};

#endif /* TTCSKINE_H */

