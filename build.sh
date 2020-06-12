g++ -c "Commander.cpp" -m64
g++ -static-libstdc++ -static-libgcc -shared -o "Commander.dll" Commander.o "-Wl,--out-implib,Commander.a"