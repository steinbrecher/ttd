//
// Created by Greg Steinbrecher on 3/4/17.
//

#ifndef TTD_H5_HIST_H
#define TTD_H5_HIST_H

int write_uint64_dataset(char* filename, char* dataset, size_t* dims, size_t ndims, uint64_t* data);
int write_int64_dataset(char* filename, char* dataset, size_t* dims, size_t ndims, int64_t* data);

#endif //TTD_H5_HIST_H
