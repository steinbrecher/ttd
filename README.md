#Time Tagged Data Processing#
=========

This is a collection of programs intended for parsing PicoQuant data files, converting
them to a format that's easier to work with, and doing some processing on them.

Specifically, these programs are written for speed and are intended for parsing
large numbers of records very quickly. As such, they are a little unstable 
(i.e. prone to segfaults when things don't go as expected) and are somewhat
bare-bones. At some point in the future, I'm planning to wrap these in MATLAB
and Python scripts to make the user experience a little nicer, but for now
that's in the future. 

## Basic installation instructions ##

1. Unzip the .tar.gz into its own folder: 

	`% tar -xvzf picoquant.tar.gz`	

	`% cd picoquant`

2. Install the software. Commands are:

	`% ./configure`
	
	`% make`
	
	`% sudo make install`

	I usually break it out like this in case there are errors, but you can do it in 
	one line also:

	`% ./configure && make && sudo make install`

	The sudo command will ask you for your password; totally normal.

## G2 Instructions ##

To run a g2 (really, a cross correlation since they're not normalized):

1. Convert your file from .pt2 (or .pt3, .ht2, or .ht3) to .ttd files:

	`% pq-ttd -i [your file] -o [prefix for per-channel files]`

	Note that this breaks out a single file in to one 'ttd' file per channel of the 
	source data.

	'ttd' stands for 'Time Tagged Data' and is the local format the software convert
	to; it's just a binary file where each 64-bit block is an unsigned integer 
	corresponding to a photon record. This makes the data processing much nicer by
	decoupling the parsing of PicoQuant's convoluted file format from the correlation
	calculations. 

2. Run the cross correlation:

	`% ttd-g2 -i [channel 1 ttd file] -I [channel 2 ttd file] -o [output csv] 
	   -b [bin time] -w [corelation window time]`

	All times are in picoseconds.

3. Process the data. The csv file from the previous point has two columns; the first
	corresponds to the bin time, the second corresponds to the number of counts in that
	bin. I prefer MATLAB for quick plotitng: 

`>> data = csvread('crosscorr.csv'); figure; plot(data(:,1), data(:,2);`

## Other Notes ##

Included in this package are a few other tools that might be of use:

- ttd-merge combines two ttd files in to a single set of arrrival times (still in 
order). This is more useful when you have HydraHarp data with more than 2 channels.

- ttd-dump prints the times of each record in the file, one per line. This isn't 
recommended except for debugging. e.g.

	`% ttd-dump blah.ttd | head -n 50`

	will let you quickly check the first 50 records from the file. 

- ttd-delay allows for a permanent time offset for one of the channels.

- ttd-doqkd is for doqkd data; unless you know what this is you probably don't need 
to touch it

