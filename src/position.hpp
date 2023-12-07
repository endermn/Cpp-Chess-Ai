TranspositionTable transposition_table;

class Position : public PieceMoves {
public:
	game_phase get_game_phase() const{
		int total_major_pieces = 0;

		for (int y = 0; y < board.size(); y++) 
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value() || board[y][x]->type == piece_type::PAWN || board[y][x]->type == piece_type::KING)
					continue;
				total_major_pieces++;

			}
		if (total_major_pieces <= 4)
			return game_phase::ENDGAME;
		else if (total_major_pieces <= 10)
			return game_phase::MIDGAME;
		else
			return game_phase::OPENING;
	}

	std::array<int, 2> targetted_squares_count() const{
		std::array<int, 2> mobility_score = {};

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;
				mobility_score[int(board[y][x].value().color)] += std::popcount(get_moves({x, y}));
			}
		}
		return mobility_score;
	}

	uint64_t hash () const {
		uint64_t current_hash = turn == piece_color::WHITE ? 2103981289031988 : 0;
		for(int y = 0; y < 8; y++) {
			for(int x = 0; x < 8; x++) {
				if(!board[y][x].has_value())
					continue;
				int hash_index = transposition_table.piece_to_hash(board[y][x]->color, board[y][x]->type);
				current_hash ^= transposition_table.piece_zobrist[y][x][hash_index];

			}
		}

		if(en_passant.has_value())
			current_hash ^= transposition_table.en_passant_zobrist[en_passant.value()];

		for(int y = 0; y < 2; y++) {
			for(int x = 0; x < 2; x++){
				if(can_castle[y][x])
					current_hash ^= transposition_table.castle_zobrist[y][x];
			}
		}

		return current_hash;
	}

	float evaluate() const{
		float score = 0;

		game_phase phase = get_game_phase();

		std::array<optional<board_pos>, 2> king_positions = {std::nullopt, std::nullopt};

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;

				piece src_piece = board[y][x].value();
				int color_value = get_color_value(src_piece.color);
				switch(src_piece.type) {
				case piece_type::KING:
					for (int i = -1; i < 2; i++) {
						board_pos y_pos = {.x = x + i, .y = y - color_value};
						if(!y_pos.is_valid())
							break;
						if (board[y_pos.y][y_pos.x].has_value() && board[y_pos.y][y_pos.x]->color != board[y][x]->color)
							score += color_value * 0.1;
						if (!board[y_pos.y][x].has_value())
							score -= color_value * 0.2;
					}
					king_positions[src_piece.color == piece_color::BLACK ? 1 : 0] = board_pos{ x, y };
					break;
				case piece_type::PAWN:
					if(x < 7 && x > 0) {
						if(!(board[y - color_value][x + 1]->type == piece_type::PAWN ||
						board[y - color_value][x - 1]->type == piece_type::PAWN)) 
							score -= color_value * 0.01f;
						else 
							score += color_value * 0.01f;
					}
					if (board[y - color_value][x]->type == piece_type::PAWN  && board[y - color_value][x]->color == board[y][x]->color)
						score += color_value * 0.01f;
					break;
				default:
					break;
				}

				int goodness_y = src_piece.color == piece_color::BLACK ? 7 - y : y;
				float piece_goodness = piece_goodnesses[phase == game_phase::ENDGAME][int(src_piece.type)][goodness_y][x];
				score += color_value * 0.01 * piece_goodness;
				
				score += color_value * get_piece_value(src_piece.type);
			}
		}


		if(phase == game_phase::ENDGAME){
			std::array<int, 2> mobility_score = targetted_squares_count();
			score += 0.001 * (mobility_score[0] - mobility_score[1]);
			float delta_x = king_positions[0]->x - king_positions[1]->x;
			float delta_y = king_positions[0]->y - king_positions[1]->y;
			score *= 3.f/sqrt(pow(delta_x, 2) + pow(delta_y , 2)) + 1;
		}

		return score;
	}


	bool is_under_check(piece_color color) {
		uint64_t targetted_squares = 0;
		optional<board_pos> king_pos = std::nullopt;
		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {

				if (!board[y][x].has_value())
					continue;

				if (board[y][x]->type == piece_type::KING && board[y][x]->color == piece_color::BLACK)
					king_pos = board_pos{ x, y };

				if (board[y][x].value().color != color)
					targetted_squares |= get_moves({ x, y });
			}
		}
		if(!king_pos.has_value())
			return false;
		return bitboard_get(targetted_squares, king_pos.value());
	}

	bool search_at_depth(std::vector<move>& old_best_moves, int depth, steady_clock::time_point then) const {
		std::vector<move> best_moves;
		size_t best_moves_size = 0;
		float max = INT_MIN;
		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value() || board[y][x].value().color != turn)
					continue;
				if (old_best_moves.size() > 0 && duration_cast<seconds>(steady_clock::now() - then).count() >= 1)
					return true;

				uint64_t move_bitboard = get_moves(board_pos{ x, y });

				while (move_bitboard != 0) {
					unsigned long index = std::countr_zero(move_bitboard);
					move_bitboard &= move_bitboard - 1;
					board_pos dst_pos = {int(index) % 8, int(index) / 8 };
					Position new_position = *this;
					new_position.make_move(dst_pos, { x, y }, nullptr);
		
					if (new_position.is_under_check(turn))
						continue;

					float evaluation = -new_position.negamax(-1e9, 1e9, depth, piece_color(!bool(turn)));
					std::cout << depth << '\n';
					
					if (evaluation > max) {
						max = evaluation;
						best_moves.clear();
					}

					if(evaluation >= max) {
						best_moves.push_back({ dst_pos, {x, y} });
						best_moves_size = std::max(best_moves.size(), best_moves_size);
					}


				}
			}
		}

		old_best_moves = best_moves;
		return false;
	}

	move ai_move() const {
		std::vector<move> best_moves;
		best_moves.reserve(4);
		int current_depth = 2;
		float last_eval;
		auto then = steady_clock::now();
		while(true){
			if(search_at_depth(best_moves, current_depth, then))
				break;
			current_depth++;
		}
		// TODO: On checkmate implementation check if best_moves.size() > 0
		return best_moves[best_moves.size() == 1 ? 0 : std::rand() % best_moves.size()];
	}

	void order_moves(std::vector<move>& moves) const {
		for (move &current_move : moves) {
			float move_score = 0;
			piece_type move_piece_type = board[current_move.src_pos.y][current_move.src_pos.x]->type;

			if (board[current_move.dst_pos.y][current_move.dst_pos.x].has_value()) {
				piece_type capture_piece_type = board[current_move.dst_pos.y][current_move.dst_pos.x]->type;
				move_score += 10 * (get_piece_value(capture_piece_type) - get_piece_value(move_piece_type));

				if(capture_piece_type == piece_type::KING)
					move_score += 1'000'000;
			}

			if (move_piece_type == piece_type::PAWN && (current_move.dst_pos.y == 0 || current_move.dst_pos.y == 7)) {
				move_score += get_piece_value(piece_type::QUEEN);
			}
			current_move.score = move_score;
		}
		// moves.erase(std::remove(moves.begin(), moves.end(), 0), moves.end());
		std::sort(moves.begin(), moves.end(), [](const move& a, const move& b) {
			return a.score > b.score;
		});

	}



	float search_captures(float alpha, float beta, int depth) const {
		float score = get_color_value(turn) * evaluate();
		if (score >= beta)
			return beta;
		alpha = std::max(score, alpha);
		if(depth == 0)
			return alpha;
		std::vector<move> moves;

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;

				uint64_t move_bitboard = get_moves(board_pos{ x, y });
				while (move_bitboard != 0) {
					unsigned long index = std::countr_zero(move_bitboard);
					move_bitboard &= move_bitboard - 1;
					board_pos dst_pos = { int(index) % 8, int(index) / 8 };
					if (board[dst_pos.y][dst_pos.x].has_value())
						moves.push_back({ dst_pos, { x, y }, true });
				}
			}
		}

		order_moves(moves);
		for (int i = 0; i < moves.size(); i++) {
			Position new_position = *this;
			new_position.make_move(moves[i].dst_pos, moves[i].src_pos, nullptr);
			score = -new_position.search_captures(-beta, -alpha, depth - 1);
			if (score >= beta)
				return beta;
			alpha = std::max(score, alpha);
		}

		return alpha;
	}

	float evaluate_negamax (float alpha, float beta, int depth) {
		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value() || board[y][x]->color != turn)
					continue;

				uint64_t move_bitboard = get_moves(board_pos{ x, y });
				while (move_bitboard != 0) {
					unsigned long index = std::countr_zero(move_bitboard);
					move_bitboard &= move_bitboard - 1;
					board_pos dst_pos = { int(index) % 8, int(index) / 8 };

					if (board[dst_pos.y][dst_pos.x].has_value() && board[dst_pos.y][dst_pos.x]->type == piece_type::KING)
						return 1'000'000.f + depth * 1'000'000.f;

					Position new_position = *this;
					new_position.make_move(dst_pos, board_pos{x, y}, nullptr);
					float score = -new_position.negamax(-beta, -alpha, depth - 1, piece_color(!bool(turn)));
					if (score >= beta)
						return beta;
					alpha = std::max(alpha, score);

				}
			}
		}

		return alpha;
	}

	float negamax(float alpha, float beta, int depth, piece_color turn) {
		float best_score = -1'000'000.f;
		uint64_t current_hash = hash();

		if(auto it = transposition_table.table.find(current_hash); it != transposition_table.table.end() && it->second.depth >= depth)
			return it->second.evaluation;

		float evaluation = depth == 0 ? search_captures(alpha, beta, 2) : evaluate_negamax(alpha, beta, depth);
		transposition_table.table[current_hash] = {.evaluation = evaluation, .depth = depth};

		return evaluation;
	}

};