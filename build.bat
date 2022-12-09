@ECHO OFF
cd target
clang++ -c -Wall -O3 ../src/*.cpp
llvm-ar rc string.a *.o
del *.o
cd ..