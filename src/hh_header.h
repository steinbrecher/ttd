#ifndef HH_HEADER_SEEN
#define HH_HEADER_SEEN

#define MAXINPCHANS 8

#pragma pack(8) //structure alignment to 8 byte boundaries

// These are substructures used further below 

typedef struct { 
  float Start;
  float Step;
  float End;  } tParamStruct;

typedef struct { 
  int32_t MapTo;
  int32_t Show; } tCurveMapping;

typedef	struct { 
  int32_t ModelCode;
  int32_t VersionCode; } tModuleInfo;

// T2 Record
typedef union { 
  uint32_t   allbits;
  struct {
    unsigned timetag  :25;
    unsigned channel  :6;
    unsigned special  :1;
  } bits; 
} tT2Rec;

// T3 Record
typedef union { 
  uint32_t allbits;
  struct {
    unsigned nsync		:10; 	// numer of sync period
    unsigned dtime		:15;    // delay from last sync in units of chosen resolution 
    unsigned channel	:6;
    unsigned special	:1;
  } bits;
} tT3Rec;

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
} tTRec;

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
  char CommentField[256]; } TxtHdr;

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
  tCurveMapping DispCurves[8];	
  tParamStruct Params[3];
  int32_t RepeatMode;
  int32_t RepeatsPerCurve;
  int32_t RepeatTime;
  int32_t RepeatWaitTime;
  char ScriptName[20];	} BinHdr;

double SyncPeriod;


// The next is a header carrying hardware information

struct {		
  char HardwareIdent[16];   
  char HardwarePartNo[8]; 
  int32_t  HardwareSerial; 
  int32_t  nModulesPresent;     
  tModuleInfo ModuleInfo[10]; //up to 10 modules can exist
  double BaseResolution;
  int64_t InputsEnabled;   //a bitfield
  int32_t InpChansPresent;  //this determines the number of ChannelHeaders below!
  int32_t RefClockSource;
  int32_t ExtDevices;     //a bitfield
  int32_t MarkerSettings; //a bitfield
  int32_t SyncDivider;
  int32_t SyncCFDLevel;
  int32_t SyncCFDZeroCross;
  int32_t SyncOffset;			} MainHardwareHdr;


//How many of the following array elements are actually present in the file 
//is indicated by InpChansPresent above. Here we allocate the possible maximum.

struct{ 
  int32_t InputModuleIndex;
  int32_t InputCFDLevel;
  int32_t InputCFDZeroCross; 
  int32_t InputOffset;		} InputChannelSettings[MAXINPCHANS]; 

//Up to here the header was identical to that of HHD files.
//The following header sections are specific for the TT modes 

//How many of the following array elements are actually present in the file 
//is indicated by InpChansPresent above. Here we allocate the possible maximum.

int32_t InputRate[MAXINPCHANS];

//The following exists only once				

struct{	
  int32_t SyncRate;
  int32_t StopAfter;
  int32_t StopReason;
  int32_t ImgHdrSize;
  int64_t nRecords;	} TTTRHdr;


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


#ifdef PQ_G2_HEADER_SEEN
void write_g2_properties() {
  // Calculations for arguments
  double sync_period = 1e12/TTTRHdr.SyncRate;
  double resolution = BinHdr.Resolution;
  int channels = MainHardwareHdr.InpChansPresent;
  int channel_pairs = channels * (channels-1)/2;
  int meas_mode = BinHdr.MeasMode;

  int file_format_version;
  if (strcmp(TxtHdr.FormatVersion, "1.0")==0) {
    file_format_version = 1;
  }
  else if (strcmp(TxtHdr.FormatVersion, "2.0")==0) {
    file_format_version = 2;
  }

  // g2 mode argument processing
  g2_properties.sync_period = sync_period;
  g2_properties.resolution = resolution;
  g2_properties.correlation_window = cli_args.correlation_window;
  g2_properties.bin_time = cli_args.bin_time;
  g2_properties.channels = channels;
  g2_properties.channel_pairs = channel_pairs;
  g2_properties.meas_mode = meas_mode;
  g2_properties.file_format_version = file_format_version;
}
#endif

#ifdef PQ_CONVERT
struct{
  int channels;
  int meas_mode; // 2 for .ht2 and 3 for .ht3
  int file_format_version;
  uint64_t resolution;
  uint64_t sync_period;
} convert_properties;

void write_convert_properties() {
  // Calculations for arguments
  uint64_t sync_period = (uint64_t)round(1e12/TTTRHdr.SyncRate);
  uint64_t resolution = (uint64_t)round(BinHdr.Resolution);
  int channels = MainHardwareHdr.InpChansPresent;
  int channel_pairs = channels * (channels-1)/2;
  int meas_mode = BinHdr.MeasMode;

  int file_format_version;
  if (strcmp(TxtHdr.FormatVersion, "1.0")==0) {
    file_format_version = 1;
  }
  else if (strcmp(TxtHdr.FormatVersion, "2.0")==0) {
    file_format_version = 2;
  }

  // convert mode argument prcoessing
  convert_properties.channels = channels; 
  convert_properties.meas_mode = meas_mode;
  convert_properties.resolution = resolution;
  convert_properties.sync_period = sync_period;
  convert_properties.file_format_version = file_format_version;
}
#endif

int read_header(FILE *fpin)
{
  int i;
  
  if (fread(&TxtHdr, 1, sizeof(TxtHdr), fpin) != sizeof(TxtHdr)) {
    return(-1);
  }

  if (fread( &BinHdr, 1, sizeof(BinHdr) ,fpin) != sizeof(BinHdr)) {
    return(-1);
  }

  if (fread( &MainHardwareHdr, 1, sizeof(MainHardwareHdr) ,fpin) != sizeof(MainHardwareHdr)) {
    return(-1);
  }

  for(i=0;i<MainHardwareHdr.InpChansPresent;++i) {
      if (fread( &(InputChannelSettings[i]), 1, sizeof(InputChannelSettings[i]) ,fpin) 
	  != sizeof(InputChannelSettings[i]))
	return(-1);
    }

  for(i=0;i<MainHardwareHdr.InpChansPresent;++i) {
      if (fread( &(InputRate[i]), 1, sizeof(InputRate[i]) ,fpin) != sizeof(InputRate[i]))
	return(-1);
    }

  if (fread(&TTTRHdr, 1, sizeof(TTTRHdr), fpin) != sizeof(TTTRHdr)) {
    return(-1);
  }

  // Print header information
  printf("\n***************************** Header Information *****************************\n");
  printf("File Mode: T%d\n", BinHdr.MeasMode);
  printf("File Version: %s\n", TxtHdr.FormatVersion);
  printf("Number of channels: %d\n", MainHardwareHdr.InpChansPresent);
  printf("Sync Rate: %g MHz\n", (double)TTTRHdr.SyncRate*1e-6);
  printf("Sync Period: %g ns\n", 1e9/TTTRHdr.SyncRate);
  printf("Resolution: %g ps\n", BinHdr.Resolution);
  printf("Acquisition Time: %g s\n", ((double)BinHdr.Tacq)/1e3);
  printf("Number of Records: %" PRIu64 "\n", TTTRHdr.nRecords);

  // Multiply TTTRHdr.ImgHdrSize by 4 because each entry is a 4-byte (32 bit) record
  fseek(fpin, TTTRHdr.ImgHdrSize*4, SEEK_CUR);


  return(0);
}

#endif /* HH_HEADER_SEEN */
