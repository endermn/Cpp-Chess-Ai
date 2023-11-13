set -e
clear
clang++ -std=c++2b -fsanitize=undefined *.cpp -o main -lSDL2 -lSDL2_image
./main