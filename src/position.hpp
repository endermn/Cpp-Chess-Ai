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
				if(board[y][x].has_value())
					current_hash ^= transposition_table.piece_zobrist[y][x][transposition_table.piece_to_hash(board[y][x]->color, board[y][x]->type)];

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
		float eval = 0;

		game_phase phase = get_game_phase();

		std::array<optional<board_pos>, 2> king_positions = {std::nullopt, std::nullopt};

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;

				piece src_piece = board[y][x].value();
				switch(src_piece.type) {
				case piece_type::KING:
					for (int i = -1; i < 2; i++) {
						board_pos y_pos = {.x = x + i, .y = y - get_color_value(src_piece.color)};
						if(!y_pos.is_valid())
							break;
						if (board[y_pos.y][x + i].has_value())
							eval += get_color_value(src_piece.color) * 0.1;
						if (!board[y_pos.y][x].has_value())
							eval -= get_color_value(src_piece.color) * 0.2;
					}
					king_positions[src_piece.color == piece_color::BLACK ? 1 : 0] = board_pos{ x, y };
					break;
				case piece_type::PAWN:
					if(x < 7 && x > 0) {
						if(!(board[y + get_color_value(src_piece.color)][x + 1]->type == piece_type::PAWN ||
						board[y + get_color_value(src_piece.color)][x - 1]->type == piece_type::PAWN)) 
							eval -= get_color_value(src_piece.color) * 0.01f;
						else 
							eval += get_color_value(src_piece.color) * 0.01f;
					}
					if (board[y + get_color_value(src_piece.color)][x]->type == board[y][x]->type)
						eval -= get_color_value(src_piece.color) * 0.5f;
					break;
				default:
					break;
				}

				eval += get_color_value(src_piece.color) * 0.01 * piece_goodness[phase == game_phase::ENDGAME ? 1 : 0][int(src_piece.type)][src_piece.color == piece_color::BLACK ? 7 - y : y][x];
				
				eval += get_color_value(src_piece.color) * get_piece_value(src_piece.type); //array_of_values[int(src_piece.type)];
			}
		}


		if(phase == game_phase::ENDGAME){
			std::array<int, 2> mobility_score = targetted_squares_count();
			eval += 0.001 * (mobility_score[0] - mobility_score[1]);
			eval *= 3.f/sqrt(pow(king_positions[0]->x - king_positions[1]->x, 2) + pow(king_positions[0]->y - king_positions[1]->y, 2))  + 1;
		}
		// if(phase == game_phase::ENDGAME)
		// if(king_positions[0].has_value() && king_positions[1].has_value())
		// 	eval *= 3/sqrt(pow(king_positions[0]->x - king_positions[1]->x, 2) + pow(king_positions[0]->y - king_positions[1]->y, 2))  + 1;
		
		return eval;
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

	bool search_at_depth(std::vector<move>& old_best_moves, int depth, std::chrono::steady_clock::time_point then) const {
		std::vector<move> best_moves;
		size_t best_moves_size = 0;
		float max = INT_MIN;
		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value() || board[y][x].value().color != turn)
					continue;
				if (old_best_moves.size() > 0) 
					if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - then).count() >= 1)
						return true;

				uint64_t move_bitboard = get_moves(board_pos{ x, y });
				while (move_bitboard != 0) {
					unsigned long index = std::countr_zero(move_bitboard);
					move_bitboard &= move_bitboard - 1;
					board_pos dst_pos = { static_cast<int>(index) % 8, static_cast<int>(index) / 8 };
						
					Position new_position = *this;
					new_position.make_move(dst_pos, { x, y }, nullptr);
					if (!new_position.is_under_check(turn)) {
						float evaluation = -new_position.negamax(-1e9, 1e9, depth, piece_color(!bool(turn)));
						std::cout << depth << '\n';
						// std::cout << transposition_table.table.size();
						if (evaluation > max) {
							max = evaluation;
							best_moves.clear();
							best_moves.push_back({ dst_pos, {x, y} });
							best_moves_size = std::max(best_moves.size(), best_moves_size);
						}
						else if (evaluation == max) {
							best_moves.push_back({ dst_pos, {x, y} });
							best_moves_size = std::max(best_moves.size(), best_moves_size);
						}
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
		auto then = std::chrono::steady_clock::now();
		while(true){
			if(search_at_depth(best_moves, current_depth, then))
				break;
			current_depth++;
		}
		// search_at_depth(best_moves, 4, then);
		// std::cout << best_moves_size << "<- Best move max size " << best_moves.size() << "<- Best move size \n";
		// TODO: On checkmate implementation check if best_moves.size() > 0
		int selected_move = best_moves.size() == 1 ? 0 : std::rand() % best_moves.size();
		return best_moves[selected_move];
	}

	void order_moves(std::vector<move>& moves) const {
		for (move &new_move : moves) {
			float move_score = 0;
			piece_type move_piece_type = board[new_move.src_pos.y][new_move.src_pos.x]->type;
			std::optional<piece_type> capture_piece_type = std::nullopt;

			if (board[new_move.dst_pos.y][new_move.dst_pos.x].has_value())
			{
				capture_piece_type = board[new_move.dst_pos.y][new_move.dst_pos.x]->type;
				move_score += 10 * (get_piece_value(capture_piece_type.value()) - get_piece_value(move_piece_type));
				if(capture_piece_type == piece_type::KING)
					move_score += 10000;
			}

			if (move_piece_type == piece_type::PAWN && (new_move.dst_pos.y == 0 || new_move.dst_pos.y == 7)) {
				move_score += 10 * get_piece_value(piece_type::QUEEN);
			}
			new_move.score = move_score;
		}
		// moves.erase(std::remove(moves.begin(), moves.end(), 0), moves.end());
		std::sort(moves.begin(), moves.end(), [](const move& a, const move& b) {
			return a.score > b.score;
		});

	}



	float search_captures(float alpha, float beta) const {
		float eval = get_color_value(turn) * evaluate();
		if (eval >= beta)
			return beta;
		alpha = std::max(eval, alpha);
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
			eval = -new_position.search_captures(-beta, -alpha);
			// eval = -new_position.evaluate();
			if (eval >= beta)
				return beta;
			// if(eval > alpha)
			// 	alpha = eval;
			alpha = std::max(eval, alpha);
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
					board_pos dst_pos = { static_cast<int>(index) % 8, static_cast<int>(index) / 8 };

					if (board[dst_pos.y][dst_pos.x].has_value() && board[dst_pos.y][dst_pos.x]->type == piece_type::KING)
						return 1920398.f + depth * 1'000'000.f;
					Position new_position = *this;
					new_position.make_move(dst_pos, board_pos{x, y}, nullptr);
					float eval = -new_position.negamax(-beta, -alpha, depth - 1, piece_color(!bool(turn)));
					if (eval >= beta)
						return beta;
					alpha = std::max(alpha, eval);

				}
			}
		}

		return alpha;
	}

	float negamax(float alpha, float beta, int depth, piece_color turn) {
		float best_score = -10000000;
		uint64_t current_hash = hash();
		//TODO: Use find instead of contains
		if(transposition_table.table.contains(current_hash)  && transposition_table.table[current_hash].depth == depth)
			return transposition_table.table[current_hash].evaluation;
		// if (depth == 0)
		// 	return search_captures(alpha, beta);
			//return turn == piece_color::BLACK ? -evaluate() : evaluate();
			
		float evaluation = depth == 0 ? search_captures(alpha, beta) : evaluate_negamax(alpha, beta, depth);
		transposition_table.table[current_hash] = {.evaluation = evaluation, .depth = depth};

		return evaluation;
	}

};