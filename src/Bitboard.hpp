

class Bitboard {
public:
	uint64_t bits = 0;

	void set(board_pos pos) {
		if(!pos.is_valid())
			throw std::runtime_error("bits_set: Invalid position");
		bits |= 1ULL << (pos.y * 8 + pos.x);
	}

	bool get(board_pos pos) {
		return bits & (1ULL << (pos.y * 8 + pos.x));
	}
	friend Bitboard operator| (Bitboard a, Bitboard b) {
		return Bitboard{a.bits | b.bits};
	}
};
