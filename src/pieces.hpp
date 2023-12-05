struct piece {
	piece_color color;
	piece_type type;

	friend bool operator==(piece, piece) = default;
};
struct Move {
	board_pos dst_pos;
	board_pos src_pos;
	bool is_capture;
	float score = 0;
};

namespace piece_offsets {
	static constexpr board_pos BISHOP[4] = {{-1,-1}, {-1,1}, {1,1}, {1, -1}};
	static constexpr board_pos KNIGHT[8] = { {-2, 1}, {2, 1}, {-2, -1}, {2, -1}, {1, 2}, {1, -2}, {-1, -2}, {-1, 2} };
	static constexpr board_pos ROOK[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	static constexpr board_pos KING_QUEEN[8] = { {1,1}, {-1, 1}, {-1, -1}, {1, -1}, {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
};

static int get_color_value(piece_color color) {
    return color == piece_color::BLACK ? -1 : 1;
}
