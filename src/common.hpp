#include <array>
#include <optional>
#include <vector>
#include <math.h>
#include <limits.h>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <bit>
#include <chrono>
#include <utility>
#include <functional>
#include <span>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <random>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>



using std::optional, std::array;
using namespace std::chrono;

const int SQUARE_SIZE = 75;

enum class piece_color : bool {
	WHITE,
	BLACK,
};

enum class castle_side : bool {
	SHORT,
	LONG,
};

enum class game_phase : int8_t {
	OPENING,
	MIDGAME,
	ENDGAME,
};

enum class piece_type : uint8_t {
	KING,
	QUEEN,
	BISHOP,
	KNIGHT,
	ROOK,
	PAWN,
};

struct zobrist_value {
	float evaluation;
	int depth;
};

struct board_pos {
	int x;
	int y;
	bool is_valid() {
		return x >= 0 && x < 8 && y >= 0 && y < 8;
	}
	friend bool operator==(board_pos, board_pos) = default;
};

static void messagebox_error(std::string outside_text, std::string inside_text) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, outside_text.data(), inside_text.data(), nullptr);
}



static std::random_device rd;
static std::mt19937_64 e2(rd());
static std::uniform_int_distribution<uint64_t> dist;

