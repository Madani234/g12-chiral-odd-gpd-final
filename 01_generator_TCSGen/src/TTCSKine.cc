/* 
 * File:   TTCSKine.cc
 * Author: rafopar
 * 
 * Created on January 11, 2020, 1:49 PM
 */

#include <TLorentzVector.h>

#include "TTCSKine.h"

TTCSKine::TTCSKine(double m, double E) {
    mprot = m;
    Eb = E;

    // Initializing the beam and and the target
    Lp.SetPxPyPzE(0., 0., 0., mprot);
    Lbeam.SetPxPyPzE(0, 0, Eb, Eb);
}

TTCSKine::TTCSKine(TLorentzVector &em, TLorentzVector &ep, TLorentzVector &p1, double m, double E) {
    Lem = em;
    Lep = ep;
    Lp1 = p1;

    mprot = m;
    Eb = E;

    Lp.SetPxPyPzE(0., 0., 0., mprot);
    Lbeam.SetPxPyPzE(0, 0, Eb, Eb);

    Define_kinematic();
}

/*
TTCSKine::TTCSKine(const TTCSKine& orig) {
}
 */

void TTCSKine::SetLemLepLp(TLorentzVector &em, TLorentzVector &ep, TLorentzVector &p1) {
    Lem = em;
    Lep = ep;
    Lp1 = p1;

    // When Lem, Lep or p1 are changed, kinematics needs to be redefined
    Define_kinematic();
}

void TTCSKine::Define_kinematic() {
    L_mis = Lbeam + Lp - Lem - Lep - Lp1;
    Lg = Lp1 + Lem + Lep - Lp;
    Lemep = Lem + Lep;

    Minv = Lemep.M();
    Eq_prime = Lemep.E();
    tM = (TLorentzVector(Lp - Lp1)).M2();
    Egamma = Lg.E();
    MM2 = L_mis.M2();
    mis_mom = L_mis.P();
    px_mis = L_mis.Px();
    py_mis = L_mis.Py();
    pz_mis = L_mis.Pz();

    Q2 = 2 * Eb * (mis_mom - pz_mis);

    Lcm = Lg + Lp;

    Lp_cm = Lp;
    Lp_cm.Boost(-Lcm.BoostVector());
    Lp1_cm = Lp1;
    Lp1_cm.Boost(-Lcm.BoostVector());
    Lem_cm = Lem;
    Lem_cm.Boost(-Lcm.BoostVector());
    Lep_cm = Lep;
    Lep_cm.Boost(-Lcm.BoostVector());

    Lem_eep_cm = Lem;
    Lem_eep_cm.Boost(-Lemep.BoostVector());
    Lep_eep_cm = Lep;
    Lep_eep_cm.Boost(-Lemep.BoostVector());
    Lp1_eep_cm = Lp1;
    Lp1_eep_cm.Boost(-Lemep.BoostVector());

    TV3_em = Lem_cm.Vect();
    TV3_ep = Lep_cm.Vect();
    TV3_p = Lp_cm.Vect();
    TV3_p1 = Lp1_cm.Vect();
    TV3_emep_crs = TV3_ep.Cross(TV3_em);
    TV3_pp1_crs = TV3_p.Cross(TV3_p1);

    if (TV3_em.Dot(TV3_pp1_crs) > 0) {
        phi_cm = TV3_emep_crs.Angle(TV3_pp1_crs) * radian;
    } else {
        phi_cm = (2 * PI - TV3_emep_crs.Angle(TV3_pp1_crs)) * radian;
    }

    theta_cm = (PI - Lem_eep_cm.Angle(Lp1_eep_cm.Vect())) * radian;

    double bb = 2 * (TLorentzVector(Lem - Lep)).Dot(Lp - Lp1);
    L = ((Minv * Minv - tM)*(Minv * Minv - tM) - bb * bb) / 4.;
    L0 = Minv * Minv * Minv * Minv * sin(theta_cm / radian) * sin(theta_cm / radian);

}

TTCSKine::~TTCSKine() {
}

