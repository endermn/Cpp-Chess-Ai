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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>



using std::optional, std::array;

const int SQUARE_SIZE = 75;

SDL_Texture* pieces_image;


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
struct piece {
	piece_color color;
	piece_type type;

	friend bool operator==(piece, piece) = default;
};
struct move {
	board_pos dst_pos;
	board_pos src_pos;
	bool is_capture;
	float score = 0;
};

namespace piece_offsets {
	board_pos BISHOP[4] = {{-1,-1}, {-1,1}, {1,1}, {1, -1}};
	board_pos KNIGHT[8] = { {-2, 1}, {2, 1}, {-2, -1}, {2, -1}, {1, 2}, {1, -2}, {-1, -2}, {-1, 2} };
	board_pos ROOK[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	board_pos KING_QUEEN[8] = { {1,1}, {-1, 1}, {-1, -1}, {1, -1}, {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
};

static constexpr std::array<piece_type, 8> start_row = {
		piece_type::ROOK, piece_type::KNIGHT, piece_type::BISHOP, piece_type::QUEEN,
		piece_type::KING, piece_type::BISHOP, piece_type::KNIGHT, piece_type::ROOK,
};

void bitboard_set(uint64_t& bitboard, board_pos pos) {
	if(pos.is_valid())
		bitboard |= 1ULL << (pos.y * 8 + pos.x);
}

bool bitboard_get(uint64_t bitboard, board_pos pos) {
	return bitboard & (1ULL << (pos.y * 8 + pos.x));
}


int get_color_value(piece_color color) {
    return color == piece_color::BLACK ? -1 : 1;
}


std::random_device rd;
std::mt19937_64 e2(rd());
std::uniform_int_distribution<uint64_t> dist;

