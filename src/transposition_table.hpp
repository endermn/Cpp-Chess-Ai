struct zobrist_value {
	float evaluation;
	int depth;
};

static std::random_device rd;
static std::mt19937_64 e2(rd());
static std::uniform_int_distribution<uint64_t> dist;

class TranspositionTable {
public:
	std::array<std::array<std::array<uint64_t, 12>, 8>, 8> piece_zobrist;
	std::array<std::array<uint64_t, 2>, 2> castle_zobrist;
	std::array<uint64_t, 8> en_passant_zobrist;
	std::unordered_map<uint64_t, zobrist_value> table;
private:
	void set_pieces() {
		for(int y = 0; y < 8; y++) 
			for(int x = 0; x < 8; x++) 
				for(int z = 0; z < 12; z++) 
					piece_zobrist[y][x][z] = dist(e2);
	}
	
	void set_castle() {
		for(int y = 0; y < 2; y++)
			for(int x = 0; x < 2; x++)
				castle_zobrist[y][x] = dist(e2);
	}

	void set_en_passant() {
		for(int y = 0; y < 8; y++)
			en_passant_zobrist[y] = dist(e2);
	}
public:
	TranspositionTable() {
		set_pieces();
		set_castle();
		set_en_passant();
	}

	int piece_to_hash (piece_color color, piece_type type) {
		return int(type) + int(color) * 6;
	}
};