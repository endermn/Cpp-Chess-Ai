

class Bitboard {
public:
	uint64_t bitboard = 0;

	void bitboard_set(board_pos pos) {
		if(!pos.is_valid())
			throw std::runtime_error("bitboard_set: Invalid position");
		bitboard |= 1ULL << (pos.y * 8 + pos.x);
	}

	bool bitboard_get(board_pos pos) {
		return bitboard & (1ULL << (pos.y * 8 + pos.x));
	}
};
