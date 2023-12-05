static void bitboard_set(uint64_t& bitboard, board_pos pos) {
	if(!pos.is_valid())
		throw std::runtime_error("bitboard_set: Invalid position");
	bitboard |= 1ULL << (pos.y * 8 + pos.x);
}

static bool bitboard_get(uint64_t bitboard, board_pos pos) {
	return bitboard & (1ULL << (pos.y * 8 + pos.x));
}