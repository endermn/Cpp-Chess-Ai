class PieceMoves {
public:
	array<array<optional<piece>, 8>, 8> board;
	optional<int8_t> en_passant;
	piece_color turn = piece_color::BLACK;
	array<array<bool, 2>, 2> can_castle = { { { {false, false} } , { {false, false} } } };  // FIRST INDEX = COLOR, 2ND INDEX = SIDE 

private:
	uint64_t king_moves(board_pos src_pos) const {
		uint64_t bitboard = 0;
		piece_color src_color = board[src_pos.y][src_pos.x].value().color;
		for (board_pos i : piece_offsets::KING_QUEEN) {
			board_pos dst = { i.x + src_pos.x , i.y + src_pos.y };
			if (dst.is_valid() && (!board[dst.y][dst.x].has_value() || board[dst.y][dst.x].value().color != src_color))
				bitboard_set(bitboard, dst);
		}
		if (can_castle[int(src_color)][int(castle_side::SHORT)]) {
			board_pos dst_castle = { 6, src_pos.y };
			if (board[dst_castle.y][7] == piece{.color = src_color, .type = piece_type::ROOK } &&
				!board[src_pos.y][5].has_value() &&
				!board[src_pos.y][6].has_value())
				bitboard_set(bitboard, dst_castle);
		}
		if (can_castle[int(src_color)][int(castle_side::LONG)]) {
			board_pos dst_castle = { 2, src_pos.y };
			if (board[dst_castle.y][0] == piece{ .color = src_color, .type = piece_type::ROOK } &&
				!board[src_pos.y][1].has_value() &&
				!board[src_pos.y][2].has_value() &&
				!board[src_pos.y][3].has_value())
				bitboard_set(bitboard, dst_castle);
		}
		return bitboard;
	}

	uint64_t bishop_moves(board_pos src_pos) const {
		uint64_t bitboard = 0;
		for (board_pos i : piece_offsets::BISHOP)
			for (int j = 1; j < 8; j++) {
				board_pos diagonal = { src_pos.x + j * i.x , src_pos.y + j * i.y };
				if (!diagonal.is_valid()) 
					break;
				if (board[diagonal.y][diagonal.x].has_value()) {
					if (board[diagonal.y][diagonal.x].value().color != board[src_pos.y][src_pos.x].value().color)
						bitboard_set(bitboard, diagonal);
					break;
				}
				bitboard_set(bitboard, diagonal);
			}
		return bitboard;
	}

	uint64_t knight_moves(board_pos src_pos) const {
		uint64_t bitboard = 0;
		piece_color src_color = board[src_pos.y][src_pos.x].value().color;
		for (board_pos i : piece_offsets::KNIGHT) {
			board_pos dst = { i.x + src_pos.x , i.y + src_pos.y };
			if (dst.is_valid() && (!board[dst.y][dst.x].has_value() || board[dst.y][dst.x].value().color != src_color))
				bitboard_set(bitboard, dst);
		}
		return bitboard;
	}

	uint64_t rook_moves(board_pos src_pos) const {
		uint64_t bitboard = 0;
		for (board_pos i : piece_offsets::ROOK)
			for (int j = 1; j < 8; j++) {
				board_pos diagonal = { src_pos.x + j * i.x , src_pos.y + j * i.y };
				if (!diagonal.is_valid()) break;
				if (board[diagonal.y][diagonal.x].has_value()) {
					if (board[diagonal.y][diagonal.x].value().color != board[src_pos.y][src_pos.x].value().color)
						bitboard_set(bitboard, diagonal);
					break;
				}
				bitboard_set(bitboard, diagonal);
			}
		return bitboard;
	}

	uint64_t queen_moves(board_pos src_pos) const {
		return bishop_moves(src_pos) | rook_moves(src_pos);
	}

	uint64_t pawn_moves(board_pos src_pos) const {
		uint64_t bitboard = 0;

		piece_color color = board[src_pos.y][src_pos.x].value().color;

		int next_y = src_pos.y - 1 + 2 * int(color);
		int double_move_y = color == piece_color::WHITE ? 4 : 3;

		if (src_pos.x < 7){
			if (optional<piece> dst = board[next_y][src_pos.x + 1];
			dst.has_value() && dst.value().color != color) {
				bitboard_set(bitboard, { src_pos.x + 1, next_y});
			} else {
				if (src_pos.x + 1 == en_passant &&
					next_y == (color == piece_color::WHITE ? 2 : 5) &&
					board[src_pos.y][src_pos.x + 1].has_value() &&
					board[src_pos.y][src_pos.x + 1]->type == piece_type::PAWN &&
					board[src_pos.y][src_pos.x + 1]->color != board[src_pos.y][src_pos.x]->color
					)
						bitboard_set(bitboard, { src_pos.x + 1, next_y });
			}
		}
		if (src_pos.x > 0) {
			if (optional<piece> dst = board[next_y][src_pos.x - 1];
			dst.has_value() && dst.value().color != color)
				bitboard_set(bitboard, { src_pos.x - 1, next_y});
			else 
				if(src_pos.x - 1 == en_passant &&
				next_y == (color == piece_color::WHITE ? 2 : 5) &&
				board[src_pos.y][src_pos.x - 1].has_value() &&
				board[src_pos.y][src_pos.x - 1]->type == piece_type::PAWN &&
				board[src_pos.y][src_pos.x - 1]->color != board[src_pos.y][src_pos.x]->color
					)
					bitboard_set(bitboard, {src_pos.x - 1, next_y});
		}
		if (!board[next_y][src_pos.x].has_value()) {
			if (!board[double_move_y][src_pos.x].has_value())
				if (src_pos.y == (color == piece_color::WHITE ? 6 : 1))
					bitboard_set(bitboard, { src_pos.x , double_move_y });
			bitboard_set(bitboard, { src_pos.x, next_y });
		}

		return bitboard;
	}

public:
	uint64_t get_moves(board_pos src_pos) const {
		switch (board[src_pos.y][src_pos.x]->type) {
		case piece_type::KING:
			return king_moves(src_pos);
		case piece_type::QUEEN:
			return queen_moves(src_pos);
		case piece_type::BISHOP:
			return bishop_moves(src_pos);
		case piece_type::KNIGHT:
			return knight_moves(src_pos);
		case piece_type::ROOK:
			return rook_moves(src_pos);
		case piece_type::PAWN:
			return pawn_moves(src_pos);
		default:
			std::unreachable();
		}
	}

	void make_move(board_pos target, board_pos src_pos, SDL_Window* win) {
		piece src_piece = board[src_pos.y][src_pos.x].value();

		int8_t compare_color = src_piece.color == piece_color::WHITE ? 7 : 0;

		int double_move_y = src_piece.color == piece_color::WHITE ? 4 : 3;
		int next_y = src_pos.y - 1 + 2 * int(src_piece.color);
		en_passant = std::nullopt;

		if (src_piece.type == piece_type::PAWN) {
			if (target.y == 0 || target.y == 7) {
				if (win != nullptr)
					src_piece.type = promote_message(win);
				else
					src_piece.type = piece_type::QUEEN;
			}
			if (target == board_pos{ src_pos.x, double_move_y })
				en_passant = src_pos.x;

			if (target == board_pos{ src_pos.x + 1, next_y } && !board[target.y][target.x].has_value()) {
				board[src_pos.y][src_pos.x + 1] = std::nullopt;
				en_passant = std::nullopt;
			}
			if(target == board_pos{ src_pos.x - 1, next_y } && !board[target.y][target.x].has_value()) {
				board[src_pos.y][src_pos.x - 1] = std::nullopt;
				en_passant = std::nullopt;
			}
		}
		const std::array<board_pos, 4> castleTargets = {
			{ {7, 7}, {0, 7}, {7, 0}, {0, 0} }
		};

		const std::array<std::array<int, 2>, 4> castleIndices = {
			{ 
				{int(piece_color::WHITE), int(castle_side::SHORT)},
				{int(piece_color::WHITE), int(castle_side::LONG)},
				{int(piece_color::BLACK), int(castle_side::SHORT)},
				{int(piece_color::BLACK), int(castle_side::LONG)} 
			}
		};

		for (int i = 0; i < castleIndices.size(); i++) {
			if (target == castleTargets[i]) {
				can_castle[castleIndices[i][0]][castleIndices[i][1]] = false;
				break;
			}
		}
			
		if (src_piece.type == piece_type::KING) {
			if (src_pos == board_pos{ 4, compare_color } &&
				target == board_pos{ 6, compare_color }) {
				board[target.y][5] = board[target.y][7];
				board[target.y][7] = std::nullopt;
			}
			if (src_pos == board_pos{ 4, compare_color } &&
				target == board_pos{ 2, compare_color }) {
				board[target.y][3] = board[target.y][0];
				board[target.y][0] = std::nullopt;
			}

			can_castle[int(src_piece.color)] = { false, false };
		}
		board[target.y][target.x] = src_piece;
		board[src_pos.y][src_pos.x] = std::nullopt;
		turn = piece_color(!bool(turn));
	}
};