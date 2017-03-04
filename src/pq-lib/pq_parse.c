#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>

#include "ttd.h"
#include "pq_parse.h"


// Determine header record type in ptu files
int check_record_type(long long RecordType, pq_fileinfo_t *file_info) {
  switch (RecordType)
  {
    case rtPicoHarpT2:
      //printf("PicoHarp T2 data");
      file_info->instrument = PQ_PH;
      file_info->fmt_version = PQ_PH_V2;
      file_info->meas_mode = PQ_T2_MODE;
      break;
    case rtPicoHarpT3:
      //printf("PicoHarp T3 data");
      file_info->instrument = PQ_PH;
      file_info->fmt_version = PQ_PH_V2;
      file_info->meas_mode = PQ_T3_MODE;
      break;
    case rtHydraHarpT2:
      //printf("HydraHarp V1 T2 data");
      file_info->instrument = PQ_HH;
      file_info->fmt_version = PQ_HH_V1;
      file_info->meas_mode = PQ_T2_MODE;
      break;
    case rtHydraHarpT3:
      //printf("HydraHarp V1 T3 data");
      file_info->instrument = PQ_HH;
      file_info->fmt_version = PQ_HH_V1;
      file_info->meas_mode = PQ_T3_MODE;
      break;
    case rtHydraHarp2T2:
      //printf("HydraHarp V2 T2 data");
      file_info->instrument = PQ_HH;
      file_info->fmt_version = PQ_HH_V2;
      file_info->meas_mode = PQ_T2_MODE;
      break;
    case rtHydraHarp2T3:
      //printf("HydraHarp V2 T3 data");
      file_info->instrument = PQ_HH;
      file_info->fmt_version = PQ_HH_V2;
      file_info->meas_mode = PQ_T3_MODE;
      break;

    default:
      printf("Unknown record type: 0x%llX\n", RecordType);
      return PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED;
  }
  return 0;
}

int pq_parse_ptu_header(FILE *fp, pq_fileinfo_t *file_info) {
  int retcode=0;
  char Buffer[40];
  char* AnsiBuffer;
  wchar_t* WideBuffer;

  fread(&TagHead, sizeof(TagHead), 1, fp);
  strcpy(Buffer, TagHead.Ident);

  if (TagHead.Idx > -1) {
    //sprintf(Buffer, "%s(%d)", TagHead.Ident,TagHead.Idx);
  }
  //printf("\n%-40s", Buffer);

  switch(TagHead.Typ) {
    case tyEmpty8:
      //printf("<Empty Tag>");
      if (strncmp(TagHead.Ident, FileTagEnd, sizeof(FileTagEnd))==0)
        retcode = PQ_DONE_PARSING;
      break;
    case tyBool8:
      // printf("%s", bool(TagHead.TagValue)?"True":"False");
      if (TagHead.TagValue==1){
        //printf("%s", "True");
      } else {
        //printf("%s", "False");
      }
      break;
    case tyInt8:
      if (strcmp(TagHead.Ident, TTTRTagTTTRRecType)==0) {// TTTR RecordType
        long long RecordType = TagHead.TagValue;
        retcode = check_record_type(RecordType,file_info);
      } else {
        //printf("%lld", TagHead.TagValue);
        // get some Values we need to analyse records
        if (strcmp(TagHead.Ident, TTTRTagNumRecords)==0) // Number of records
          file_info->num_records = (size_t)TagHead.TagValue;

        if (strcmp(TagHead.Ident, TTTRInputChannels)==0) // number of input chans
          file_info->num_channels = (size_t)TagHead.TagValue;
        if (strcmp(TagHead.Ident, TTTRSyncRate)==0) // sync rate
          file_info->sync_rate = (int32_t) TagHead.TagValue;
      }
      break;
    case tyBitSet64:
      //printf("0x%16.16llX", TagHead.TagValue);
      break;
    case tyColor8:
      //printf("0x%16.16llX", TagHead.TagValue);
      break;
    case tyFloat8:
      //printf("%E", *(double*)&(TagHead.TagValue));
      if (strcmp(TagHead.Ident, TTTRTagRes)==0) // Resolution for TCSPC-Decay
        //             Resolution = *(double*)&(TagHead.TagValue);
        // if (strcmp(TagHead.Ident, TTTRTagGlobRes)==0) // Global resolution for timetag
        file_info->resolution =
                (ttd_t) round(*(double*)&(TagHead.TagValue) * 1e12); // in ps
      break;
    case tyFloat8Array:
      //printf("<Float Array with %" PRIu64 " Entries>", (uint64_t)(TagHead.TagValue / sizeof(double)));
      // only seek the Data, if one needs the data, it can be loaded here
      fseek(fp, (long)TagHead.TagValue, SEEK_CUR);
      break;
    case tyTDateTime:
      // WE DON'T REALLY CARE ABOUT THE CREATION TIME
      // time_t CreateTime;
      // CreateTime = TDateTime_TimeT(*((double*)&(TagHead.TagValue)));
      // printf("%s", asctime(gmtime(&CreateTime)), "\0");
      break;
    case tyAnsiString:
      AnsiBuffer = (char*)calloc((size_t)TagHead.TagValue,1);
      fread(AnsiBuffer, (size_t)TagHead.TagValue, 1, fp);
      // if (Result!= TagHead.TagValue) {
      //   printf("\nIncomplete File.");
      //   free(AnsiBuffer);
      //   retcode = PQ_ILLEGAL_IDENTIFIER;
      // }
      //printf("%s", AnsiBuffer);
      free(AnsiBuffer);
      break;
    case tyWideString:
      WideBuffer = (wchar_t*)calloc((size_t)TagHead.TagValue,1);
      fread(WideBuffer, (size_t)TagHead.TagValue, 1, fp);
      // if (Result!= TagHead.TagValue) {
      //   printf("\nIncomplete File.");
      //   free(WideBuffer);
      //   retcode = PQ_ILLEGAL_IDENTIFIER;
      // }
      //printf(L"%s", WideBuffer);
      free(WideBuffer);
      break;
    case tyBinaryBlob:
      printf("<Binary Blob contains %lld Bytes>", TagHead.TagValue);
      // only seek the Data, if one needs the data, it can be loaded here
      fseek(fp, (long)TagHead.TagValue, SEEK_CUR);
      break;
    default:
      retcode = PQ_ILLEGAL_IDENTIFIER;
      break;
  }
  return retcode;
}

// This function is separated from the following because it's common to all
// PicoQuant files (PicoHarp and HydraHarp) and so might be useful to call
// from other code.
int pq_parse_filetype(FILE *fp, pq_fileinfo_t *file_info) {
  fread(&pq_identifier, sizeof(pq_identifier), 1, fp);
  if (strcmp(pq_identifier.Magic, "PQTTTR") == 0) {
    //fprintf(stderr, KHEAD2 "PTU File Format Version %s\n" KNRM, pq_identifier.Version);
    return(PTU_FILE);
  }

  fread(&pq_file_text, sizeof(pq_file_text), 1, fp);

  if (strcmp(pq_identifier.instrument, PQ_HH_TEXT)==0) {
    file_info->instrument = PQ_HH;
    if (strcmp(pq_file_text.version, PQ_HH_V1_TEXT)==0) {
      file_info->fmt_version = PQ_HH_V1;
      return(0);
    }
    else if (strcmp(pq_file_text.version, PQ_HH_V2_TEXT)==0) {
      file_info->fmt_version = PQ_HH_V2;
      return(0);
    }
    else {
      return(PQ_FILETYPE_VERSION_NOT_RECOGNIZED);
    }
  }
  else if (strcmp(pq_identifier.instrument, PQ_PH_TEXT)==0) {
    file_info->instrument = PQ_PH;
    if (strcmp(pq_file_text.version, PQ_PH_V2_TEXT)==0) {
      file_info->fmt_version = PQ_PH_V2;
      return(0);
    }
    else {
      return(PQ_FILETYPE_VERSION_NOT_RECOGNIZED);
    }
  }
  else {
    return(PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED);
  }
  // Shouldn't get here, so not returning anything. If compiler throws a warning, 
  // look for a bug above.
}

int pq_parse_header(FILE *fp, pq_fileinfo_t *file_info) {
  int retcode=0;
  int image_header_size;
  //printf("Parsing header...\n");
  // Instrument type and file format version  are necessary to decide how to parse the 
  // rest of the header
  retcode = pq_parse_filetype(fp, file_info);

  switch (retcode) {
  case PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED:
    printf("ERROR: Didn't recognize the instrument type in this file.\n");
    printf("       Is it a PicoHarp or HydraHarp time-tagged file?\n");
    goto exit_block;

  case PQ_FILETYPE_VERSION_NOT_RECOGNIZED:
    printf("ERROR: File version %s not supported.\n", pq_file_text.version);
    goto exit_block;

  default:
    if (retcode < 0) {
      // Really shouldn't get here
      printf("Warning: pq_parse_filetype raised unrecognized error. Please report.\n");
      goto exit_block;
    }
    break;
  }

  if (retcode == PTU_FILE) {
    retcode = 0;
    while (retcode != PQ_DONE_PARSING) {
      retcode = pq_parse_ptu_header(fp, file_info);
      switch (retcode) {
        case PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED:
          printf("ERROR: Didn't recognize the instrument type in this file.\n");
          printf("       Is it a PicoHarp or HydraHarp time-tagged file?\n");
          goto exit_block;

        case PQ_FILETYPE_VERSION_NOT_RECOGNIZED:
          printf("ERROR: File version %s not supported.\n", pq_file_text.version);
          goto exit_block;

        case PQ_ILLEGAL_IDENTIFIER:
          printf("ERROR: Illegal Type identifier found! Broken file?");
          goto exit_block;

        default:
          if (retcode < 0) {
            // Really shouldn't get here
            printf("Warning: pq_parse_filetype raised unrecognized error.\n");
            goto exit_block;
          }
          break;
      }
    }
  }

  else if (file_info->instrument == PQ_HH) {
    // Read most of header, including # of channels
    fread(&pq_hh_header, sizeof(pq_hh_header), 1, fp);

    // Read channel-dependent data
    fread(&pq_hh_chanblock, sizeof(pq_hh_chan_t), (size_t)pq_hh_header.num_input_channels, fp);
    fread(&pq_hh_rates, sizeof(int32_t), (size_t)pq_hh_header.num_input_channels, fp);
    // Read tttr data
    fread(&pq_hh_tttr, sizeof(pq_hh_tttr), 1, fp);

    image_header_size = pq_hh_tttr.image_header_size;

    // Write the shared data structure information
    file_info->meas_mode = pq_hh_header.meas_mode;
    file_info->resolution = (ttd_t) round(pq_hh_header.resolution);
    file_info->sync_rate = pq_hh_tttr.sync_rate;
    file_info->num_records = (size_t)pq_hh_tttr.num_records;
    file_info->num_channels = (size_t)pq_hh_header.num_input_channels;
    fseek(fp, image_header_size*sizeof(pq_image_header_record), SEEK_CUR);
  }
  else if (file_info->instrument == PQ_PH) {
    // Reads through everything but the image header. PicoHarp has a nice static header!
    fread(&pq_ph_header, sizeof(pq_ph_header), 1, fp);
    image_header_size = pq_ph_header.image_header_size;
    
    file_info->meas_mode = pq_ph_header.meas_mode;
    if (file_info->meas_mode == PQ_T2_MODE) {
      file_info->resolution = 4;
    }
    else if (file_info->meas_mode == PQ_T3_MODE) {
      // Cast from float->double
      double res_d = (double) pq_ph_header.board_info.resolution;
      // Multiply by 1000 to go from ns->ps
      file_info->resolution = (ttd_t)round(res_d * 1e3);
    }
    file_info->sync_rate = pq_ph_header.chan0_inp_rate;
    file_info->num_records = (size_t)pq_ph_header.num_records;
    file_info->num_channels = 2;
    fseek(fp, image_header_size*sizeof(pq_image_header_record), SEEK_CUR);
  }
  if (file_info->sync_rate > 0) {
    file_info->sync_period = (ttd_t) round(1e12 / ((double) file_info->sync_rate));
  }
  else {
    file_info->sync_period = 0;
  }

 exit_block:
  return(retcode);
}

void pq_print_file_info(pq_fileinfo_t *file_info) {
  setlocale(LC_NUMERIC, "");
  printf(KHEAD1 "\nInput File Information\n" KNRM);
  if (file_info->instrument == PQ_HH) {
    printf(KHEAD2 "Instrument: "  "HydraHarp\n");
  }
  else if (file_info->instrument == PQ_PH) {
    printf(KHEAD2 "Instrument: "  "PicoHarp\n");
  }
  else {
    printf(KHEAD2 "Unrecognized Instrument!\n");
  }
  if (file_info->meas_mode == PQ_T2_MODE) {
    printf(KHEAD2 "Measurement Mode: " "T2\n");
  }
  else if (file_info->meas_mode == PQ_T3_MODE) {
    printf(KHEAD2 "Measurement Mode: "  "T3\n");
  }
  else {
    printf(KHEAD2 "Unrecgonized Measurement Mode!\n");
  }
  printf(KHEAD2"File Format Version: " "%d\n", file_info->fmt_version);
  printf(KHEAD2"Number of records: " KNUMBER "%lu\n", file_info->num_records);
  printf(KHEAD2"Timing Resolution: " KTIME "%" PRIu64 " ps\n", file_info->resolution);
  printf(KHEAD2"Sync Period: " KTIME "%" PRIu64 " ps\n", file_info->sync_period);
  printf(KHEAD2"Sync Rate: " KRATE "%d Hz\n", file_info->sync_rate);
  printf(KNRM "\n");
}

  
  
