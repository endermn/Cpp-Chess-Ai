class TranspositionTable {
public:
	std::array<std::array<std::array<uint64_t, 12>, 8>, 8> piece_zobrist;
	std::array<std::array<uint64_t, 2>, 2> castle_zobrist;
	std::array<uint64_t, 8> en_passant_zobrist;
	std::unordered_map<uint64_t, zobrist_value> table;
private:
	void init_pieces() {
		for(int y = 0; y < 8; y++) 
			for(int x = 0; x < 8; x++) 
				for(int z = 0; z < 12; z++) 
					piece_zobrist[y][x][z] = dist(e2);
	}
	
	void init_castles() {
		for(int y = 0; y < 2; y++)
			for(int x = 0; x < 2; x++)
				castle_zobrist[y][x] = dist(e2);
	}

	void init_en_passants() {
		for(int y = 0; y < 8; y++)
			en_passant_zobrist[y] = dist(e2);
	}
public:
	TranspositionTable() {
		init_pieces();
		init_castles();
		init_en_passants();
	}

	int piece_to_hash (piece_color color, piece_type type) {
		return int(type) + int(color) * 6;
	}
};

TranspositionTable transposition_table;