#ifndef _PQ_PARSE_HEADER_SEEN
#define _PQ_PARSE_HEADER_SEEN

#include "pq_records.h" // Include for all code linking pq_parse

// Constants
#define PQ_HH 1
#define PQ_HH_TEXT "HydraHarp"

#define PQ_PH 2
#define PQ_PH_TEXT "PicoHarp 300"

#define PQ_HH_V1 1
#define PQ_HH_V1_TEXT "1.0"

#define PQ_HH_V2 2
#define PQ_HH_V2_TEXT "2.0"

#define PQ_PH_V2 2
#define PQ_PH_V2_TEXT "2.0"

#define PQ_HH_MAX_CHANNELS 8

#define PQ_T2_MODE 2
#define PQ_T3_MODE 3

// Errors
#define PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED -1
#define PQ_FILETYPE_VERSION_NOT_RECOGNIZED -2

// Structures for parsing header
static struct {
  char instrument[16];
  char version[6];
  char creator_name[18];
  char creator_version[12];
  char datetime[18];
  char crlf[2];
  char comments[256];
  int32_t num_curves; // Irrelevant
  int32_t num_record_bits; // 32 in all known file versions
} pq_file_text;

typedef struct {
  int32_t map_to;
  int32_t show;
} pq_curve_t;

typedef struct {
  int32_t disp_linlog;
  uint32_t disp_info[4];
  pq_curve_t disp_curves[8];
} pq_dispblock_t;

typedef struct {
  float start;
  float step;
  float end; 
} pq_params_t;

typedef struct {
  int32_t mode;
  int32_t num_per_curve;
  int32_t time;
  int32_t wait_time;
} pq_repeat_block_t;

typedef struct {
  int32_t model_code;
  int32_t version_code;
} pq_hh_moduleinfo_t;

typedef struct {
  int32_t module_index;
  int32_t cfd_level;
  int32_t cfd_zero_cross;
  int32_t offset;
} pq_hh_chan_t;

static struct {
  int32_t sync_rate; // in Hz
  int32_t stop_after; // in ms
  int32_t stop_reason; // 0=timeover, 1=manual, 2=overflow, 3=error
  int32_t image_header_size; 
  int64_t num_records;
} pq_hh_tttr;

pq_hh_chan_t pq_hh_chanblock[PQ_HH_MAX_CHANNELS]; 
int32_t pq_hh_rates[PQ_HH_MAX_CHANNELS]; 

static struct {
  int32_t active_curve; // Irrelevant for tttr modes
  int32_t meas_mode; // 0=interactive, 1=reserved, 2=T2, 3=T3
  int32_t meas_sub_mode; // 0=oscilloscope, 1=integration, 2=TRES
  int32_t binning; // Irrelevant for tttr modes
  double resolution; // in ps
  int32_t offset; // Irrelevant for tttr modes
  int32_t acquire_time;	// in ms
  uint32_t stop_at_num_counts; // Irrelevant for tttr modes
  int32_t stop_on_overflow; // Irrelevant for tttr modes
  int32_t restart; // Irrelevant for tttr modes
  pq_dispblock_t dispblock;
  pq_params_t params[3];
  pq_repeat_block_t repeat; //Irrelevant for tttr modes
  char script_name[20];	
  char hardware_ident[16];
  char hardware_part_num[8];
  int32_t hardware_serial;
  int32_t num_modules;
  pq_hh_moduleinfo_t modules_info[10]; 
  double base_resolution; // in ps
  int64_t inputs_enabled; // bitfield for enabled inputs
  int32_t num_input_channels; 
  int32_t clock_source; // 0=internal, 1=external
  int32_t external_devices; // bitfield
  int32_t marker_settings; // bitfield
  int32_t sync_divider;
  int32_t sync_cfd_level; // in mV
  int32_t sync_cfd_zerocross; // in mV
  int32_t sync_offset; // in ps
} pq_hh_header;

typedef struct {
  int32_t input_type; // 0=custom, 1=NIM, 2=TTL
  int32_t input_level; // in mV
  int32_t input_edge; // 0=falling, 1=rising
  int32_t cfd_present; // 0=no, 1=yes
  int32_t cfd_level; // in mV
  int32_t cfd_zero_cross; // in mV
} pq_ph_router_info_t;

// Note: as of PicoHarp file version 2.0, pq_ph_board_info occurs only once in a file, but in 
// later versions this may change. 
typedef struct {
  char hardware_name[16]; // Currently "PicoHarp 300"
  char hardware_version[8]; // Currently "1.0" or "2.0"
  int32_t hardware_serial;
  int32_t sync_divider; // Note that chan0 == sync
  int32_t chan0_cfd_zerocross; // in mV
  int32_t chan0_cfd_level; // in mV
  int32_t chan1_cfd_zerocross; // in mV
  int32_t chan1_cfd_level; // in mV
  float resolution; // In nanoseconds!!!
  int32_t router_model; // 0=none, 1=PHR_402, 2=PHR_403, 3=PHR_800
  int32_t router_enabled; // 0=disabled, 1=enabled
  pq_ph_router_info_t router_info[4]; // As of version 2.0, number of router channels must be 4.
} pq_ph_board_info_t;

static struct {
  int32_t num_routing_channels; // 1 or 4
  int32_t num_boards; // Currently 1, but may change in future. See comment above pq_ph_board_info
  int32_t active_curve; // Irrelevant for tttr modes
  int32_t meas_mode; // 0=interactive, 1=reserved, 2=T2, 3=T3
  int32_t meas_sub_mode; // 0=oscilloscope, 1=integration, 2=TRES
  int32_t binning; // Irrelevant for tttr modes, but notable that it's called RangeNo in PicoHarp manual
  int32_t offset; // Irrelevant for tttr modes
  int32_t acquire_time; // in ms
  int32_t stop_at_num_counts; // Irrelevant for tttr modes
  int32_t stop_on_overflow; // Irrelevant for tttr modes
  int32_t restart; // Irrelevant for tttr modes
  pq_dispblock_t dispblock; // Irrelevant for tttr modes
  pq_params_t params[3]; // Irrelevant for tttr modes
  pq_repeat_block_t repeat; //Irrelevant for tttr modes
  char ScriptName[20];	
  pq_ph_board_info_t board_info;
  // This is where the T2/T3 header begins
  int32_t external_devices; // Bitwise coded
  int32_t reserved[2];
  int32_t chan0_inp_rate;
  int32_t chan1_inp_rate;
  int32_t stop_after; // in ms
  int32_t stop_reason; // 0=timeover, 1=manual, 2=overflow
  int32_t num_records;
  int32_t image_header_size;
} pq_ph_header;

typedef uint32_t pq_image_header_record;


// This is the output format
typedef struct {
  int32_t instrument;
  int32_t fmt_version;
  int32_t meas_mode;
  ttd_t resolution;
  int32_t sync_rate;
  ttd_t sync_period;
  int64_t num_records;
  int32_t num_channels;
} pq_fileinfo_t;


// Function API
int pq_parse_header(FILE *fp, pq_fileinfo_t *file_info);
int pq_parse_filetype(FILE *fp, pq_fileinfo_t *file_info);
void pq_printf_file_info(pq_fileinfo_t *file_info);

#endif // _PQ_PARSE_HEADER_SEEN
