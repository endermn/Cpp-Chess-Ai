#include "PieceMoves.hpp"
TranspositionTable transposition_table;

class PieceChecks : public PieceMoves {
public:
	static float get_piece_value(piece_type type) {
		float piece_values[6] = {
			200,
			9.5,
			3.33,
			3.05,
			5.63,
			1,
		};
		return piece_values[int(type)];
	}

	game_phase get_game_phase() const {
		int total_major_pieces = 0;

		for (int y = 0; y < board.size(); y++) 
			for (int x = 0; x < board[y].size(); x++)
				if (board[y][x].has_value() && board[y][x]->type != piece_type::PAWN && board[y][x]->type != piece_type::KING)
					total_major_pieces++;

		if (total_major_pieces <= 4)
			return game_phase::ENDGAME;
		else if (total_major_pieces <= 10)
			return game_phase::MIDGAME;
		else
			return game_phase::OPENING;
	}
		std::array<int, 2> targetted_squares_count() const{
		std::array<int, 2> mobility_score = {};

		for (int i = 0; i < 64; i++) {
			int y = i / 8;
			int x = i % 8;
			if (!board[y][x].has_value())
				continue;
			mobility_score[int(board[y][x].value().color)] += std::popcount(get_moves({x, y}).bits);
		}
		return mobility_score;
	}

	uint64_t hash() const {
		uint64_t current_hash = turn == piece_color::WHITE ? 2103981289031988 : 0;
		for (int i = 0; i < 64; i++) {
			int y = i / 8;
			int x = i % 8;
			if(!board[y][x].has_value())
				continue;
			int hash_index = transposition_table.piece_to_hash(board[y][x]->color, board[y][x]->type);
			current_hash ^= transposition_table.piece_zobrist[y][x][hash_index];
		}

		if(en_passant.has_value())
			current_hash ^= transposition_table.en_passant_zobrist[en_passant.value()];

		for (int i = 0; i < 4; i++) {
			if(can_castle[i / 2][i % 2])
				current_hash ^= transposition_table.castle_zobrist[i / 2][i % 2];
		}

		return current_hash;
	}

	bool is_under_check(piece_color color) {
		Bitboard targetted_squares;
		optional<board_pos> king_pos = std::nullopt;
		for (int i = 0; i < 64; i++) {
			int y = i / 8;
			int x = i % 8;

			if (!board[y][x].has_value())
				continue;

			if (board[y][x]->type == piece_type::KING && board[y][x]->color == piece_color::BLACK)
				king_pos = board_pos{ x, y };

			if (board[y][x].value().color != color)
				targetted_squares.bits |= get_moves({ x, y }).bits;
		}

		if(!king_pos.has_value())
			return false;
		return targetted_squares.get(king_pos.value());
	}
};
