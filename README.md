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

		% ./configure
		% make
		% sudo make install

	I usually break it out like this in case there are errors, but you can do it in 
	one line also:

		% ./configure && make && sudo make install

	The sudo command will ask you for your password; totally normal.

## G2 Instructions ##

To run a g2 (really, a cross correlation since they're not normalized):

1. Convert your file from .pt2 (or .pt3, .ht2, or .ht3) to .ttd files:

		% pq-ttd -i [your file] -o [prefix for per-channel files]

	Note that this breaks out a single file in to one 'ttd' file per channel of the 
	source data.

	'ttd' stands for 'Time Tagged Data' and is the local format the software convert
	to; it's just a binary file where each 64-bit block is an unsigned integer 
	corresponding to a photon record. This makes the data processing much nicer by
	decoupling the parsing of PicoQuant's convoluted file format from the correlation
	calculations. 

2. Run the cross correlation:

		% ttd-g2 -b [bin time] -w [corelation window time] -1 [channel 1 ttd file] -2 [channel 2 ttd file] -o [output csv]

	All times are in picoseconds.

3. Process the data. The csv file from the previous point has two columns; the first
	corresponds to the bin time, the second corresponds to the number of counts in that
	bin. I prefer MATLAB for quick plotitng: 

		>> data = csvread('crosscorr.csv');
		>> figure;
		>> plot(data(:,1), data(:,2);
		
#### Output Format
The output file is a csv file with the first column corresponding to the centers of the time bins (in picoseconds) and the second column corresponding to the number of counts in that bin. The times are [arrival on channel 2] - [arrival on channel 1]
so photons in the file supplied with the '-2' flag that arrive after those in the '-1' file will have a positive time correlation.
		
## G3 Notes ##

Running a g3 or g4 correlation is effectively the same as running a g2. 
You just have to modify the command to be ttd-g3 or ttd-g4 and give it the extra files:

	% ttd-g3 -b [bin time] -w [corelation window time] -1 [c1.ttd] -2 [c2.ttd] -3 [c3.ttd] -o [output csv]
	
or

	% ttd-g4 -b [bin time] -w [corelation window time] -1 [c1.ttd] -2 [c2.ttd] -3 [c3.ttd] -4 [c4.ttd] -o [output csv]
	
In case you forget these incantations, just run the program with the flag '--help' to output documentation. For example:

	% ttd-g3 --help
	Usage: ttd-g3 [-1 in1.ttd] [-2 in2.ttd] [-3 in3.ttd] [-o output_file]
              [-b bin_time] [-w window_time]
	Notes:
		-b (--bin-time):	Specified in picoseconds
		-w (--window-time):	Window time in picoseconds
	Other options:
		-v (--verbose):		Enable verbose output to stdout
		-h (--help):		Print this help dialog
		-V (--version):		Print program version
		
#### Output Format
Unlike the g2, which is one dimensional data, the g3 needs both dimensions of the csv file.
As such, it outputs a seperate file with '-times' appended before the '.csv' of the filename. 
That is, if you pass the flag '-o crosscorr.csv' it will create both crosscorr.csv and crosscorr-times.csv. 
The latter will be have the center time of each bin, in order. 

The csv file itself is written such that lines correspond to tau1 and columns correspond to tau2. That is, 
the times on line 1 all have the same offset from the events in file passed with '-2' to the file passed with '-1'.
		
## G4 Notes
As mentioned above, the running a g4 is basically the same as a g3 or g2. The only important difference
is that, because CSV files can only hold two-dimensional data, it outputs one CSV file *per time bin*, so
you need to work with lots of files, unfortunately. Future versions will probably take advantage of the HDF5 file
format to store multi-dimensional data. 
		
#### Output Format
When running a g4 autocorrelation, it outputs one CSV file per slice. I find it best to make a folder called 'csvs'
or equivalent and pass in the output name as '-o csvs/[filename.csv]'. It will then create csvs/filename-0.csv through
csvs/filename-[num_bins-1].csv in that folder, along with [filename-times.csv], as before. 
Each file corresponds to a constant tau3, with the individual files being in the same format of the g3; rows are tau1, 
columns are tau2.

## Correcting for Time Offsets ##
It's best if the offsets between the different channels are corrected for in the PicoQuant software ahead of time;
it's kind of a pain to correct the data afterword. However, since it's often necessary, I've included a program that
makes this somewhat easier. The program ttd-delay takes in a ttd file and writes a new one, with all the timestamps
shifted by some number of picoseconds. For example,

	% ttd-delay -i chan1.ttd -o chan1-del.ttd -T 512
	
would produce a file with all records shifted forwards in time by 512ps. Unfortunately, only positive time delays are supported
right now, as the file format doesn't support negative times. A future version will allow for negative shifts that 
maintain the postivity of all records, but that hasn't been done yet. 

I've found the best way to figure out which offsets to apply is to run a g2 between the channels (assuming you have correlated inputs) and detect the difference that way. 

## Other Notes ##

Included in this package are a few other tools that might be of use:

- ttd-merge combines two ttd files in to a single set of arrrival times (still in 
order). This is more useful when you have HydraHarp data with more than 2 channels.

- ttd-dump prints the times of each record in the file, one per line. This isn't 
recommended except for debugging. e.g.

		% ttd-dump blah.ttd | head -n 50

	will let you quickly check the first 50 records from the file. 

