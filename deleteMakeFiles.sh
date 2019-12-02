#Author: Teodor Fratiloiu teodor.fratiloiu@tum.de
# This script will delete all the CMake-related files to allow for a new, clean execution of cmake.
#If it doesnt work on the first try, run chmod +x deleteMakeFiles.sh in your terminal.

rm Asteroids.elf
rm Makefile
rm Asteroids.lst
rm Asteroids.bin
rm Asteroids.hex
rm cmake_install.cmake
rm CMakeCache.txt
rm -r CMakeFiles
