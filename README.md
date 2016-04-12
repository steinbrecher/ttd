#Time Tagged Data Processor#
=========

This is a collection of programs intended for working with time-tagged photon data, primarily deriving from PicoQuant photon counters.

The tools are split into three main categories:

1. Libraries for interacting with the PicoQuant data formats (such as `.ht2`, `.ptu`, etc.) and translating them to other formats that may be easier to work with (such as the native format of this package, `.ttd` files)

2. Programs and libraries for working with a more simple file format (`.ttd` files)

3. Programs for fast computation of cross-correlation functions (g(2), g(3), and g(4) for now) of photon arrival time statistics, which can operate on `.ttd` files or directly on picoquant files

There are variants of the programs in the third category included for working with either `.ttd` files or directly with data files in PicoQuant formats. 

## Basic installation instructions ##

1. Download the latest version: [ttd.tar.gz](ttd.tar.gz)

2. Unzip the .tar.gz into its own folder: 

        $ tar -xvzf ttd.tar.gz
        $ cd ttd

3. Install the software. Commands are:

		$ mkdir build
		$ cd build
		$ cmake ..
		$ make
		$ sudo make install

Depending on your system 'sudo' may be optional in the last command.

# Summary of Executable Tools #
=========

(Note: for help with any of the installed tools, run them with the flag --help)

## Working with PicoQuant-formatted data files
* `pq-gn`: Computes g(2), g(3), g(4) cross-correlations between the channels of a picoquant TTTR file (.ht2, .ht3, etc.)
* `pq-dump`: Prints photon arrival time and channel information to stdout. Super inefficient but useful for debugging

## Converting to the Time Tagged Data format (.ttd)
* `pq-ttd`: Takes a PicoQuant-formatted data file as input, produces .ttd files (one per channel) as output. 

## Working with Time Tagged Data Files
* `ttd-shift`: Move all the time records in a ttd file forward or back in time (useful for aligning channels)
* `ttd-merge`: Combine two ttd files into a single one for further manipulation / computation
* `ttd-dump`: Prints the records in a ttd file to stdout. Super inefficient, but useful for debugging.
* `ttd-g2`, `ttd-g3`, `ttd-g4`: Cross-correlation computation programs for `ttd` files

# Cross-Correlation Output Format #
=========
Currently, all of the cross-correlation programs output CSV files. For g(n) calculations, the first `n-1` columns are the times of the bin centers and the final column is the number of correlation counts in that bin. For example, for a g(3) run on channels 0,1, and 3, the first column is (t1 - t0), the second column is (t3 - t0), and the third column is the number of counts in each bin. 

# Summary of Libraries Included #
=========
In addition to the executable programs discussed above, there are a bunch of fast C libraries included with this package. In general, these APIs will be more stable so that if, e.g. the `.ttd` data format changes or new PicoQuant file formats are introduced, code using these libraries won't have to be updated (or, at the least, impact will be minimized...). 

See the `README` files in the `src/` sub-folders for more detailed information; doxygen-produced documentation for all of these should be completed soon.

Broken down by directory structure (taking `src/` to be the root):

* `pq-lib`
  * `pq_filebuffer`: Provides API for getting photon arrival times (along with channel) in-order from PicoQuant-formatted data files. Allows channels to be shifted in time (returning arrivals in the new ordering) as well as automatically ignoring certain channels (to make calling code cleaner when not working with all channels). 
  * `pq_parse`: File format translation for PicoQuant-formatted files. 
* `ttd`
  * `ttd`: Library-level reference for `ttd` format
* `ttd-lib`
  * `ttd_filebuffer`: Provides API for getting photon arrival times from `.ttd` files
  * `ttd_crosscorrN`: Data structures and API for efficient computation of cross-correlations
  * `ttd_ringbuffer`: Ring-buffer structure and API for the `ttd_t` data type 

