/* 
 * File:   Test1_v3_BOS.cpp
 * Author: Madani
 * Based on "rafopar"  TCSGen.cc  File
 *
 * Created on June 03, 2026, 2:53 PM
 *
 * ---------------------------------------------------------------------------
 * CONVENTION DE NOMMAGE IMPOSEE PAR BOSwrite
 * ---------------------------------------------------------------------------
 * BOSwrite apparie les branches PAR NOM (SetBranchAddress), pas par position.
 * Les noms ci-dessous sont donc ceux du code original (topologie Lambda(1405)),
 * reutilises pour la topologie gamma p -> gamma rho0 p, rho0 -> pi+ pi-.
 *
 *   Branche    Particule ici              -> ligne MCTK   beg_vtx  end_vtx
 *   -------    -----------------------    -------------   -------  -------
 *   beam       photon incident               mctk[0]         0        1
 *   Qp2        photon sortant (P2)           mctk[1]         1        0
 *   Qp1        rho0 (masse variable)         mctk[2]         1        1
 *   Sp2        pi+  (fille1 du rho0)         mctk[3]         1        0
 *   Sp1        pi-  (fille2 du rho0)         mctk[4]         1        0
 *   Rp2        proton de recul               mctk[5]         1        0
 *   Rp1        placeholder (non lu)            --           --       --
 *   Vertex1    vertex de reaction            mcvx[0]
 *   Vertex2    copie de Vertex1 (non lu)       --
 *
 * Rp1 et Vertex2 sont ecrits uniquement parce que BOSwrite fait un
 * SetBranchAddress dessus : si la branche manque, le pointeur reste nul et
 * le Clear() de fin de boucle segfaute. Leur contenu n'est jamais utilise.
 *
 * UNITES : impulsions en GeV/c, vertex en cm (convention CLAS/GSIM).
 * BOSwrite n'utilise que Px, Py, Pz (cosinus directeurs + module). La
 * composante E n'est lue que pour Qp1, via ->M(), pour la masse du rho0.
 * ---------------------------------------------------------------------------
 */

#include <TF1.h>
#include <TH2D.h>
#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <iomanip>
#include <fstream>
#include <TRandom2.h>
#include <TRandom3.h>
#include <TVector3.h>
#include <TTCSCrs.h>
#include <TTCSKine.h>
#include <KinFunctions.h>
#include <TLorentzVector.h>

#include <cstdlib>
#include <iostream>

using namespace std;
using namespace KinFuncs;

double SigmaPlusProche(const std::vector<double>& tab, double s, double M2, double mu_prime){
    // mu_prime = -u'  (postritif)
    const int N_S=29, N_M2=73, N_UP=100;
    const double S_MIN=6.0, S_STEP=0.5, MP2=0.938*0.938;
    int i = (int)lround((s - S_MIN)/S_STEP);     i = std::min(std::max(i,0), N_S-1);
    double f = (S_MIN + S_STEP*i - MP2)/(20.0 - MP2);
    double M2lo = 2.2*f, dM2 = (9.4*f - M2lo)/(N_M2-1);
    int j = (int)lround((M2 - M2lo)/dM2);        j = std::min(std::max(j,0), N_M2-1);
    double M2node = M2lo + dM2*j;
    double uplo = 1.0*f, dup = (M2node - 0.518219*f - uplo)/(N_UP-1);
    int k = (int)lround((mu_prime - uplo)/dup);  k = std::min(std::max(k,0), N_UP-1);
    return tab[i*(N_M2*N_UP) + j*N_UP + k];   // ordre fichier : -u' varie le + vite, puis M2, puis s
}

// ==========================================================================
// Calcul de -u' a partir de cos_th, pour la derivee numerique du jacobien.
//
// REMARQUE IMPORTANTE : l'angle theta (cos_th) N'EST PAS defini par rapport
// a la direction du photon incident.(Voir la definition)
// ==========================================================================
double uprime_from_costh(double costh, double phi_cm,
                         double P_P1_P2, double Masse1, double Masse2,
                         const TLorentzVector& L_gprime, double Eg)
{
    double sinth = sqrt(1.0 - costh*costh);
    double E_P1  = sqrt(P_P1_P2*P_P1_P2 + Masse1*Masse1);

    // L_P1 = rho sortant, construit exactement comme dans la boucle
    TLorentzVector L_P1_loc;
    L_P1_loc.SetPxPyPzE(P_P1_P2 * sinth * cos(phi_cm),
                        P_P1_P2 * sinth * sin(phi_cm),
                        P_P1_P2 * costh,
                        E_P1);
    L_P1_loc.Boost(L_gprime.BoostVector());

    TLorentzVector L_gamma_in(0., 0., Eg, Eg);
    return -( L_P1_loc - L_gamma_in ).M2();   // = -u' (positif)
}

/*
 * 
 */
int main(int argc, char** argv) {

    // ==================================
    // ==== Reading the input config file
    // ==================================
    
    ifstream inpconfig("Input.dat");
    
    map<std::string, std::string> m_Settings;
    if( inpconfig.is_open() ){
        while( !inpconfig.eof() ){
            std::string Key;
            std::string Val;
            inpconfig>>Key;
            inpconfig>>Val;
            m_Settings[Key] = Val;
        }
    }else{
        cout<<"Can not open the file Input.dat"<<endl;
        cout<<"So can not initialize settings "<<endl;
        cout<<"Exiting"<<endl;
        exit(1);
    }
    
    int parametre;
    int parametre2;
    int parametre3;
    int parametre4;

    
    int Nsim;
    double Eb;
    double t_lim;
    double Eg_min;
    double Eg_max;
    double MinvMin;
    bool isLund;
    double q2_cut;
    int seed;
    double vz_max;
    double vz_min;

    double Mn;

    double M1;
    double M2;
    double psf_M1;
    double psf_M2;

    double M_p1;
    double M_p2;
    double M_p3;
    double M_p4;

    bool decay_P1;
    bool decay_P2;

    parametre=0;
    parametre2=0;
    parametre3=0;
    parametre4=0;

    const int N_S=29, N_M2=73, N_UP=100;
    std::vector<double> sigma_tab(N_S*N_M2*N_UP);
    {
      std::ifstream ftab("table_clean.dat");
      if(!ftab.is_open()){ cout<<"Impossible d'ouvrir table_clean.dat"<<endl; exit(1); }
      double cs, cM2, cup, sp, sm, sx, sy;  int idx=0;
      while( ftab >> cs >> cM2 >> cup >> sp >> sm >> sx >> sy ){
          sigma_tab[idx] = sp + sm;   // NON polarisee
          idx++;
      }
      cout << "Table chargee : " << idx << " / " << N_S*N_M2*N_UP << " points" << endl;
    }
    
    for( map<std::string, std::string>::iterator it =  m_Settings.begin(); it!= m_Settings.end(); it++ ){
    
        std::string key = (*it).first;
        std::string val = (*it).second;

        if( key.compare("Nsim") == 0 ){
            Nsim = atoi(val.c_str());
        }else if( key.compare("Eb") == 0 ){
            Eb = atof(val.c_str());
        }else if( key.compare("tLim") == 0 ){
            t_lim = atof(val.c_str());
        }else if( key.compare("EgMin") == 0 ){
            Eg_min = atof(val.c_str());
        }else if( key.compare("EgMax") == 0 ){
            Eg_max = atof(val.c_str());
        }else if( key.compare("Q2Cut") == 0 ){
            q2_cut = atof(val.c_str());
        }else if( key.compare("LUND") == 0 ){
            isLund = atoi(val.c_str());
        }else if( key.compare("Seed") == 0 ){
            seed = atoi(val.c_str());
        }else if( key.compare("vzMax") == 0 ){
            vz_max = atof(val.c_str());
        }else if( key.compare("vzMin") == 0 ){
            vz_min = atof(val.c_str());
        }else if( key.compare("Mn") == 0 ){
            Mn = atof(val.c_str());
        }else if( key.compare("M1") == 0 ){
            M1 = atof(val.c_str());
        }else if( key.compare("M2") == 0 ){
            M2 = atof(val.c_str());
        }else if( key.compare("psfM1") == 0 ){
            psf_M1 = atof(val.c_str());
        }else if( key.compare("psfM2") == 0 ){
            psf_M2 = atof(val.c_str());
        }else if( key.compare("Massefille1") == 0 ){
            M_p1 = atof(val.c_str());
        }else if( key.compare("Massefille2") == 0 ){
            M_p2 = atof(val.c_str());
        }else if( key.compare("Massefille3") == 0 ){
            M_p3 = atof(val.c_str());
        }else if( key.compare("Massefille4") == 0 ){
            M_p4 = atof(val.c_str());
        }else if( key.compare("DecayP1") == 0 ){
            decay_P1 = atoi(val.c_str());
        }else if( key.compare("DecayP2") == 0 ){
            decay_P2 = atoi(val.c_str());
        }
        
    }
    
    cout<<"Nsim = "<<Nsim<<endl;
    cout<<"Eb = "<<Eb<<endl;
    cout<<"t_lim = "<<t_lim<<endl;
    cout<<"Eg_min = "<<Eg_min<<endl;
    cout<<"Eg_max = "<<Eg_max<<endl;
    cout<<"q2_cut = "<<q2_cut<<endl;
    cout << "vz_max = " << vz_max << endl;
    cout << "vz_min = " << vz_min << endl;
    cout<<"IsLund = "<<isLund<<" (sortie LUND desactivee dans cette version)"<<endl;

    cout << "Mn = " << Mn << endl;
    
    

    if( M1 == 0 ){cout << "M1 = " << M1 << endl;
                  cout << "La particule 1 est un photon (pas forcement mais bon !)" << endl;}
    else {cout << "M1 = " << M1 << endl;}
    if( M2 == 0 ){cout << "M2 = " << M2 << endl;
                  cout << "La particule 2 est un photon (pas forcement mais bon !)" << endl;}
    else {cout << "M2 = " << M2 << endl;}

    cout << "psf_M1 = " << psf_M1 << endl;
    cout << "psf_M2 = " << psf_M2 << endl;
    cout << "Massefille1 = " << M_p1 << endl;
    cout << "Massefille2 = " << M_p2 << endl;
    cout << "Massefille3 = " << M_p3 << endl;
    cout << "Massefille4 = " << M_p4 << endl;

    cout << "DecayP1 = " << decay_P1 << endl;
    cout << "DecayP2 = " << decay_P2 << endl;

    // Avertissement si la topologie n'est pas celle attendue par BOSwrite
    if( !decay_P1 ){
        cout << "\n*** ATTENTION : DecayP1 = 0 ***" << endl;
        cout << "L'arbre mytree a besoin des deux filles du rho0 (pi+ et pi-)." << endl;
        cout << "Aucun evenement ne sera ecrit dans mytree." << endl;
    }
    // ================================================================

    cout<<"**************************************************"<<endl;
    cout<<"*******"<<" RandomSeedActuallyUsed: "<<seed<<" *******"<<endl;
    cout<<"**************************************************"<<endl;
    
    const double PI = 3.14159265358979312;
    const double radian = 57.2957795130823229;
    const double Mp = Mn;
    const double M1_Centre = M1;
    const double M2_Centre = M2;
    const double M_fille1 = M_p1;
    const double M_fille2 = M_p2;
    const double M_fille3 = M_p3;
    const double M_fille4 = M_p4;

    double up_cut = 1.0;     
    double tp_cut = 1.0;      
    int useTableDomain = 1;   

    // ========================================================================
    // P1 = rho0 (113) si massif, photon (22) si M=0 ; idem pour P2.
    // Dans ta config actuelle : P1 = rho0 -> pi+pi-, P2 = photon sortant.
    int pid_P1 = ( M1_Centre == 0.0 ) ? 22 : 113;
    int pid_P2 = ( M2_Centre == 0.0 ) ? 22 : 113;
    // ========================================================================

    TRandom2 rand;
    rand.SetSeed(seed);
    TRandom3 randGen(seed);
    

    TLorentzVector target(0., 0., 0., Mp);
    TLorentzVector Lcm;

    TFile *file_out = new TFile("MonteCarlo_Two_To_Three.root", "RECREATE");
    file_out->SetCompressionSettings(101);   // ZLIB niveau 1 — lisible par ROOT 5

    TTree *tr1 = new TTree("tr1", "Monte Carlo GammaRho");

    //================= Definition of Tree Variables =================
    double Eg, t, Q2, u, s;
    double u_prime, t_prime;
    double psf;
    double flux_factor;
    double cos_theta_H, cos_theta_H2;
    double weight;        // poids total de l'evenement (sigma * R_t * psf * jac)
    double sigma_cs;      // section efficace tabulee au point (s, M2, -u')
    double jac;           // jacobien |d(-u')/d(cos_th)|
    double R_t;           // facteur de repondaration en t
    double Masse1_evt;    // masse du rho tiree evenement par evenement
    double Masse2_evt;    // masse de P2 tiree evenement par evenement
    double vz;            // vertex z (le meme que celui ecrit dans le LUND)
    int    ievent;        
   
    TLorentzVector L_fille1, L_fille2, L_fille3, L_fille4, L_P1, L_P2, L_prot;
    TLorentzVector L_gprime;
    TLorentzVector L_fille1_CMP1,  L_fille3_CMP2;
    TLorentzVector L_miss;


    TH2D *h_fille1  = new TH2D("fille1","Theta vs Phi fille1; #phi (rad); #theta (rad)",200, -3.14, 3.14, 200, 0., 3.14);
    TH2D *h_fille2  = new TH2D("fille2","Theta vs Phi fille2; #phi (rad); #theta (rad)",200, -3.14, 3.14, 200, 0., 3.14);
    TH2D *h_fille3  = new TH2D("fille3","Theta vs Phi fille3; #phi (rad); #theta (rad)",200, -3.14, 3.14, 200, 0., 3.14);
    TH2D *h_fille4  = new TH2D("fille4","Theta vs Phi fille4; #phi (rad); #theta (rad)",200, -3.14, 3.14, 200, 0., 3.14);
    TH2D *h_P2  = new TH2D("P2","Theta vs Phi Particule 2; #phi (rad); #theta (rad)",200, -3.14, 3.14, 200, 0., 3.14);
    TH1D *h_filles = new TH1D("filles", "Masse invariante filles",200, 0, 1);
    TH1D *h_filles99 = new TH1D("filles99", "Masse invariante filles99",200, 0, 1);

    TH2D *h_t_Eg    = new TH2D("h_t_Eg","t vs Eg; Eg (GeV); -t (GeV^{2})", 200, 0., 6, 200, 0., 0.5);
    TH2D *h_t_u    = new TH2D("h_t_u","t vs u; -t (GeV^{2}); -u (GeV^{2})", 200, 0., 0.5, 200, 0., 10);
    TH2D *h_t_Q2    = new TH2D("h_t_Q2","t vs Masse invariante de Particule1-Particule2; -t (GeV^{2}); Masse invariante de Particule1-Particule2 (GeV)", 200, 0, 0.5, 200, 0.7, 3);
    TH2D *h_u_Q2    = new TH2D("h_u_Q2","u vs Masse invariante de Particule1-Particule2; -u (GeV^{2}); Masse invariante de Particule1-Particule2 (GeV)", 200, 0, 10, 200, 0.7, 3);
    
    TH2D *h_Pfille1Theta  = new TH2D("Pfille1Theta","P vs Theta fille1; #theta (rad); Pfille1",200, 0, 3.14, 200, 0, 5.5);
    TH2D *h_Pfille2Theta  = new TH2D("Pfille2Theta","P vs Theta fille2; #theta (rad); Pfille2",200, 0, 3.14, 200, 0, 5.5);
    TH2D *h_Pfille3Theta  = new TH2D("Pfille3Theta","P vs Theta fille3; #theta (rad); Pfille3",200, 0, 3.14, 200, 0, 5.5);
    TH2D *h_Pfille4Theta  = new TH2D("Pfille4Theta","P vs Theta fille4; #theta (rad); Pfille4",200, 0, 3.14, 200, 0, 5.5);
    TH2D *h_PP2Theta  = new TH2D("P2Theta","P vs Theta P2; #theta (rad); P2",200, 0, 3.14, 200, 0, 5.5);
    TH2D *h_PprotTheta  = new TH2D("Pprot","P vs Theta prot; #theta (rad); Pprot",200, 0, 3.14, 200, 0, 1);
    

    TH2D *h_Pfille1Phi  = new TH2D("Pfille1Phi","P vs Phi fille1; #phi (rad); Pfille1",200, -3.14, 3.14, 1000, 0, 5.5);
    TH2D *h_Pfille2Phi  = new TH2D("Pfille2Phi","P vs Phi fille2; #phi (rad); Pfille2",200, -3.14, 3.14, 1000, 0, 5.5);


    tr1->Branch("L_miss", "TLorentzVector", &L_miss, 3200, 99);
    tr1->Branch("L_fille1", "TLorentzVector", &L_fille1, 3200, 99);
    tr1->Branch("L_fille2", "TLorentzVector", &L_fille2, 3200, 99);
    tr1->Branch("L_fille3", "TLorentzVector", &L_fille3, 3200, 99);
    tr1->Branch("L_fille4", "TLorentzVector", &L_fille4, 3200, 99);
    tr1->Branch("L_P1", "TLorentzVector", &L_P1, 3200, 99);
    tr1->Branch("L_P2", "TLorentzVector", &L_P2, 3200, 99);
    tr1->Branch("L_prot", "TLorentzVector", &L_prot, 3200, 99);
    tr1->Branch("Eg", &Eg, "Eg/D");
    tr1->Branch("Q2", &Q2, "Q2/D");
    tr1->Branch("t", &t, "t/D");
    tr1->Branch("u", &u, "u/D");
    tr1->Branch("s", &s, "s/D");
    tr1->Branch("cos_theta_H", &cos_theta_H, "cos_theta_H/D");
    tr1->Branch("cos_theta_H2", &cos_theta_H2, "cos_theta_H2/D");
    tr1->Branch("u_prime", &u_prime, "u_prime/D");
    tr1->Branch("t_prime", &t_prime, "t_prime/D");
    tr1->Branch("sigma",       &sigma_cs,    "sigma/D");
    tr1->Branch("psf",         &psf,         "psf/D");
    tr1->Branch("jac",         &jac,         "jac/D");
    tr1->Branch("R_t",         &R_t,         "R_t/D");
    tr1->Branch("flux_factor", &flux_factor, "flux_factor/D");
    tr1->Branch("Masse1_evt",  &Masse1_evt,  "Masse1_evt/D");
    tr1->Branch("Masse2_evt",  &Masse2_evt,  "Masse2_evt/D");
    tr1->Branch("vz",          &vz,          "vz/D");
    tr1->Branch("ievent",      &ievent,      "ievent/I");
    // ========================================================================


    // ========================================================================
    // === ARBRE "mytree" AU FORMAT ATTENDU PAR BOSwrite ===========
    //
    // Noms, types et ordre de declaration strictement identiques a ceux du
    // SetBranchAddress de BOSwrite_v1.1.cc. Split level 0 : l'objet est
    // serialise en bloc, ce qui garantit la relecture par
    // SetBranchAddress(nom, &pointeur_nul) cote BOSwrite.
    // ========================================================================
    TTree *mytree = new TTree("mytree", "Format BOSwrite : gamma p -> gamma rho0 p");

    TLorentzVector *V_beam = new TLorentzVector();   // photon incident
    TLorentzVector *V_Qp1  = new TLorentzVector();   // rho0 (masse variable)
    TLorentzVector *V_Qp2  = new TLorentzVector();   // photon sortant
    TLorentzVector *V_Sp1  = new TLorentzVector();   // pi-
    TLorentzVector *V_Sp2  = new TLorentzVector();   // pi+
    TLorentzVector *V_Rp1  = new TLorentzVector();   // placeholder (non lu)
    TLorentzVector *V_Rp2  = new TLorentzVector();   // proton de recul
    TVector3       *V_Vertex1 = new TVector3();      // vertex de reaction (cm)
    TVector3       *V_Vertex2 = new TVector3();      // copie (non lue)

    mytree->Branch("beam",    "TLorentzVector", &V_beam,    32000, 0);
    mytree->Branch("Qp1",     "TLorentzVector", &V_Qp1,     32000, 0);
    mytree->Branch("Qp2",     "TLorentzVector", &V_Qp2,     32000, 0);
    mytree->Branch("Sp1",     "TLorentzVector", &V_Sp1,     32000, 0);
    mytree->Branch("Sp2",     "TLorentzVector", &V_Sp2,     32000, 0);
    mytree->Branch("Rp1",     "TLorentzVector", &V_Rp1,     32000, 0);
    mytree->Branch("Rp2",     "TLorentzVector", &V_Rp2,     32000, 0);
    mytree->Branch("Vertex1", "TVector3",       &V_Vertex1, 32000, 0);
    mytree->Branch("Vertex2", "TVector3",       &V_Vertex2, 32000, 0);

    // Position transverse du vertex (cm) : le faisceau est sur l'axe z.
    // Le smearing reel est applique par GSIM via la ffread card.
    const double vx_bos = 0.0, vy_bos = 0.0;

    int n_bos = 0;        // evenements ecrits dans mytree
    int n_bos_skip = 0;   // evenements acceptes mais sans les deux pions
    // ========================================================================


    int n_accepted = 0;

    int n_rej_fact  = 0;
    int n_rej_table = 0;
    int n_bad_sum   = 0;

    for (int i = 0; i < Nsim; i++) {
        if (i % 5000 == 0) {
           cout.flush() << "Processed " << i << " events, approximetely " << double(100. * i / double(Nsim)) << "%\r";
        }

        double Masse1;
        double Masse2; 
        if (psf_M1 == 0){ Masse1 = M1_Centre;}
        else { Masse1 = randGen.Gaus(M1_Centre, psf_M1/2.35);}

        if (psf_M2 == 0){ Masse2 = M2_Centre;}
        else { Masse2 = randGen.Gaus(M2_Centre, psf_M2/2.35);}

        MinvMin = Masse1 + Masse2 ;

        const double Minv_Egmin = sqrt( Mp*Mp + 2*Mp*Eg_min ) - Mp;
        
        double EgMin = Eg_min;

        if( Minv_Egmin < MinvMin ){
            EgMin = (MinvMin*MinvMin + 2*Mp*MinvMin)/(2*Mp);
            
        }
    
        const double MinvMin2 = MinvMin*MinvMin;

        double psf_Eg = Eg_max - EgMin;
        Eg = rand.Uniform(EgMin, EgMin + psf_Eg);


        flux_factor = 1.0;
        s = Mp * Mp + 2 * Mp*Eg;
        double t_min = T_min(0., Mp*Mp, MinvMin2, Mp*Mp, s);
        double t_max = T_max(0., Mp*Mp, MinvMin2, Mp*Mp, s);
        double psf_t = t_min - TMath::Max(t_max, t_lim);

        if (t_min > t_lim) {
             t = rand.Uniform(t_min - psf_t, t_min);
            double Q2max = 2 * Mp * Eg + t - (Eg / Mp)*(2 * Mp * Mp - t - sqrt(t * t - 4 * Mp * Mp * t)); 
            
            double psf_Q2 = Q2max - MinvMin2;

            Q2 = rand.Uniform(MinvMin2, MinvMin2 + psf_Q2);

            u = 2 * Mp * Mp + Q2 - s - t;
            double th_qprime = acos((s * (t - u) - Mp * Mp * (Q2 - Mp * Mp)) / sqrt(Lambda(s, 0, Mp * Mp) * Lambda(s, Q2, Mp * Mp)));
            double th_pprime = PI + th_qprime;

            double Pprime = 0.5 * sqrt(Lambda(s, Q2, Mp * Mp) / s);

            double psf_phi_lab = 2 * PI;
            double phi_rot = rand.Uniform(0., psf_phi_lab);

            Lcm.SetPxPyPzE(0., 0., Eg, Mp + Eg);
            L_prot.SetPxPyPzE(Pprime * sin(th_pprime), 0., Pprime * cos(th_pprime), sqrt(Pprime * Pprime + Mp * Mp));
            L_gprime.SetPxPyPzE(Pprime * sin(th_qprime), 0., Pprime * cos(th_qprime), sqrt(Pprime * Pprime + Q2));

            L_gprime.Boost(Lcm.BoostVector());
            L_prot.Boost(Lcm.BoostVector());

            L_prot.RotateZ(phi_rot);
            L_gprime.RotateZ(phi_rot);

            L_miss = Lcm - L_gprime - L_prot;

            double psf_cos_th = 2.;
            double psf_phi_cm = 2 * PI;

            double cos_th = rand.Uniform(-1., -1 + psf_cos_th);
            double sin_th = sqrt(1 - cos_th * cos_th);
            double phi_cm = rand.Uniform(0., 0. + psf_phi_cm);
            
            double P_P1_P2 = sqrt(Lambda(Q2, Masse1*Masse1, Masse2*Masse2)) / (2*sqrt(Q2));
            double E_P1 = sqrt(P_P1_P2*P_P1_P2 + Masse1 * Masse1);
            double E_P2 = sqrt(P_P1_P2*P_P1_P2 + Masse2 * Masse2);

            L_P1.SetPxPyPzE(P_P1_P2 * sin_th * cos(phi_cm), P_P1_P2 * sin_th * sin(phi_cm), P_P1_P2*cos_th, E_P1);
            L_P2.SetPxPyPzE(-P_P1_P2 * sin_th * cos(phi_cm), -P_P1_P2 * sin_th * sin(phi_cm), -P_P1_P2*cos_th, E_P2);

            L_P1.Boost(L_gprime.BoostVector());
            L_P2.Boost(L_gprime.BoostVector());

            TLorentzVector L_gamma_in(0., 0., Eg, Eg);
            u_prime = (L_P1 - L_gamma_in).M2();   // P1 = rho
            t_prime = (L_P2 - L_gamma_in).M2();   // P2 = gamma sortant

            if ( fabs( (t_prime + u_prime)
                     - (Masse1*Masse1 + Masse2*Masse2 + t - Q2) ) > 1e-6 ) {
                n_bad_sum = n_bad_sum + 1;
            }

            if ( -u_prime < up_cut || -t_prime < tp_cut ) {
                n_rej_fact = n_rej_fact + 1;
                continue;
            }

            if ( useTableDomain == 1 ) {
                const double mp2_table = 0.938 * 0.938;
                double f_scale = (s - mp2_table) / (20.0 - mp2_table);
                bool inTable = ( s >= 6.0 && s <= 20.0
                              && Q2 >= 2.2 * f_scale && Q2 <= 9.4 * f_scale
                              && -u_prime >= 1.0 * f_scale
                              && -u_prime <= Q2 - 0.518219 * f_scale );
                if ( !inTable ) {
                    n_rej_table = n_rej_table + 1;
                    continue;
                }
            }

            // --- Jacobien numerique |d(-u')/d(cos_th)| ---
            const double eps_jac = 1e-5;
            double up_plus  = uprime_from_costh(cos_th + eps_jac, phi_cm,
                                    P_P1_P2, Masse1, Masse2, L_gprime, Eg);
            double up_minus = uprime_from_costh(cos_th - eps_jac, phi_cm,
                                    P_P1_P2, Masse1, Masse2, L_gprime, Eg);
            jac = fabs( (up_plus - up_minus) / (2.0 * eps_jac) );  // |d(-u')/d(cos_th)|

            psf = psf_Eg * psf_t * psf_Q2 * psf_cos_th * psf_phi_cm ;

            R_t      = 1.4641 / ((t - 0.71)*(t - 0.71));  
            sigma_cs = SigmaPlusProche(sigma_tab, s, Q2, -u_prime);
            weight   = sigma_cs * R_t * psf * jac;   

            // === MODIF : variables evenement pour le tree + vertex genere ICI
            //             (avant les decays) pour etre identique tree <-> BOS
            Masse1_evt = Masse1;
            Masse2_evt = Masse2;
            ievent     = i;
            vz         = rand.Uniform(vz_min, vz_max);
            // ================================================================

            if ( (decay_P1 && Masse1 < M_fille1 + M_fille2) ||
                 (decay_P2 && Masse2 < M_fille3 + M_fille4) ) {
                continue;
            }

            if (decay_P1){
                if (Masse1 < M_fille1 + M_fille2) {
                    continue;
                }
                else{
                    double psf_cos_th_filles = 2.; 
                    double psf_phi_cm_filles = 2 * PI;

        
                    double cos_th_filles = rand.Uniform(-1., -1 + psf_cos_th_filles);
                    double sin_th_filles = sqrt(1 - cos_th_filles * cos_th_filles);
                    double phi_cm_filles = rand.Uniform(0., 0. + psf_phi_cm_filles);
            
                    double P_f1_f2 = sqrt(Lambda(Masse1*Masse1, M_fille1*M_fille1, M_fille2*M_fille2)) / (2*Masse1);
                    double E_f1 = sqrt(P_f1_f2*P_f1_f2 + M_fille1*M_fille1);
                    double E_f2 = sqrt(P_f1_f2*P_f1_f2 + M_fille2*M_fille2);
                    
                    L_fille1.SetPxPyPzE(P_f1_f2 * sin_th_filles * cos(phi_cm_filles), P_f1_f2 * sin_th_filles * sin(phi_cm_filles), P_f1_f2*cos_th_filles, E_f1);
                    L_fille2.SetPxPyPzE(-P_f1_f2 * sin_th_filles * cos(phi_cm_filles), -P_f1_f2 * sin_th_filles * sin(phi_cm_filles), -P_f1_f2*cos_th_filles, E_f2);

                    L_fille1.Boost(L_P1.BoostVector());
                    L_fille2.Boost(L_P1.BoostVector());

                    L_fille1_CMP1 = L_fille1;  
                    L_fille1_CMP1.Boost(-L_P1.BoostVector());     
                    TVector3 P1_direction = L_P1.Vect().Unit();

                    cos_theta_H = L_fille1_CMP1.Vect().Unit().Dot(P1_direction);


                    h_fille1 ->Fill(L_fille1.Phi(),  L_fille1.Theta(), weight);
                    h_fille2->Fill(L_fille2.Phi(), L_fille2.Theta(), weight);

                    h_filles->Fill((L_fille1 + L_fille2).Mag(), weight);

                    h_Pfille1Theta ->Fill(L_fille1.Theta(), L_fille1.P(), weight);
                    h_Pfille2Theta ->Fill(L_fille2.Theta(), L_fille2.P(), weight);


                    h_Pfille1Phi ->Fill(L_fille1.Phi(), L_fille1.P(), weight);
                    h_Pfille2Phi ->Fill(L_fille2.Phi(), L_fille2.P(), weight);

                    if ( fabs(L_P1.Px() - L_fille1.Px() - L_fille2.Px()) > 1e-6 ||
                         fabs(L_P1.Py() - L_fille1.Py() - L_fille2.Py()) > 1e-6 ||
                         fabs(L_P1.Pz() - L_fille1.Pz() - L_fille2.Pz()) > 1e-6 ||
                         fabs(L_P1.E()  - L_fille1.E() - L_fille2.E())  > 1e-6 ) {
                            parametre3 = parametre3 + 1;
                        }
    
   
                }
            }

            if (decay_P2) {
                if (Masse2 < M_fille3 + M_fille4) {
                    continue;
                }
                else{
                    double psf_cos_th_filles99 = 2.; 
                    double psf_phi_cm_filles99 = 2 * PI;

                    double cos_th_filles99 = rand.Uniform(-1., -1 + psf_cos_th_filles99);
                    double sin_th_filles99 = sqrt(1 - cos_th_filles99 * cos_th_filles99);
                    double phi_cm_filles99 = rand.Uniform(0., 0. + psf_phi_cm_filles99);

                    double P_f3_f4 = sqrt(Lambda(Masse2*Masse2, M_fille3*M_fille3, M_fille4*M_fille4)) / (2*Masse2);
                    double E_f3 = sqrt(P_f3_f4*P_f3_f4 + M_fille3*M_fille3);
                    double E_f4 = sqrt(P_f3_f4*P_f3_f4 + M_fille4*M_fille4);

                    L_fille3.SetPxPyPzE(P_f3_f4 * sin_th_filles99 * cos(phi_cm_filles99), P_f3_f4 * sin_th_filles99 * sin(phi_cm_filles99), P_f3_f4*cos_th_filles99, E_f3);
                    L_fille4.SetPxPyPzE(-P_f3_f4 * sin_th_filles99 * cos(phi_cm_filles99), -P_f3_f4 * sin_th_filles99 * sin(phi_cm_filles99), -P_f3_f4*cos_th_filles99, E_f4);

                    L_fille3.Boost(L_P2.BoostVector());
                    L_fille4.Boost(L_P2.BoostVector());

                    L_fille3_CMP2 = L_fille3;  
                    L_fille3_CMP2.Boost(-L_P2.BoostVector());     
                    TVector3 P2_direction = L_P2.Vect().Unit();

                    cos_theta_H2 = L_fille3_CMP2.Vect().Unit().Dot(P2_direction);

                    h_fille3 ->Fill(L_fille3.Phi(),  L_fille3.Theta(), weight);
                    h_fille4->Fill(L_fille4.Phi(), L_fille4.Theta(), weight);

                    h_filles99->Fill((L_fille3 + L_fille4).Mag(), weight);

   
                    h_Pfille3Theta ->Fill(L_fille3.Theta(), L_fille3.P(), weight);
                    h_Pfille4Theta ->Fill(L_fille4.Theta(), L_fille4.P(), weight);

                    if ( fabs(L_P2.Px() - L_fille3.Px() - L_fille4.Px()) > 1e-6 ||
                      fabs(L_P2.Py() - L_fille3.Py() - L_fille4.Py()) > 1e-6 ||
                      fabs(L_P2.Pz() - L_fille3.Pz() - L_fille4.Pz()) > 1e-6 ||
                      fabs(L_P2.E()  - L_fille3.E() - L_fille4.E())  > 1e-6 ) {
                      parametre4 = parametre4 + 1;
                      }

                }
            }

            

            h_P2     ->Fill(L_P2.Phi(),      L_P2.Theta(), weight);

            h_t_Eg   ->Fill(Eg, -t, weight);
            h_t_u   ->Fill(-t, -u, weight);
            h_t_Q2    ->Fill(-t, sqrt(Q2), weight);
            h_u_Q2   ->Fill(-u, sqrt(Q2), weight);

            h_PP2Theta ->Fill(L_P2.Theta(), L_P2.P(), weight);
            h_PprotTheta ->Fill(L_prot.Theta(), L_prot.P(), weight);

            tr1->Fill();

            n_accepted++;

            if ( fabs(Lcm.Px() - L_gprime.Px() - L_prot.Px()) > 1e-6 ||
                 fabs(Lcm.Py() - L_gprime.Py() - L_prot.Py()) > 1e-6 ||
                 fabs(Lcm.Pz() - L_gprime.Pz() - L_prot.Pz()) > 1e-6 ||
                 fabs(Lcm.E()  - L_gprime.E() - L_prot.E())  > 1e-6 ) {
                 parametre = parametre + 1;
                 } 
            if ( fabs(L_gprime.Px() - L_P2.Px() - L_P1.Px()) > 1e-6 ||
                      fabs(L_gprime.Py() - L_P2.Py() - L_P1.Py()) > 1e-6 ||
                      fabs(L_gprime.Pz() - L_P2.Pz() - L_P1.Pz()) > 1e-6 ||
                      fabs(L_gprime.E()  - L_P2.E() - L_P1.E())  > 1e-6 ) {
                        parametre2 = parametre2 + 1;
                }


            //=================== Remplissage de mytree (format BOSwrite) =============
            //
            // Le vertex vz est celui tire plus haut, avant les desintegrations :
            // arbre d'analyse (tr1) et arbre BOS decrivent donc le meme evenement.
            //
            if (decay_P1) {

                V_beam->SetPxPyPzE(0., 0., Eg, Eg);   // photon incident, sur la ligne du faisceau

                *V_Qp1 = L_P1;        // rho0    -> mctk[2], masse relue via ->M()
                *V_Qp2 = L_P2;        // gamma'  -> mctk[1]
                *V_Sp2 = L_fille1;    // pi+     -> mctk[3]
                *V_Sp1 = L_fille2;    // pi-     -> mctk[4]
                *V_Rp2 = L_prot;      // proton  -> mctk[5]

                *V_Rp1 = L_P1;        // placeholder : branche requise par le
                                      // SetBranchAddress de BOSwrite, jamais utilisee

                V_Vertex1->SetXYZ(vx_bos, vy_bos, vz);   // cm
                *V_Vertex2 = *V_Vertex1;                 // idem, non lue

                mytree->Fill();
                n_bos++;

            } else {
                n_bos_skip++;
            }
            //=========================================================================

        } else {
            cout << " |t_min| > |t_lim|" << endl;
            cout << " t_min =  " << t_min << "   t_lim = " << t_lim << "  Eg = " << Eg << endl;
        }
    }

    tr1->Write();
    mytree->Write();   // === MODIF v3

    h_fille1->Write();
    h_fille2->Write();
    h_filles->Write();
    h_P2->Write();

    h_t_Eg->Write();
    h_t_u->Write();
    h_t_Q2->Write();
    h_u_Q2->Write();

    h_Pfille1Theta ->Write();
    h_Pfille2Theta ->Write();
    h_PP2Theta ->Write();
    h_PprotTheta ->Write();

    h_fille3->Write(); 
    h_fille4->Write();   
    h_filles99->Write(); 
    h_Pfille3Theta->Write(); 
    h_Pfille4Theta->Write(); 

    h_Pfille1Phi ->Write();
    h_Pfille2Phi ->Write();

    
    cout << "\n----------------------------------------------" << endl;
    cout << "Evenements generes (Nsim)        : " << Nsim        << endl;
    cout << "Rejetes coupures factorisation   : " << n_rej_fact  << endl;
    cout << "Rejetes hors grille de Saad      : " << n_rej_table << endl;
    cout << "Identite t'+u' violee (>1e-6)    : " << n_bad_sum   << endl;
    cout << "Evenements acceptes (tr1)        : " << n_accepted  << endl;
    cout << "Evenements ecrits dans mytree    : " << n_bos       << endl;
    cout << "Non ecrits (pas de decay du rho) : " << n_bos_skip  << endl;
    cout << "----------------------------------------------" << endl;
    // ========================================================================

    if(parametre == 0 && parametre2 == 0 && parametre3 == 0 && parametre4 == 0){
        cout << "\nLes vecteurs energie-impultion sont conserves :) " << endl;
    } else{cout << "\nparamatre =" << parametre << endl;
           cout << parametre2 << endl;
           cout << parametre3 << endl;
           cout << parametre4 << endl;
           cout << "\nLes vecteurs energie-impultion ne sont pas conserves :( " << endl;}
    if (n_accepted == 0) {
    cout << "\n=====================================================" << endl;
    cout << "AUCUN evenement cinematiquement possible !" << endl;}

    file_out->Close();

    return 0;

}