//************************************* rbtest.C *****************************//
//                      A part of the rootbeer package                        //
//                                                                            //
//                  Ken Livingstn (k.livingston@physics,gla,ac,uk)            //
//                                                                            //
//      Version         Date                    Comment                       //
//      1               21st Feb 2003                                         //
//      1.2d            23st Mar 2004           Simplified                    //
//      2.0             16th Sep 2005           All in root framework         //
//      2.1             20st Feb 2006           Buf fix edition               //
//                                                                            //
// ******************************* rbtest.C **********************************//

// A sample code to access BANKs in root.
// This code can be run as a root macro, or built as an executable:
// See http://nuclear.gla.ac.uk/~kl/rootbeer/manual/html/node5.php
// Or the tutorial in doc/manual.ps

// ******** required headers ********************************************
//                                                                      *
#include "RootBeerUtil.h"	//gets everything for RootBeer scheme
#include "TFile.h"              // for root files
#include "TString.h"              // for root files
#include "TLorentzVector.h"
//                                                                      *
// ******** end of required headers *************************************

// ******** my headers here *********************************************
//                                                                      *
#include "TH1.h"                // eg. for 1D Histogram                 
#include "TH2.h"                // eg. for 2D Histogram
//                                                                      *
// ******** end of my headers here **************************************



// ********** required global variables *********************************
//                                                                      *
// They are global to get easily filled by parseArgs()
int 	nEventsTot=0;			// ie do all events in all files
char  	InFileName[128];		// in file name parsed from args 
char  	OutFileName[128];		// out file name parsed from args

class TRootBeer *rootbeer;        	// The main class
//                                                                      *
// ********** end of required global variables **************************

// **************** my global variables *********************************
//                                                                      *
// Eg. my hists are global to be easily accessed from the 
// root> prompt after the function is called as a macro
class   TH2F *tagr_hist;            	// A Sample 2d histogram for tagr
class   TH1F *scr_hist[7];              // A Sample 1d hist array for SCR
float mpi = 0.13957;
float mp = 0.93827;;
//                                                                      *
// **********************************************************************


// ********* required functions defined in this code ******************** 
//                                                                      *
void printUsage();			// print usage when run as exe       
int  parseArgs(int,char **);		// parse args when run as exe
//                                                                      *
// ********* end of required functions defined in this code ************* 

// ********* my functions defined in this code ************************** 
//                                                                      *
void rbtest(int, TString , TString); // main func. (same name as this macro)
//                                                                      *
// ********* end of my functions defined in this code ******************* 


// ---------- FUNCTION DEFINITIONS BEGIN HERE ---------------------

// ---------- Required main() for making an executable -------------
#ifdef ROOTEXE
int main(int argc,char **argv){		
  if((parseArgs(argc,argv))!=0) return(-1);	//parse input args
  
  rbtest(nEventsTot,InFileName,OutFileName);	//call user function
  return(0);
}
#endif
// ---------- end of required main() for making an executable ------

// ---------- user main function  -----------------------------------------------
// This gets called by main() when running as an executable,
// or interactively from the root> prompt.
// should have the same name as this file (minus the .C suffix)
// customize the "non-required" sections
 
void rbtest(int nEvents,TString file,TString outFileName){   // main user function

  // *****************variables which are required***********************
  //                                                                    *
  int	event=0;             	// Event counter for current file
  int	eventTot=0;	       	// Total event count
  class	TFile *outFile;         // the file to put the results
  char 	inFile[128];	       	// holds the current file name
  int 	fileNo=0;	       	// current file no if sorting though a list
  //                                                                    *
  // ********* end of variables which are required **********************


  // ******************* my variables ***********************************
  //                                                                    *
  int 	sect_no=0;			// hold the number of a bank sector
  char 	name[100];			// hold temp names for hists 

  int tot_event_of_interest = 0;

  TLorentzVector piPlus, piMinus, proton, photon;
  TLorentzVector photon_ini, target, missing;
  float mpipi, mprotphot, mpipprot, mpimprot, mpipphot, mpimphot, mrhophot;
  float miss_m_tot, miss_E_tot;
  float s, t, u, t_prime, u_prime;

  //                                                                    *
  // ******************* end of my variables ****************************
  
  // ************************* required init stuff **********************
  //                                                                    *
  //#ifdef ROOTEXE					//if running as an executable
  outFile=new TFile(outFileName,"recreate");	//open output file 
  // Branches
  TTree *outtree = new TTree("T", "Tree with event particles");
  outtree->Branch("piPlus", &piPlus);
  outtree->Branch("piMinus", &piMinus);
  outtree->Branch("proton", &proton);
  outtree->Branch("photon", &photon);
  outtree->Branch("missing", &missing);
  outtree->Branch("photon_ini", &photon_ini);

  outtree->Branch("mpipi", &mpipi, "mpipi/F");
  outtree->Branch("mprotphot", &mprotphot, "mprotphot/F");
  outtree->Branch("mpipprot", &mpipprot, "mpipprot/F");
  outtree->Branch("mpimprot", &mpimprot, "mpimprot/F");
  outtree->Branch("mpipphot", &mpipphot, "mpipphot/F");
  outtree->Branch("mpimphot", &mpimphot, "mpimphot/F");
  outtree->Branch("mrhophot", &mrhophot, "mrhophot/F");

  outtree->Branch("miss_m_tot", &miss_m_tot, "miss_m_tot/F");
  outtree->Branch("miss_E_tot", &miss_E_tot, "miss_E_tot/F");

  outtree->Branch("s", &s, "s/F");
  outtree->Branch("t", &t, "t/F");
  outtree->Branch("u", &u, "u/F");
  outtree->Branch("t_prime", &t_prime, "t_prime/F");
  outtree->Branch("u_prime", &u_prime, "u_prime/F");
  
  //#endif
  eventTot=0;			 		// init the total event counter
  //
  //                                                                    *
  // ****************** end of required init stuff **********************

  // ******* User initialisation ****************************************
  //                                                                    *
  //


  // ** DONT FORGET TO DEFINE THE BANKS YOU WANT RIGHT HERE
  // This line can be generated by running:
  // >getbanks -Nevents <BosFile| rootDSTFile>

  const  char *mybanks[]={"SCR ","TAGR", "PART","null"};	// make a list of the banks you want to use

  // ** DONT FORGET TO DEFINE THE BANKS YOU WANT RIGHT HERE
  //


  //Set up my histograms (variables already defined as globals above)
  tagr_hist = new TH2F("tpho_vs_tid","tagr info",
		       121,-0.5,121.5,1000,-100,100); 	//make 2d hist
  tagr_hist->GetYaxis()->SetTitle("tpho");		//set axis titles
  tagr_hist->GetXaxis()->SetTitle("T-ID");

  for(int sect=1; sect<7;sect++){        		//for each sector
    sprintf(name,"scr_sect_%d",sect);			//make a name for each
    scr_hist[sect] = new TH1F(name,name,50,0,50);  	//make 1d hist 
    scr_hist[sect]->GetXaxis()->SetTitle("ID");		//set the x-axis title	
  }
  //                                                                    *
  // ******* End of User initialisation  ********************************


  // ********************** main file loop **********************************************************
  //                                                                                                *
  resetNextFile();
  while((fileNo=getNextFile(inFile,const_cast<char *>(file.Data())))!=-1){  // loop while files are still avialable 

    // Start of user stuff to do before sorting each file  ---------------
    fprintf(stderr,"Sorting file - %s\n",inFile);
    // End of user stuff to do before sorting each file  ---------------


    if((rootbeer=createBeerObject(inFile))==NULL) return; 	// create rootbeer object
    rootbeer->SetBankStatus(const_cast<char**>(mybanks),ON);			// switch on the banks required
    rootbeer->StartServer();                  			// start the server running
    rootbeer->ListServedBanks();			       	// list the banks which will be served

    event=0;		//zero the event counter for the file

    // ********************** main event loop ******************************************************
    //                                                                                             *

    while(event>=0){                                           	// do all events
      event = rootbeer->GetEvent();                            	// get next event

      // Start of user customized section ----------------------------

      //eg of handling of single sector bank TAGR
      for(int row=0;row<TAGR_NH;row++){ 		      	// loop over rows in TAGR bank
	      tagr_hist->Fill(TAGR[row].T_id,TAGR[row].TPHO);         // fill hist 
      }

      //printf("New event \n");

      int nb_prot = 0;
      int nb_pip = 0;
      int nb_pim = 0;
      int nb_pho = 0;

      int pid =0;
      float px = 0.0;
      float py = 0.0;
      float pz = 0.0;

      float e_phot_ini = TAGR[0].ERG;

      for(int row=0;row<PART_NH;row++){ 		      	// loop over rows in TAGR bank
        pid = PART[row].pid;
        px = PART[row].px;
        py = PART[row].py;
        pz = PART[row].pz;

	      //printf("pid %d, px %f, py %f, pz %f \n", PART[row].pid, PART[row].px, PART[row].py, PART[row].pz);         // fill hist 
        
        switch (pid)
        {
        case 8:
          nb_pip++;
          piPlus.SetXYZM(px, py, pz, mpi);
          break;
        case 9:
          nb_pim++;
          piMinus.SetXYZM(px, py, pz, mpi);
          break;
        case 14:
          nb_prot++;
          proton.SetXYZM(px, py, pz, mp);
          break;
        case 1:
          nb_pho++;
          photon.SetXYZM(px, py, pz, 0.0);       
          break;

        default:
          break;
        }
       
      }
      //printf("\n");

      if(nb_pip==1 && nb_pim==1 && nb_prot==1 && nb_pho==1)
      {
        tot_event_of_interest++;

        photon_ini.SetXYZM(0, 0, e_phot_ini, 0.0);       
        target.SetXYZM(0.0, 0.0, 0.0, mp);  

        missing= photon_ini + target - piPlus - piMinus - proton - photon;

        mpipi = (piPlus + piMinus).M();
        mprotphot = (proton + photon).M();
        mpipprot = (piPlus + proton).M();
        mpimprot = (piMinus + proton).M();
        mpipphot = (piPlus + photon).M();
        mpimphot = (piMinus + photon).M();
        mrhophot = (piPlus + piMinus + photon).M();
        miss_m_tot = missing.M();
        miss_E_tot = missing.E();

        s = (photon_ini + target).M2();
        t = (target - proton).M2();
        u = 0.0;
        t_prime = (photon_ini - photon).M2();
        u_prime = (photon_ini - piPlus - piMinus ).M2();

        outtree->Fill();
      }

      //8 9 1 14

      //eg of handling of multiple sector bank SCR
      for(int sector=0; sector<SCR_NS;sector++){              	// for all sectors
	      sect_no=SCR_S[sector];                                 	// get the number of the sector 
	      for(int row=0;row<SCR_NH[sector];row++){            	// for all hits in that sector
	        scr_hist[sect_no]->Fill(SCR[sector][row].id);       	// fill  hist
	      } 
      }   
      // End of user customized section ----------------------------

      eventTot++;
      if(eventTot%100000 == 0) fprintf(stdout,"Done %d\n",eventTot);
      if((nEvents>0)&&(eventTot >=nEvents)) break;		//break if nEvents done
    }
    //                                                                                             *
    // ********************** end of main event loop ***********************************************

    // Start of user stuff to do after sorting each file  ---------------
    fprintf(stdout,"sorted %d events from file: %s\n",abs(event),inFile);
    // End of user stuff to do after sorting each file  ---------------

    delete rootbeer;                              		//Call the class destructor
    if((nEvents>0)&&(eventTot >=nEvents)) break;		//break if nEvents done
  }
  //                                                                                                *
  // ********************** end of main file loop ***************************************************



  // ******** Do any final stuff here ***********************************
  //                                                                    *
  
  // Start of user stuff to do at the end  ------------------------
  fprintf(stdout,"Sorted Total of  %d events\n",eventTot);

  fprintf(stdout,"Found %d events of interest \n",tot_event_of_interest);
  // End of user stuff to do at the end  --------------------------

  //#ifdef ROOTEXE
  //if it's an executable, write to the output file
  outtree->Write();
  outFile->Write();
  outFile->Close();
  //#endif
  //                                                                    *
  // ********************************************************************
}
// ---------- end of user main function  -----------------------------------------------



// ---------- required parseArgs() function  -------------------------------------------
// parses the input args when running as an executable
// set up the no of events, and input and output file names in global variables
// or prints usage
int parseArgs(int argc,char **argv){
  switch(argc){
  case(1):				// 1 or 2 args not enough
  case(2):
    printUsage();
    return(-1);
    break;
  case(3):				// 3 args rbtest <infile> <outfile>
    if((strstr(argv[1],"-N"))!=NULL){
      printUsage();
      return(-1);
    }
    else{
      strcpy(InFileName,argv[1]);
      strcpy(OutFileName,argv[2]);
    }
    break;
  case(4):				// 4 args must be "rbtest -N# <infile> <outfile>" 
    if(strncmp(argv[1],"-N",2)!=0){  	//print usage then exit */
      printUsage();
      return(-1);
    }
    else{
      sscanf(argv[1]+2,"%d",&nEventsTot);	//get the required event no
      strcpy(InFileName,argv[2]);
      strcpy(OutFileName,argv[3]);
    }      
    break;
  default:
    printUsage();
    return(-1);
    break;
  }
  return(0);
}
// ---------- end of required parseArgs() function  -------------------------------------

// ---------- required printUsage() function  -------------------------------------------
// When running as executable, gets called if no args, or silly args
void printUsage(){
  fprintf(stderr,"\nusage:\n\n");
  fprintf(stderr,"rbtest -h   Print this message\n\n");
  fprintf(stderr,"rbtest [-Nevents] <infile> <outfile.root> \n");
  fprintf(stderr,"rbtest [-Nevents] <-Lfilelist> <outfile.root> \n");
}
// ---------- end of required printUsage() function  -------------------------------------------
