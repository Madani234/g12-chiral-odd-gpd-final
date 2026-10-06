using namespace std;
#include <unistd.h>
#include <string>

extern "C"
{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ntypes.h>
#include <bostypes.h>
#include <clas_cern.h>
#include <scalers.h>
#include <utility.h>
#include <printBOS.h>
#include <time.h>
  //Paul//
#include <bosfun.h>
#include <bosio.h>
#include <biofun.h>
  //    //
#define speed_light 29979245800 // cm/s
#define lifetime_Lambda 0.00000000026 // s
#define lifetime_SigmaStar 0.000000000000000000000018 // s
#define lifetime_SigmaStarMinus 0.0000000000000000000000167 // s

#define mass_protPDG 0.93827201
#define mass_neutPDG 0.939565378
#define mass_deutPDG 1.875612942
#define mass_piplusPDG 0.13957018
#define mass_piminusPDG 0.13957018
#define mass_pizeroPDG 0.1349766
#define mass_kaonPDG 0.493677
#define mass_lambdaPDG 1.115683
#define mass_photonPDG 0.0
#define mass_sigmaPDG 1.192642
#define mass_possigmaPDG 1.18937
#define mass_sigmastarPDG 1.3837
#define mass_sigmastarminusPDG 1.3872
#define mass_rhoPDG 0.77526



#define prot_numPDG 2212
#define neut_numPDG 2112
#define deut_numPDG 45
#define piplus_numPDG 211
#define piminus_numPDG -211
#define pizero_numPDG 111
#define kaon_numPDG 321
#define lambda_numPDG 3122
#define photon_numPDG 22
#define sigma_numPDG 3212
#define possigma_numPDG 3222
#define sigmastar_numPDG 3214
#define sigmastarminus_numPDG 3114
#define rho_numPDG 113

  BOSbank bcs_;
}

#include <iostream>
#include <TROOT.h>
#include <TFile.h>
#include <TChain.h>
#include <TNtuple.h>
#include <TLorentzVector.h>
#include <TRandom3.h>
#include <TF1.h>
#include <TSystem.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TStyle.h>
#include <sstream>

void Display_Help();

main (int argc, char *argv[]){
  ////////////////////////////////////////////     ////////////////////////////////////////////
  cout << "Author: Tongtong Cao (caot@jlab.org)" << endl;
  cout << "Author of origninal version: Nicholas Zachariou" << endl;
  cout << "Modified by: Joshua Bryce" << endl;
  cout << "Adapted to gamma rho0 p topology" << endl;
  cout << "Run with '-H' or '-h' switch for help." << endl;
  int loc_i, locMaxNumEventsPerFile = -1, locNumEvents, loc_numbroot, numbroot=0, rlabel = 1;
  vector<string> locStringInputs; //vectors (can change their size and allocate by push_back command)
  string locTempString;
  istringstream locIStream;
  for(loc_i = 1; loc_i < argc; loc_i++){
    if(argv[loc_i][0] != '-'){
      locStringInputs.push_back(argv[loc_i]);
      numbroot++;
    }
    else{	//it's a switch:
      switch(argv[loc_i][1]){
      case 'h'://display help
	Display_Help();
	return 0;
      case 'H'://display help
	Display_Help();
	return 0;
      case 'R'://set rlabel
	locTempString = argv[loc_i];
	locTempString = locTempString.substr(2, locTempString.length() - 2);	//strip "-R"
	locIStream.str(locTempString);	//stores locTempString to locIStreat
	if(!(locIStream >> rlabel))	//converts locIStream into integer and assigns it to rlabel
	  cout << "ERROR: COMMAND LINE INPUT NOT RECOGNIZED. rlabel NOT SET. DEFAULTING TO  rlabel = 1." << endl;
	break;
      case 'M'://set MaxNumEventsPerFile
	locTempString = argv[loc_i];
	locTempString = locTempString.substr(2, locTempString.length() - 2);	//strip "-M"
	locIStream.str(locTempString);	//stores locTempString to locIStreat
	if(!(locIStream >> locMaxNumEventsPerFile))	//converts locIStream into integer and assigns it to LocMaxNumEventsPerFile
	  cout << "ERROR: COMMAND LINE INPUT NOT RECOGNIZED.  MaxNumEventsPerFile NOT SET." << endl;
	break;
      default:
	break;
      }//kills switch
    }
  }//kills loop over arguments
  if(locStringInputs.size() == 0){	//Checks if there is an input bos file
    cout << "ERROR: NEED INPUT ROOT FILE." << endl;
    return 2;
  }
  /////////////////////////////////////////////////////////////////////////////


  ////// Load root files in a TChain for running through all events//////
  TChain *tree = new TChain("mytree");
  for (loc_numbroot=0; loc_numbroot<numbroot; loc_numbroot++){
    cout <<" Additing to TChain, Root file: "<<locStringInputs[loc_numbroot].c_str()<<endl;
    tree->Add(locStringInputs[loc_numbroot].c_str());
  }
  ///////////////////////////////////////////////////////////////////////


  ////////////// Set up BOS file -- Open and initialize/////////////////
  BOSbank bThreadBOSCommonBlock_Output;
  int bThreadBOSIOptr_Output;
  //initialize BOS common block
  bosInit(bThreadBOSCommonBlock_Output.iw, 700000); //if more than one thread accessing the same file, should have one per thread (as well as locks around BOS access!)

  //open the file
  //string locOutputFileName = "rename.bos";
  string locOutputFileName = locStringInputs[0].substr(0, locStringInputs[0].length()-4);
  locOutputFileName.append("evt");

  char* locCommand_Char = new char[locOutputFileName.size() + 1];
  strncpy(locCommand_Char, locOutputFileName.c_str(), locOutputFileName.size() + 1);
  bool locOutputFileOpenFlag = !bosOpen(locCommand_Char, "w", &bThreadBOSIOptr_Output);
  delete locCommand_Char;

  /////////////////// Set up Root Branches to read //////////////////////
  //TLorentzVector *rtarget=0;
  TLorentzVector *rbeam=0;
  TLorentzVector *rQp1=0;
  TLorentzVector *rQp2=0;
  //TLorentzVector *rStarget=0;
  //TLorentzVector *rSbeam=0;
  TLorentzVector *rSp1=0;
  TLorentzVector *rSp2=0;
  TLorentzVector *rRp1=0;
  TLorentzVector *rRp2=0;
  TVector3 *rVertex1=0;
  TVector3 *rVertex2=0;

  //tree->SetBranchAddress("target",&rtarget);
  tree->SetBranchAddress("beam",&rbeam);
  tree->SetBranchAddress("Qp1",&rQp1); //rho0 particle track
  tree->SetBranchAddress("Qp2",&rQp2); //outgoing photon track
  //tree->SetBranchAddress("Starget",&rStarget);
  //tree->SetBranchAddress("Sbeam",&rSbeam);
  tree->SetBranchAddress("Sp1",&rSp1); //pi-
  tree->SetBranchAddress("Sp2",&rSp2); //pi+
  tree->SetBranchAddress("Rp1",&rRp1); //placeholder (not used)
  tree->SetBranchAddress("Rp2",&rRp2); //proton
  tree->SetBranchAddress("Vertex1",&rVertex1);
  tree->SetBranchAddress("Vertex2",&rVertex2);

  float vertex_x=0, vertex_y=0, vertex_z=0;

  locNumEvents=0;
  int loc_NumEntries = tree->GetEntries();
  if (locMaxNumEventsPerFile>0)
    cout << "Main file has " << loc_NumEntries<<" entries. Will process "<<locMaxNumEventsPerFile<<" events" << endl;
  else
    cout << "Main file has " << loc_NumEntries<<" entries. "<< endl;

  for(loc_i = 0; loc_i < loc_NumEntries; loc_i++){
    if((locMaxNumEventsPerFile <= locNumEvents) && (locMaxNumEventsPerFile > 0))//past max
      break;
    tree->GetEvent(loc_i);
    if (loc_i%500==0)
      cout<<loc_i<<"  events processed"<<endl;

    int locNumRowsHead=1, locNumRowsMCVX=1, locNumRowsMCTK=6;
    //for each event: create a BOS bank
    string locBankList;
    locBankList.clear();

    clasHEAD_t* locHEADBank = (clasHEAD_t*)makeBank(&bThreadBOSCommonBlock_Output, "HEAD", 0, sizeof(head_t)/sizeof(int), locNumRowsHead);
    locBankList += "HEAD";
    clasMCTK_t* locMCTKBank = (clasMCTK_t*)makeBank(&bThreadBOSCommonBlock_Output, "MCTK", 0, sizeof(mctk_t)/sizeof(int), locNumRowsMCTK);
    locBankList += "MCTK";
    clasMCVX_t* locMCVXBank = (clasMCVX_t*)makeBank(&bThreadBOSCommonBlock_Output, "MCVX", 0, sizeof(mcvx_t)/sizeof(int), locNumRowsMCVX);
    locBankList += "MCVX";

    bosNformat(bThreadBOSCommonBlock_Output.iw, (char*)"HEAD", "8I");
    bosNformat(bThreadBOSCommonBlock_Output.iw, (char*)"MCTK","6F,5I");
    bosNformat(bThreadBOSCommonBlock_Output.iw, (char*)"MCVX","4F,I");

    locHEADBank->head[0].version = 1;
    locHEADBank->head[0].nrun = 1;
    locHEADBank->head[0].nevent = locNumEvents++;
    locHEADBank->head[0].type = -4;  //Negative for simulations
    locHEADBank->head[0].evtclass = 15; //0-15 for physics events
    locHEADBank->head[0].trigbits = 0;
    locHEADBank->head[0].time =0;

    //// vertex is smeared with ffread card for beam position and sigma
    // Reaction vertex

    locMCVXBank->mcvx[0].x = rVertex1 -> X();
    locMCVXBank->mcvx[0].y = rVertex1 -> Y();
    locMCVXBank->mcvx[0].z = rVertex1 -> Z();
    locMCVXBank->mcvx[0].tof = 0.0;
    locMCVXBank->mcvx[0].flag = 0;
    //-----------FIRST TRACK is Photon -------------//
    locMCTKBank->mctk[0].cx = rbeam->Px()/rbeam->Rho();
    locMCTKBank->mctk[0].cy = rbeam->Py()/rbeam->Rho();;
    locMCTKBank->mctk[0].cz = rbeam->Pz()/rbeam->Rho();;
    locMCTKBank->mctk[0].pmom = rbeam->Rho();
    locMCTKBank->mctk[0].mass = mass_photonPDG;
    locMCTKBank->mctk[0].charge = 0;
    locMCTKBank->mctk[0].id = photon_numPDG ;
    locMCTKBank->mctk[0].beg_vtx = 0;
    locMCTKBank->mctk[0].end_vtx = 1;
    locMCTKBank->mctk[0].flag = 1;
    locMCTKBank->mctk[0].parent = 0;
    //--------------------------------------------//

    //rlabel==1 : gamma p -> gamma rho0 p, rho0 -> pi+ pi-
    if (rlabel == 1){

      //Need 5 final tracks besides the incoming photon:
      // outgoing photon, rho0, pi+, pi-, proton
      //Mapping from mytree branches (read by name):
      //  Qp2 -> outgoing photon   (was K+)
      //  Qp1 -> rho0              (was Lambda1405)
      //  Sp2 -> pi+
      //  Sp1 -> pi-               (was pre-scatter Sigma)
      //  Rp2 -> proton

      // Outgoing photon (Qp2)
      locMCTKBank->mctk[1].cx = rQp2->Px()/rQp2->Rho();
      locMCTKBank->mctk[1].cy = rQp2->Py()/rQp2->Rho();
      locMCTKBank->mctk[1].cz = rQp2->Pz()/rQp2->Rho();
      locMCTKBank->mctk[1].pmom = rQp2->Rho();
      locMCTKBank->mctk[1].mass = mass_photonPDG;
      locMCTKBank->mctk[1].charge = 0;
      locMCTKBank->mctk[1].id = photon_numPDG;
      locMCTKBank->mctk[1].beg_vtx = 1;
      locMCTKBank->mctk[1].end_vtx = 0;
      locMCTKBank->mctk[1].flag = 1;
      locMCTKBank->mctk[1].parent = 0;

      //rho0 Track (Qp1) - variable mass via ->M()
      locMCTKBank->mctk[2].cx = rQp1->Px()/rQp1->Rho();
      locMCTKBank->mctk[2].cy = rQp1->Py()/rQp1->Rho();
      locMCTKBank->mctk[2].cz = rQp1->Pz()/rQp1->Rho();
      locMCTKBank->mctk[2].pmom = rQp1->Rho();
      locMCTKBank->mctk[2].mass = rQp1->M();
      locMCTKBank->mctk[2].charge = 0;
      locMCTKBank->mctk[2].id = rho_numPDG;
      locMCTKBank->mctk[2].beg_vtx = 1; //produced at reaction vertex
      locMCTKBank->mctk[2].end_vtx = 1; //immediately decays
      locMCTKBank->mctk[2].flag = 1;
      locMCTKBank->mctk[2].parent = 0;

      //pi+ (Sp2)
      locMCTKBank->mctk[3].cx = rSp2->Px()/rSp2->Rho();
      locMCTKBank->mctk[3].cy = rSp2->Py()/rSp2->Rho();
      locMCTKBank->mctk[3].cz = rSp2->Pz()/rSp2->Rho();
      locMCTKBank->mctk[3].pmom = rSp2->Rho();
      locMCTKBank->mctk[3].mass = mass_piplusPDG;
      locMCTKBank->mctk[3].charge = 1;
      locMCTKBank->mctk[3].id = piplus_numPDG;
      locMCTKBank->mctk[3].beg_vtx = 1; //from rho0 decay
      locMCTKBank->mctk[3].end_vtx = 0; //processed by GSIM
      locMCTKBank->mctk[3].flag = 1;
      locMCTKBank->mctk[3].parent = 0;

      //pi- (Sp1)
      locMCTKBank->mctk[4].cx = rSp1->Px()/rSp1->Rho();
      locMCTKBank->mctk[4].cy = rSp1->Py()/rSp1->Rho();
      locMCTKBank->mctk[4].cz = rSp1->Pz()/rSp1->Rho();
      locMCTKBank->mctk[4].pmom = rSp1->Rho();
      locMCTKBank->mctk[4].mass = mass_piminusPDG;
      locMCTKBank->mctk[4].charge = -1;
      locMCTKBank->mctk[4].id = piminus_numPDG;
      locMCTKBank->mctk[4].beg_vtx = 1; //from rho0 decay
      locMCTKBank->mctk[4].end_vtx = 0; //processed by GSIM
      locMCTKBank->mctk[4].flag = 1;
      locMCTKBank->mctk[4].parent = 0;

      //Proton (Rp2)
      locMCTKBank->mctk[5].cx = rRp2->Px()/rRp2->Rho();
      locMCTKBank->mctk[5].cy = rRp2->Py()/rRp2->Rho();
      locMCTKBank->mctk[5].cz = rRp2->Pz()/rRp2->Rho();
      locMCTKBank->mctk[5].pmom = rRp2->Rho();
      locMCTKBank->mctk[5].mass = mass_protPDG;
      locMCTKBank->mctk[5].charge = 1;
      locMCTKBank->mctk[5].id = prot_numPDG;
      locMCTKBank->mctk[5].beg_vtx = 1; //recoil at reaction vertex
      locMCTKBank->mctk[5].end_vtx = 0; //processed by GSIM
      locMCTKBank->mctk[5].flag = 1;
      locMCTKBank->mctk[5].parent = 0;
    }

    //for each event: write, drop, clean
    char* locCommand_Char = new char[locBankList.size() + 1];
    strncpy(locCommand_Char, locBankList.c_str(), locBankList.size() + 1);
    bosWrite(bThreadBOSIOptr_Output, bThreadBOSCommonBlock_Output.iw, locCommand_Char);
    bosLdrop(bThreadBOSCommonBlock_Output.iw, locCommand_Char);
    bosNgarbage(bThreadBOSCommonBlock_Output.iw);
    delete[] locCommand_Char;


    //rtarget->Clear();
    rbeam->Clear();
    rQp1->Clear();
    rQp2->Clear();
    //rStarget->Clear();
    //rSbeam->Clear();
    rSp1->Clear();
    rSp2->Clear();
    rRp1->Clear();
    rRp2->Clear();
  }

  //cleanup at the end
  bosWrite(bThreadBOSIOptr_Output, bThreadBOSCommonBlock_Output.iw, "0");

  //close the file
  bosClose(bThreadBOSIOptr_Output);


}

void Display_Help(){
  cout << "DISPLAY HELP:" << endl;
  cout << "Command Line: ExecutableFile InputRootFile" << endl;
  cout << "The optional '-M' flag is used for specifying the maximum number of events to be evaluated from each file." << endl;
  cout << "This version is adapted for gamma p -> gamma rho0 p, rho0 -> pi+ pi- (rlabel = 1)." << endl;
}
