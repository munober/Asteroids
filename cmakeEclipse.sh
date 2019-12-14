#Author: Teodor Fratiloiu
# This script will run an ESPL specific kind of cmake, to make this code work with the Eclipse dev environment.
#If it doesnt work on the first try, run chmod +x cmakeEclipse.sh in your terminal.

cmake -G"Eclipse CDT4 - Unix Makefiles" ./
