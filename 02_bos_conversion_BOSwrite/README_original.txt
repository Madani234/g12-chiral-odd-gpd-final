This software will compile the program RootToBOS_CARBON.cc into an exeutable RootToBOS_CARBON that will
take an input root file (usually of the format NTuple) and convert it into a BOS file that can be read
by the gsim software.
Source the following first:
source ../.cshrc /
****In case the above does not work, source the following environment:****
~> source /home/clasg13/env_svn_centos62_64bit
To Compile the software run the command:
~> scons -j8
For HELP run the following command:
~> ./build/bin/RootToBOS_CARBON -h
To run the software, use a command like the following, where "inputrootfile" is the root file you wish to convert:
~> ./build/bin/RootToBOS_CARBON inputrootfile
Check number of events by running the following command on the output file that you have created in the BOS format:
~> countbos renamedfile.bos
