////////////////////////////////////////////////////////////////////////
//
// PanTypes.hh
//
// Data types for pan 
//
// Author: R. Holmes  Sep 2001
//
////////////////////////////////////////////////////////////////////////

#ifndef PAN_PanTypes
#define PAN_PanTypes

typedef UInt_t RunNumber_t;
typedef UInt_t EventNumber_t;
typedef UInt_t SlotNumber_t;
typedef Int_t ErrCode_t;
enum ECutType { LowBeamCut, BeamBurpCut, OversampleCut, SequenceCut, MaxCuts };
enum EHelicity { RightHeli, LeftHeli, UnkHeli };
enum EPairSynch { FirstPS, SecondPS };
enum EPairType { FromPair };

#endif
