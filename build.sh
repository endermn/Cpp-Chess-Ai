set -e
clang++ -std=c++2b -fsanitize=undefined -g ./src/*.cpp -o bin/main -lSDL2 -lSDL2_image
./bin/main
