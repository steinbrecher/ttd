#Time Tagged Data Processor#
=========

This is a collection of programs intended for working with time-tagged photon data, primarily deriving from PicoQuant photon counters.

Specifically, these programs are written for speed and are intended for parsing
large numbers of records very quickly. As such, they are somewhat bare-bones.

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

# Summary of Tools #
=========

(Note: for help with any of the installed tools, run them with the flag --help)

* pq-gn: Computes g(2), g(3), g(4) cross-correlations between the channels of a picoquant TTTR file (.ht2, .ht3, etc.)

		
# Cross-Correlation Output Format #
=========
Currently, all of the cross-correlation programs output CSV files.
