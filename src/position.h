class Position : public PieceMoves {
public:
	game_phase get_game_phase() const{
		int total_pieces = 0;
		int total_major_pieces = 0;
		int total_pawns = 0;

		for (int y = 0; y < board.size(); y++) 
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;
				total_pieces++;
				if (board[y][x]->type == piece_type::KING)
					continue;
				if (board[y][x].value().type != piece_type::PAWN)
					total_pawns++;
				else
					total_major_pieces++;

			}
		if (total_pieces <= 10 && total_major_pieces <= 3)
			return game_phase::ENDGAME;
		else if (total_pawns >= 11)
			return game_phase::OPENING;
		else
			return game_phase::MIDGAME;
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

	float evaluate() const{
		float eval = 0;

		game_phase phase = get_game_phase();

		std::array<board_pos, 2> king_positions;

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;

				piece src_piece = board[y][x].value();
				switch(src_piece.type) {
				case piece_type::KING:
					king_positions[src_piece.color == piece_color::BLACK ? 1 : 0] = board_pos{ x, y };
					break;
				case piece_type::PAWN:
					if(x < 7 && x > 0) {
						if(!(board[y + get_color_value(src_piece.color)][x + 1]->type == piece_type::PAWN ||
						board[y + get_color_value(src_piece.color)][x - 1]->type == piece_type::PAWN)) 
							eval -= get_color_value(src_piece.color) * 0.5f;
						else 
							eval += get_color_value(src_piece.color) * 1.1f;
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
			eval *= 3.f/sqrt(pow(king_positions[0].x - king_positions[1].x, 2) + pow(king_positions[0].y - king_positions[1].y, 2))  + 1;
		}
		// if(phase == game_phase::ENDGAME)
		// if(king_positions[0].has_value() && king_positions[1].has_value())
		// 	eval *= 3/sqrt(pow(king_positions[0]->x - king_positions[1]->x, 2) + pow(king_positions[0]->y - king_positions[1]->y, 2))  + 1;
		
		return eval;
	}

	
	bool is_under_check(piece_color color) {
		uint64_t targetted_squares = 0;
		board_pos king_pos;
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
		return bitboard_get(targetted_squares, king_pos);
	}

	move ai_move() const {
		std::vector<move> best_moves;
		best_moves.reserve(4);
		size_t best_moves_size = 0;
		array<int, 3> game_phase_depth = {2, 2, 4}; 
		float max = INT_MIN;

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value() || board[y][x].value().color != turn)
					continue;

				uint64_t move_bitboard = get_moves(board_pos{ x, y });
				while (move_bitboard != 0) {
					unsigned long index = std::countr_zero(move_bitboard);
					move_bitboard &= move_bitboard - 1;
					board_pos dst_pos = { static_cast<int>(index) % 8, static_cast<int>(index) / 8 };
					Position new_position = *this;
					new_position.make_move(dst_pos, { x, y }, nullptr);
					if (!new_position.is_under_check(turn)) {
						float eval = -new_position.negamax(-1e9, 1e9, game_phase_depth[int(get_game_phase())], static_cast<piece_color>(!static_cast<bool>(turn)));
						if (eval > max) {
							max = eval;
							best_moves.clear();
							best_moves.push_back({ dst_pos, {x, y} });
							best_moves_size = std::max(best_moves.size(), best_moves_size);
						}
						else if (eval == max) {
							best_moves.push_back({ dst_pos, {x, y} });
							best_moves_size = std::max(best_moves.size(), best_moves_size);
						}
					}
				}
			}

		}
		// std::cout << best_moves_size << "<- Best move max size " << best_moves.size() << "<- Best move size \n";
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

	float negamax(float alpha, float beta, int depth, piece_color turn) const {
		float best_score = -10000000;
		if (depth == 0)
			return search_captures(alpha, beta);
			//return turn == piece_color::BLACK ? -evaluate() : evaluate();

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value() || board[y][x]->color != turn)
					continue;

				uint64_t move_bitboard = get_moves(board_pos{ x, y });
				while (move_bitboard != 0) {
					unsigned long index = std::countr_zero(move_bitboard);
					move_bitboard &= move_bitboard - 1;
					board_pos dst_pos = { static_cast<int>(index) % 8, static_cast<int>(index) / 8 };
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

};