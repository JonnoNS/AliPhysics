#include <Riostream.h>
#include "TTree.h"
#include "TRandom3.h"
#include "TLorentzVector.h"
#include "TChain.h"
#include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TList.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TFile.h"
#include "TParticle.h"
#include "TParticlePDG.h"
#include "THnSparse.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TDatabasePDG.h"

#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisTaskSE.h"
#include "AliESDEvent.h"
#include "AliESDInputHandler.h"
#include "AliESDtrackCuts.h"

#include "AliMCEvent.h"
#include "AliMCEventHandler.h"
#include "AliStack.h"

#include "AliAnalysisTaskDeutFlucpp.h"
#include "AliPIDResponse.h"
#include "AliMultSelection.h"
#include "AliCentrality.h"
#include "AliEventCuts.h"
#include "AliHeader.h"
#include "AliGenEventHeader.h"

using std::cout;
using std::endl;

ClassImp(AliAnalysisTaskDeutFlucpp)
    AliAnalysisTaskDeutFlucpp::AliAnalysisTaskDeutFlucpp() : AliAnalysisTaskSE(), fTreeEvent(NULL), fPIDResponse(NULL), fESDtrackCuts(NULL), fEventCuts(0), fTriggerMask(0), fTreeTrackVariableCentrality(0), fTreeTrackVariableVtxz(0), fTreeTrackVariableVtxz_Gen(0), fTreeTrackVariableNTrack(0), fTreeTrackVariableNTrack_d_Gen(0), fTreeTrackVariableNTrack_p_Gen(0), fMCstack(0), fMCevent(0),
      fUseMC(kTRUE)
{
}

AliAnalysisTaskDeutFlucpp::AliAnalysisTaskDeutFlucpp(const char *name) : AliAnalysisTaskSE(name), fTreeEvent(NULL), fPIDResponse(NULL), fESDtrackCuts(NULL), fEventCuts(0), fTriggerMask(0), fTreeTrackVariableCentrality(0), fTreeTrackVariableVtxz(0), fTreeTrackVariableVtxz_Gen(0), fTreeTrackVariableNTrack(0), fTreeTrackVariableNTrack_d_Gen(0), fTreeTrackVariableNTrack_p_Gen(0), fMCstack(0), fMCevent(0),
      fUseMC(kTRUE)
{

  std::cout << " i am in constrctor " << std::endl;
  DefineInput(0, TChain::Class()); // Event chain
  DefineOutput(1, TTree::Class()); // Event Tree

  for (int str = 0; str < kMaxTrack; str++)
  {
    fTreeTrackVariableDCAXY[str] = -999;
    fTreeTrackVariableDCAZ[str] = -999;
    fTreeTrackVariableITSchi2[str] = -999;
    fTreeTrackVariableTPCchi2[str] = -999;
    fTreeTrackVariableNCR[str] = -999;
    fTreeTrackVariabledeuteronnsigmaTPC[str] = -999;
    fTreeTrackVariableprotonnsigmaTPC[str] = -999;
    fTreeTrackVariablemasssquare[str] = -999;
    fTreeTrackVariableCharge[str] = -999;
    fTreeTrackVariableMomentumPx[str] = -999;
    fTreeTrackVariableMomentumPy[str] = -999;
    fTreeTrackVariableMomentumPz[str] = -999;
  }

  for (int str_d_Gen = 0; str_d_Gen < kMaxTrack; str_d_Gen++)
  {
    fTreeTrackVariableMomentumPx_d_Gen[str_d_Gen] = -999;
    fTreeTrackVariableMomentumPy_d_Gen[str_d_Gen] = -999;
    fTreeTrackVariableMomentumPz_d_Gen[str_d_Gen] = -999;
  }
for (int str_p_Gen = 0; str_p_Gen < kMaxTrack; str_p_Gen++)
  {
    fTreeTrackVariableMomentumPx_d_Gen[str_p_Gen] = -999;
    fTreeTrackVariableMomentumPy_d_Gen[str_p_Gen] = -999;
    fTreeTrackVariableMomentumPz_d_Gen[str_p_Gen] = -999;
  }
  
}

AliAnalysisTaskDeutFlucpp::~AliAnalysisTaskDeutFlucpp()
{
  //------------------------------------------------
  // DESTRUCTOR
  //------------------------------------------------

  if (fTreeEvent)
  {
    delete fTreeEvent;
    fTreeEvent = 0x0;
  }

  if (fESDtrackCuts)
  {
    delete fESDtrackCuts;
    fESDtrackCuts = 0x0;
  }
}

//________________________________________________________________________
void AliAnalysisTaskDeutFlucpp::UserCreateOutputObjects()
{

  //------------------------------------------------
  // Particle Identification Setup
  //------------------------------------------------
  AliAnalysisManager *man = AliAnalysisManager::GetAnalysisManager();
  AliInputEventHandler *inputHandler = (AliInputEventHandler *)(man->GetInputEventHandler());
  fPIDResponse = inputHandler->GetPIDResponse();

  AliMCEventHandler* mcHandler = new AliMCEventHandler();
	man->SetMCtruthEventHandler(mcHandler);

  //------------------------------------------------
  // track cut
  //------------------------------------------------
  if (!fESDtrackCuts)
  {
    fESDtrackCuts = new AliESDtrackCuts();
    fESDtrackCuts = AliESDtrackCuts::GetStandardITSTPCTrackCuts2011(kFALSE, 1); // 0 for cluster cut 1 for crossed row cut
    fESDtrackCuts->SetClusterRequirementITS(AliESDtrackCuts::kSPD, AliESDtrackCuts::kAny);
    fESDtrackCuts->SetMaxDCAToVertexXY(0.5);
    fESDtrackCuts->SetMaxDCAToVertexZ(3.0);
    fESDtrackCuts->SetMaxChi2PerClusterTPC(6);
    fESDtrackCuts->SetMaxChi2PerClusterITS(40);
    fESDtrackCuts->SetMaxChi2TPCConstrainedGlobal(36);
    fESDtrackCuts->SetMinNCrossedRowsTPC(60.0);
    fESDtrackCuts->SetPtRange(0.35, 2.0);
    fESDtrackCuts->SetEtaRange(-0.8, 0.8);
  }
  //  fEventCuts.AddQAplotsToList(fList);
  OpenFile(1);
  fTreeEvent = new TTree("fTreeEvent", "Event");
  fTreeEvent->Branch("fTreeTrackVariableCentrality", &fTreeTrackVariableCentrality, "fTreeTrackVariableCentrality/F");
  fTreeEvent->Branch("fTreeTrackVariableVtxz", &fTreeTrackVariableVtxz, "fTreeTrackVariableVtxz/F");
  fTreeEvent->Branch("fTreeTrackVariableNTrack", &fTreeTrackVariableNTrack, "fTreeTrackVariableNTrack/I");
  fTreeEvent->Branch("fTreeTrackVariableCharge", &fTreeTrackVariableCharge, "fTreeTrackVariableCharge[fTreeTrackVariableNTrack]/I");

  fTreeEvent->Branch("fTreeTrackVariableDCAXY", &fTreeTrackVariableDCAXY, "fTreeTrackVariableDCAXY[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableDCAZ", &fTreeTrackVariableDCAZ, "fTreeTrackVariableDCAZ[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableNCR", &fTreeTrackVariableNCR, "fTreeTrackVariableNCR[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableITSchi2", &fTreeTrackVariableITSchi2, "fTreeTrackVariableITSchi2[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableTPCchi2", &fTreeTrackVariableTPCchi2, "fTreeTrackVariableTPCchi2[fTreeTrackVariableNTrack]/F");

  fTreeEvent->Branch("fTreeTrackVariabledeuteronnsigmaTPC", &fTreeTrackVariabledeuteronnsigmaTPC, "fTreeTrackVariabledeuteronnsigmaTPC[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableprotonnsigmaTPC", &fTreeTrackVariableprotonnsigmaTPC, "fTreeTrackVariableprotonnsigmaTPC[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariablemasssquare", &fTreeTrackVariablemasssquare, "fTreeTrackVariablemasssquare[fTreeTrackVariableNTrack]/F");

  fTreeEvent->Branch("fTreeTrackVariableMomentumPx", &fTreeTrackVariableMomentumPx, "fTreeTrackVariableMomentumPx[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPy", &fTreeTrackVariableMomentumPy, "fTreeTrackVariableMomentumPy[fTreeTrackVariableNTrack]/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPz", &fTreeTrackVariableMomentumPz, "fTreeTrackVariableMomentumPz[fTreeTrackVariableNTrack]/F");
  
  fTreeEvent->Branch("fTreeTrackVariableNTrack_d_Gen", &fTreeTrackVariableNTrack_d_Gen, "fTreeTrackVariableNTrack_d_Gen/I");
  fTreeEvent->Branch("fTreeTrackVariableNTrack_p_Gen", &fTreeTrackVariableNTrack_p_Gen, "fTreeTrackVariableNTrack_p_Gen/I");

  fTreeEvent->Branch("fTreeTrackVariableVtxz_Gen", &fTreeTrackVariableVtxz_Gen, "fTreeTrackVariableVtxz_Gen/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPx_d_Gen", &fTreeTrackVariableMomentumPx_d_Gen, "fTreeTrackVariableMomentumPx_d_Gen[fTreeTrackVariableNTrack_d_Gen]/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPy_d_Gen", &fTreeTrackVariableMomentumPy_d_Gen, "fTreeTrackVariableMomentumPy_d_Gen[fTreeTrackVariableNTrack_d_Gen]/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPz_d_Gen", &fTreeTrackVariableMomentumPz_d_Gen, "fTreeTrackVariableMomentumPz_d_Gen[fTreeTrackVariableNTrack_d_Gen]/F");

  fTreeEvent->Branch("fTreeTrackVariableMomentumPx_p_Gen", &fTreeTrackVariableMomentumPx_p_Gen, "fTreeTrackVariableMomentumPx_p_Gen[fTreeTrackVariableNTrack_p_Gen]/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPy_p_Gen", &fTreeTrackVariableMomentumPy_p_Gen, "fTreeTrackVariableMomentumPy_p_Gen[fTreeTrackVariableNTrack_p_Gen]/F");
  fTreeEvent->Branch("fTreeTrackVariableMomentumPz_p_Gen", &fTreeTrackVariableMomentumPz_p_Gen, "fTreeTrackVariableMomentumPz_p_Gen[fTreeTrackVariableNTrack_p_Gen]/F");
   

  PostData(1, fTreeEvent);
}

// Bool_t AliAnalysisTaskDeutFlucpp::IsMCEventSelected(TObject* obj){

// 	Bool_t isSelected = kTRUE;

// 	AliMCEvent *event = 0x0;
// 	event = dynamic_cast<AliMCEvent*>(obj);
// 	if( !event ) 
// 		isSelected = kFALSE;

// 	return isSelected;
// }

//________________________________________________________________________
void AliAnalysisTaskDeutFlucpp::UserExec(Option_t *)
{

  // Main loop
  // Called for each event

  AliESDEvent *lESDevent = 0x0; 
  fMCevent = 0x0;
  fMCstack = 0x0;

  lESDevent = dynamic_cast<AliESDEvent *>(InputEvent());
  if (!lESDevent)
  {
    AliWarning("ERROR: lESDevent not available \n");
    // PostData(1,fTreeEvent);
    return;
  }
  if(fUseMC)
  {
  fMCevent = MCEvent();
    if (!fMCevent) {
      Printf("ERROR: Could not retrieve MC event \n");
      //cout << "Name of the file with pb :" <<  fInputHandler->GetTree()->GetCurrentFile()->GetName() << endl;
      return;
    }

    fMCstack = fMCevent->Stack();
    if (!fMCstack) {
      Printf("ERROR: Could not retrieve MC stack \n");
      //cout << "Name of the file with pb :" <<  fInputHandler->GetTree()->GetCurrentFile()->GetName() << endl;
      return;
    }
  }

  AliHeader *headerMC;
	Bool_t isGoodVtxPosMC = kFALSE;
  
 if(fUseMC)
    {
    //Int_t ntracks_Gen = fMCstack->GetNtrack();
    Int_t str_d_Gen = 0;
    Int_t str_p_Gen = 0;
    Bool_t isdeuteron_Gen;
    Bool_t isproton_Gen;
    //const Int_t label = TMath::Abs(esdt->GetLabel());
    //--------generated-------------

		headerMC = fMCevent->Header();
		AliGenEventHeader *genHeader = headerMC->GenEventHeader();
		TArrayF vtxMC(3); // primary vertex  MC
		vtxMC[0] = 9999;
		vtxMC[1] = 9999;
		vtxMC[2] = 9999; // initialize with dummy
		if (genHeader)
			genHeader->PrimaryVertex(vtxMC);
    //cout<<"vtx ====" << vtxMC[2]<<endl;

    fTreeTrackVariableVtxz_Gen = vtxMC[2];

		if (TMath::Abs(vtxMC[2]) <= 10)
			isGoodVtxPosMC = kTRUE;

    if (isGoodVtxPosMC)
    {
    Int_t noGenMCtracks = fMCevent->GetNumberOfTracks();
    for (Int_t i = 0; i < noGenMCtracks; i++) {
        
      AliMCParticle* particle = (AliMCParticle*)fMCevent->GetTrack(i);
      if (!particle)
	    {
	    cout<<"Could not find track in MC generated loop !!!"<<endl;
	    continue;
	    }
		  if (!particle->IsPhysicalPrimary()) continue;
		  if (TMath::Abs(particle->Eta()) > 0.8) continue;
		  if (AliAnalysisUtils::IsParticleFromOutOfBunchPileupCollision(i,fMCevent)) continue;

        Int_t trk_PID_gen = particle->PdgCode();
        Float_t trk_eta_gen = particle->Eta();
        Float_t trk_pt_gen = particle->Pt();
        Float_t trk_px_gen = particle->Px();
        Float_t trk_py_gen = particle->Py();
        Float_t trk_pz_gen = particle->Pz();
        isdeuteron_Gen = kFALSE;
        isproton_Gen = kFALSE;


      if (trk_pt_gen > 0.8 && trk_pt_gen <= 2.0 && trk_PID_gen == -1000010020)
        isdeuteron_Gen = kTRUE;

      if (trk_pt_gen > 0.4 && trk_pt_gen <= 1.0 && trk_PID_gen == -2212)
        isproton_Gen = kTRUE;

      if (isdeuteron_Gen)
       {
      //   fTreeTrackVariableCharge_d_Gen[str_d_Gen] = trk_charge_gen;
         fTreeTrackVariableMomentumPx_d_Gen[str_d_Gen] = trk_px_gen;
         fTreeTrackVariableMomentumPy_d_Gen[str_d_Gen] = trk_py_gen;
         fTreeTrackVariableMomentumPz_d_Gen[str_d_Gen] = trk_pz_gen;
         str_d_Gen = str_d_Gen+1;
       }

       if (isproton_Gen)
       {
      //   fTreeTrackVariableCharge_p_Gen[str_p_Gen] = trk_charge_gen;
         fTreeTrackVariableMomentumPx_p_Gen[str_p_Gen] = trk_px_gen;
         fTreeTrackVariableMomentumPy_p_Gen[str_p_Gen] = trk_py_gen;
         fTreeTrackVariableMomentumPz_p_Gen[str_p_Gen] = trk_pz_gen;
         str_p_Gen = str_p_Gen+1;
       }
        
      }
      fTreeTrackVariableNTrack_p_Gen = str_p_Gen;
      fTreeTrackVariableNTrack_d_Gen = str_d_Gen; 
     }
    }
  //IsMCEventSelected = 1;
  
  ////tigger/////////////
  UInt_t maskIsSelected = ((AliInputEventHandler *)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
  Bool_t isSelected = 0;

  isSelected = (maskIsSelected & fTriggerMask); // AliVEvent::kINT7)

  //IsMCEventSelected(fMCevent);

  if (!isSelected)
  {
    PostData(1, fTreeEvent);
    return;
  }

  Bool_t EventAccepted;
  EventAccepted = fEventCuts.AcceptEvent(lESDevent);
  if (!EventAccepted)
  {
    PostData(1, fTreeEvent);
    return;
  }

  // primary vertex
  //
  //    const AliVVertex* *vertex = fEventCuts.GetPrimaryVertex(lESDevent);
  const AliVVertex *vertex = lESDevent->GetPrimaryVertex();
  if (vertex->GetNContributors() < 1)
  {
    PostData(1, fTreeEvent);
    return;
  }
  if (!vertex)
  {
    PostData(1, fTreeEvent);
    return;
  }
  if (TMath::Abs(vertex->GetZ()) > 10.0)
  {
    PostData(1, fTreeEvent);
    return;
  }

  //////centrality selection/////////
  Float_t lV0M;
  Int_t lEvSelCode = 300;
  AliMultSelection *MultSelection = (AliMultSelection *)lESDevent->FindListObject("MultSelection");
  if (!MultSelection)
  {
    AliWarning("AliMultSelection object not found!");
    PostData(1, fTreeEvent);
    return;
  }
  else
  {
    lV0M = MultSelection->GetMultiplicityPercentile("V0M");
  }

  fTreeTrackVariableCentrality = lV0M;
  fTreeTrackVariableVtxz = vertex->GetZ();

  Int_t ntracks = lESDevent->GetNumberOfTracks();
  Int_t str = 0;

  Double_t mass_square = -999.0;
  Double_t p[3] = {-999.0, -999.0, -999.0};
  Float_t nSigma_proton_TPC = -999.0;
  Float_t nSigma_deuteron_TPC = -999.0;
  Float_t nSigma_proton_TOF = -999;
  Float_t nSigma_deuteron_TOF = -999;
  Int_t trackcharge = -999;
  float itschi2 = -999.0;
  float tpcchi2 = -999.0;
  float itsclust = -999.0;
  float tpcclust = -999.0;
  Float_t dcaxy = -999.0;
  Float_t dcaz = -999.0;
  Float_t ncr = -999.0;
  float Track_momentum = -999.0;
  float Track_pt = -999.0;
  float tpc_mom = -999.0;
  Bool_t isdeuteron;
  Bool_t isproton;
  for (Int_t itr = 0; itr < ntracks; itr++)
  {
    AliVTrack *track = (AliVTrack *)lESDevent->GetTrack(itr);
    if (!track)
      continue;
    AliESDtrack *esdt = dynamic_cast<AliESDtrack *>(track);

    if (!esdt)
      continue;
    if (!fESDtrackCuts->AcceptTrack(esdt))
      continue;

    ////////Track information/////////////////
    esdt->GetImpactParameters(dcaxy, dcaz);
    ncr = esdt->GetTPCCrossedRows();
    esdt->PxPyPz(p);
    trackcharge = esdt->Charge();
    Track_momentum = esdt->GetP();
    Track_pt = TMath::Sqrt(p[0] * p[0] + p[1] * p[1]);
    itsclust = esdt->GetITSclusters(0);
    tpcclust = esdt->GetTPCclusters(0);
    if (tpcclust != 0)
      tpcchi2 = esdt->GetTPCchi2() / tpcclust;
    if (itsclust != 0)
      itschi2 = esdt->GetITSchi2() / itsclust;
    if (MatchTOF(track))
    {
      mass_square = GetTOFBeta(track);
    }
    else
    {
      mass_square = -100.0;
    }

    nSigma_proton_TPC = fPIDResponse->NumberOfSigmasTPC(esdt, AliPID::kProton);
    nSigma_deuteron_TPC = fPIDResponse->NumberOfSigmasTPC(esdt, AliPID::kDeuteron);

    isdeuteron = kFALSE;
    isproton = kFALSE;

    if (Track_pt > 0.8 && Track_pt <= 1.2 && TMath::Abs(nSigma_deuteron_TPC) < 5.0)
      isdeuteron = kTRUE;
    if (Track_pt > 1.2 && Track_pt <= 2.0 && TMath::Abs(nSigma_deuteron_TPC) < 5.0 && mass_square > 2 && mass_square < 5.0)
      isdeuteron = kTRUE;

    if (Track_pt > 0.4 && Track_pt <= 0.6 && TMath::Abs(nSigma_proton_TPC) < 5.0)
      isproton = kTRUE;
    if (Track_pt > 0.6 && Track_pt <= 1.0 && TMath::Abs(nSigma_proton_TPC) < 5.0 && mass_square > 0.5 && mass_square < 1.2)
      isproton = kTRUE;

    if (isdeuteron || isproton)
    {
      fTreeTrackVariableCharge[str] = trackcharge;
      fTreeTrackVariableDCAXY[str] = dcaxy;
      fTreeTrackVariableDCAZ[str] = dcaz;
      fTreeTrackVariableNCR[str] = ncr;
      fTreeTrackVariableITSchi2[str] = itschi2;
      fTreeTrackVariableTPCchi2[str] = tpcchi2;
      fTreeTrackVariabledeuteronnsigmaTPC[str] = nSigma_deuteron_TPC;
      fTreeTrackVariableprotonnsigmaTPC[str] = nSigma_proton_TPC;
      fTreeTrackVariablemasssquare[str] = mass_square;
      fTreeTrackVariableMomentumPx[str] = p[0];
      fTreeTrackVariableMomentumPy[str] = p[1];
      fTreeTrackVariableMomentumPz[str] = p[2];
      str = str + 1;
    }

  }
  fTreeTrackVariableNTrack = str;

  fTreeEvent->Fill();
  PostData(1, fTreeEvent);
}
//________________________________________________________________________
void AliAnalysisTaskDeutFlucpp::Terminate(Option_t *)
{
}
//----------------------------------------------------------------------------
Double_t AliAnalysisTaskDeutFlucpp::MyRapidity(Double_t rE, Double_t rPz) const
{
  // Local calculation for rapidity
  Double_t ReturnValue = -100;
  if ((rE - rPz + 1.e-13) != 0 && (rE + rPz) != 0)
  {
    ReturnValue = 0.5 * TMath::Log((rE + rPz) / (rE - rPz + 1.e-13));
  }
  return ReturnValue;
}
//----------------------------------------------------------------------------
void AliAnalysisTaskDeutFlucpp::SetPersonalESDtrackCuts(AliESDtrackCuts *trackcuts)
{
  if (fESDtrackCuts)
  {
    delete fESDtrackCuts;
    fESDtrackCuts = 0x0;
  }
  fESDtrackCuts = trackcuts;
}
//----------------------------------------------------------------------------
Double_t AliAnalysisTaskDeutFlucpp::GetTOFBeta(AliVTrack *vtrack)
{
  AliESDtrack *esdtrack = dynamic_cast<AliESDtrack *>(vtrack);
  if (!esdtrack)
    return -1;
  const Double_t c = 2.99792457999999984e-02;
  Double_t p = esdtrack->GetTPCmomentum();
  Double_t l = esdtrack->GetIntegratedLength();
  Double_t trackT0 = fPIDResponse->GetTOFResponse().GetStartTime(p);
  Double_t timeTOF = esdtrack->GetTOFsignal() - trackT0;
  Double_t mass_square = (p * p) * (TMath::Power(c * timeTOF / l, 2.0) - 1);
  return mass_square;
}
//______________________________________________________________________________
Bool_t AliAnalysisTaskDeutFlucpp::MatchTOF(AliVTrack *vtrack)
{
  if (!vtrack)
  {
    AliWarning("NULL argument: impossible to check status");
    return kFALSE;
  }
  if (!(vtrack->GetStatus() & AliESDtrack::kTOFout))
    return kFALSE;
  if (!(vtrack->GetStatus() & AliESDtrack::kTIME))
    return kFALSE;

  // if (!(vtrack->GetStatus() & AliESDtrack::kTOFpid)) return kFALSE;

  // float probMis = fPIDResponse->GetTOFMismatchProbability(vtrack);
  // if(probMis>0.01) return kFALSE;

  AliESDtrack *esdtrack = dynamic_cast<AliESDtrack *>(vtrack);
  Double_t l = esdtrack->GetIntegratedLength();
  if (l < 350)
    return kFALSE;

  return kTRUE;
}
