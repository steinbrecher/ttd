//
// Created by Greg Steinbrecher on 3/4/17.
//

#include "h5_interface.h"
#include "hdf5.h"

#include <stdio.h>
#include <stdlib.h>

#define DIM0 4
#define DIM1 7
#define NDIMS 2

int write_uint64_dataset(char* filename, char* dataset, size_t* dims, size_t ndims, uint64_t* data) {
  hid_t file, space, dset;
  herr_t status;
  hsize_t hdims[ndims];
  size_t i;
  for(i=0; i<ndims; i++) {
    hdims[i] = dims[i];
  }

  file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  space = H5Screate_simple(ndims, hdims, NULL);
  dset = H5Dcreate(file, dataset, H5T_STD_U64LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(dset, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  status = H5Dclose(dset);
  status = H5Sclose(space);
  status = H5Fclose(file);

  return(0);
}


int write_int64_dataset(char* filename, char* dataset, size_t* dims, size_t ndims, int64_t* data) {
  hid_t file, space, dset;
  herr_t status;
  hsize_t hdims[ndims];
  size_t i;
  for(i=0; i<ndims; i++) {
    hdims[i] = dims[i];
  }

  file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  space = H5Screate_simple(ndims, hdims, NULL);
  dset = H5Dcreate(file, dataset, H5T_STD_I64LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(dset, H5T_NATIVE_INT64, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  status = H5Dclose(dset);
  status = H5Sclose(space);
  status = H5Fclose(file);

  return(0);
}

int main(void) {
  size_t dims[NDIMS] = {DIM0, DIM1};
  uint64_t wdata[DIM0*DIM1];
  size_t i,j;

  for (i=0; i<DIM0; i++) {
    for (j=0; j<DIM1; j++) {
      wdata[i*DIM1 + j] = i*j-j+DIM0+DIM1;
    }
  }
  char filename[] = "testhist.h5";
  char dset_name[] = "0-1";
  write_uint64_dataset(filename, dset_name, dims, NDIMS, wdata);
}

