#include "Position.hpp"


Position fen_to_position(std::string_view fen) {
	std::unordered_map<char, piece_type> umap;
	umap['r'] = piece_type::ROOK;
	umap['n'] = piece_type::KNIGHT;
	umap['b'] = piece_type::BISHOP;
	umap['q'] = piece_type::QUEEN;
	umap['k'] = piece_type::KING;
	umap['p'] = piece_type::PAWN;

	Position position;
	bool after_board = false;

	int x = 0, y = 0;
	for (char c : fen) {
		if (c == '/') {
			if(y == 7){
				messagebox_error("INVALID FEN", "Failed to load position from given fen");
				exit(1);
			}
			y++;
			x = 0;
		}
		else if (c == ' ') {
			after_board = true;
		}
		else if (c == fen[fen.length() - 6]) {
			position.en_passant = c - 'a';
		}
		else if (after_board) {
			switch (c) {
			case 'k':
				position.can_castle[int(piece_color::BLACK)][int(castle_side::SHORT)] = true;
				break;
			case 'q':
				position.can_castle[int(piece_color::BLACK)][int(castle_side::LONG)] = true;
				break;
			case 'K':
				position.can_castle[int(piece_color::WHITE)][int(castle_side::SHORT)] = true;
				break;
			case 'Q':
				position.can_castle[int(piece_color::WHITE)][int(castle_side::LONG)] = true;
				break;
			case 'w':
				position.turn = piece_color::WHITE;
				break;
			default:
				break;
			}
		} else if (isdigit(c)) {
			if (c - '0' > 8 || c - '0' < 1) {
				messagebox_error("INVALID FEN", "Invalid fen!");
				exit(1);
			}
			for (int i = 0; i < c - '0'; i++)
				position.board[y][x++] = {};
		} else {
			position.board[y][x++] = piece{ isupper(c) ? piece_color::WHITE : piece_color::BLACK, umap[tolower(c)] };
		}
	}

	return position;
}
