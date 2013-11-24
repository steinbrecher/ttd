#ifndef HH_HEADER_SEEN
#define HH_HEADER_SEEN

#define MAXINPCHANS 8

#pragma pack(8) //structure alignment to 8 byte boundaries

// These are substructures used further below 

typedef struct { 
  float Start;
  float Step;
  float End;  } pq_hh_params_t;

typedef struct { 
  int32_t MapTo;
  int32_t Show; } pq_hh_curvemap_t;

typedef	struct { 
  int32_t ModelCode;
  int32_t VersionCode; } pq_hh_modinfo_t;

// T2 Record
typedef union { 
  uint32_t   allbits;
  struct {
    unsigned timetag  :25;
    unsigned channel  :6;
    unsigned special  :1;
  } bits; 
} pq_hh_t2rec_t;

// T3 Record
typedef union { 
  uint32_t allbits;
  struct {
    unsigned nsync		:10; 	// numer of sync period
    unsigned dtime		:15;    // delay from last sync in units of chosen resolution 
    unsigned channel	:6;
    unsigned special	:1;
  } bits;
} pq_hh_t3rec_t;

// Combining into a union allows the program reading the file script to be agnostic to T2 vs. T3
// until it actually reads the information out
typedef union { 
  uint32_t allbits;
  struct {
    unsigned timetag  :25;
    unsigned channel  :6;
    unsigned special  :1; // or sync, if channel==0
  } T2bits; 
  struct {
    unsigned nsync		:10; 	// numer of sync period
    unsigned dtime		:15;    // delay from last sync in units of chosen resolution 
    unsigned channel	:6;
    unsigned special	:1;
  } T3bits;
} pq_hh_rec_t;

////////////////////////////////////////////////////////////////////////////////
//                     Begin Header Format Definition.                        //
////////////////////////////////////////////////////////////////////////////////

// The following represents the readable ASCII file header portion. 

struct {		
  char Ident[16];				// "HydraHarp"
  char FormatVersion[6];		// file format version
  char CreatorName[18];		// name of creating software
  char CreatorVersion[12];	// version of creating software
  char FileTime[18];
  char CRLF[2];
  char CommentField[256]; } pq_hh_hdr_txt;

// The following is binary file header information indentical to that in HHD files.
// Note that some items are not meaningful in the time tagging modes.

struct {		
  int32_t Curves;
  int32_t BitsPerRecord;		// data of one event record has this many bits
  int32_t ActiveCurve;
  int32_t MeasMode;
  int32_t SubMode;
  int32_t Binning;			
  double Resolution;		// in ps
  int32_t Offset;				
  int32_t Tacq;				// in ms
  uint32_t StopAt;
  int32_t StopOnOvfl;
  int32_t Restart;
  int32_t DispLinLog;
  int32_t DispTimeFrom;		// 1ns steps
  int32_t DispTimeTo;
  int32_t DispCountsFrom;
  int32_t DispCountsTo;
  pq_hh_curvemap_t DispCurves[8];	
  pq_hh_params_t Params[3];
  int32_t RepeatMode;
  int32_t RepeatsPerCurve;
  int32_t RepeatTime;
  int32_t RepeatWaitTime;
  char ScriptName[20];	} pq_hh_hdr_bin;

double SyncPeriod;


// The next is a header carrying hardware information

struct {		
  char HardwareIdent[16];   
  char HardwarePartNo[8]; 
  int32_t  HardwareSerial; 
  int32_t  nModulesPresent;     
  pq_hh_modinfo_t ModuleInfo[10]; //up to 10 modules can exist
  double BaseResolution;
  int64_t InputsEnabled;   //a bitfield
  int32_t InpChansPresent;  //this determines the number of ChannelHeaders below!
  int32_t RefClockSource;
  int32_t ExtDevices;     //a bitfield
  int32_t MarkerSettings; //a bitfield
  int32_t SyncDivider;
  int32_t SyncCFDLevel;
  int32_t SyncCFDZeroCross;
  int32_t SyncOffset;			} pq_hh_hdr_hardware;


//How many of the following array elements are actually present in the file 
//is indicated by InpChansPresent above. Here we allocate the possible maximum.

struct{ 
  int32_t InputModuleIndex;
  int32_t InputCFDLevel;
  int32_t InputCFDZeroCross; 
  int32_t InputOffset;		} pq_hh_hdr_chansettings[MAXINPCHANS]; 

//Up to here the header was identical to that of HHD files.
//The following header sections are specific for the TT modes 

//How many of the following array elements are actually present in the file 
//is indicated by InpChansPresent above. Here we allocate the possible maximum.

int32_t pq_hh_hdr_inprate[MAXINPCHANS];

//The following exists only once				

struct{	
  int32_t SyncRate;
  int32_t StopAfter;
  int32_t StopReason;
  int32_t ImgHdrSize;
  int64_t nRecords;	} pq_hh_hdr_tttr;

struct{
  double sync_period;
  double resolution;
  double correlation_window;
  double bin_time;
  int channels;
  int channel_pairs;
  int meas_mode; // 2 for .ht2 and 3 for .ht3
  int file_format_version;
} g2_properties;

struct{
  int channels;
  int meas_mode; // 2 for .ht2 and 3 for .ht3
  int file_format_version;
  uint64_t resolution;
  uint64_t sync_period;
} convert_properties;


#endif /* HH_HEADER_SEEN */
