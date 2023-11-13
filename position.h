class Position {
public:
	BOARD board;
	optional<int8_t> en_passant;
	piece_color turn = piece_color::BLACK;
	array<array<bool, 2>, 2> can_castle;  // FIRST INDEX = COLOR, 2ND INDEX = SIDE 

	uint64_t king_moves(board_pos selected_piece_pos) const {
		uint64_t bitboard = 0;
		piece_color src_color = board[selected_piece_pos.y][selected_piece_pos.x].value().color;
		for (board_pos i : piece_offsets::KING_QUEEN)
		{
			board_pos dst = { i.x + selected_piece_pos.x , i.y + selected_piece_pos.y };
			if (dst.is_valid() && (!board[dst.y][dst.x].has_value() || board[dst.y][dst.x].value().color != src_color))
				bitboard_set(bitboard, dst);
			if (can_castle[int(src_color)][int(castle_side::SHORT)]) {
				board_pos dst_castle = { 6, selected_piece_pos.y };
				if (board[dst_castle.y][7] == piece{.color = src_color, .type = piece_type::ROOK } &&
					!board[selected_piece_pos.y][5].has_value() &&
					!board[selected_piece_pos.y][6].has_value())
					bitboard_set(bitboard, dst_castle);
			}
			if (can_castle[int(src_color)][int(castle_side::LONG)]) {
				board_pos dst_castle = { 2, selected_piece_pos.y };
				if (board[dst_castle.y][0] == piece{ .color = src_color, .type = piece_type::ROOK } &&
					!board[selected_piece_pos.y][1].has_value() &&
					!board[selected_piece_pos.y][2].has_value() &&
					!board[selected_piece_pos.y][3].has_value())
					bitboard_set(bitboard, dst_castle);
			}
		}
		return bitboard;
	}

	uint64_t bishop_moves(board_pos selected_piece_pos) const {
		uint64_t bitboard = 0;
		for (board_pos i : piece_offsets::BISHOP)
			for (int j = 1; j < 8; j++) {
				board_pos diagonal = { selected_piece_pos.x + j * i.x , selected_piece_pos.y + j * i.y };
				if (!diagonal.is_valid()) break;
				if (board[diagonal.y][diagonal.x].has_value())
				{
					if (board[diagonal.y][diagonal.x].value().color != board[selected_piece_pos.y][selected_piece_pos.x].value().color)
						bitboard_set(bitboard, diagonal);
					break;
				}
				bitboard_set(bitboard, diagonal);
			}
		return bitboard;
	}

	uint64_t knight_moves(board_pos selected_piece_pos) const {
		uint64_t bitboard = 0;
		piece_color src_color = board[selected_piece_pos.y][selected_piece_pos.x].value().color;
		for (board_pos i : piece_offsets::KNIGHT)
		{
			board_pos dst = { i.x + selected_piece_pos.x , i.y + selected_piece_pos.y };
			if (dst.is_valid() && (!board[dst.y][dst.x].has_value() || board[dst.y][dst.x].value().color != src_color))
				bitboard_set(bitboard, dst);
		}
		return bitboard;
	}

	uint64_t rook_moves(board_pos selected_piece_pos) const {
		uint64_t bitboard = 0;
		for (board_pos i : piece_offsets::ROOK)
			for (int j = 1; j < 8; j++) {
				board_pos diagonal = { selected_piece_pos.x + j * i.x , selected_piece_pos.y + j * i.y };
				if (!diagonal.is_valid()) break;
				if (board[diagonal.y][diagonal.x].has_value())
				{
					if (board[diagonal.y][diagonal.x].value().color != board[selected_piece_pos.y][selected_piece_pos.x].value().color)
						bitboard_set(bitboard, diagonal);
					break;
				}
				bitboard_set(bitboard, diagonal);
			}
		return bitboard;
	}

	uint64_t queen_moves(board_pos selected_piece_pos) const {
		return bishop_moves(selected_piece_pos) | rook_moves(selected_piece_pos);
	}

	uint64_t pawn_moves(board_pos selected_piece_pos) const {
		uint64_t bitboard = 0;

		piece_color selected_piece_color = board[selected_piece_pos.y][selected_piece_pos.x].value().color;

		int next_y = selected_piece_pos.y - 1 + 2 * int(selected_piece_color);
		//std::cout << selected_piece_pos.y << " " << next_y << '\n';
		int double_move_y = selected_piece_color == piece_color::WHITE ? 4 : 3;

		if (selected_piece_pos.x < 7)
			if (optional<piece> dst = board[next_y][selected_piece_pos.x + 1];
			dst.has_value() && dst.value().color != selected_piece_color)
				bitboard_set(bitboard, { selected_piece_pos.x + 1, next_y});
			else
				if (selected_piece_pos.x + 1 == en_passant &&
					next_y == (selected_piece_color == piece_color::WHITE ? 2 : 5) &&
					board[selected_piece_pos.y][selected_piece_pos.x + 1].has_value() &&
					board[selected_piece_pos.y][selected_piece_pos.x + 1]->type == piece_type::PAWN &&
					board[selected_piece_pos.y][selected_piece_pos.x + 1]->color != board[selected_piece_pos.y][selected_piece_pos.x]->color
					)
						bitboard_set(bitboard, { selected_piece_pos.x + 1, next_y });
		
		if (selected_piece_pos.x > 0)
			if (optional<piece> dst = board[next_y][selected_piece_pos.x - 1];
			dst.has_value() && dst.value().color != selected_piece_color)
				bitboard_set(bitboard, { selected_piece_pos.x - 1, next_y});
			else 
				if(selected_piece_pos.x - 1 == en_passant &&
				next_y == (selected_piece_color == piece_color::WHITE ? 2 : 5) &&
				board[selected_piece_pos.y][selected_piece_pos.x - 1].has_value() &&
				board[selected_piece_pos.y][selected_piece_pos.x - 1]->type == piece_type::PAWN &&
				board[selected_piece_pos.y][selected_piece_pos.x - 1]->color != board[selected_piece_pos.y][selected_piece_pos.x]->color
					)
					bitboard_set(bitboard, {selected_piece_pos.x - 1, next_y});

		if (!board[next_y][selected_piece_pos.x].has_value()) {
			if (!board[double_move_y][selected_piece_pos.x].has_value())
				if (selected_piece_pos.y == (selected_piece_color == piece_color::WHITE ? 6 : 1))
					bitboard_set(bitboard, { selected_piece_pos.x , double_move_y });
			bitboard_set(bitboard, { selected_piece_pos.x, next_y });
		}

		return bitboard;
	}
	void make_move(board_pos target, board_pos selected_piece_pos, SDL_Window* win) {
		piece src_piece = board[selected_piece_pos.y][selected_piece_pos.x].value();

		int8_t compare_color = src_piece.color == piece_color::WHITE ? 7 : 0;

		int double_move_y = src_piece.color == piece_color::WHITE ? 4 : 3;
		int next_y = selected_piece_pos.y - 1 + 2 * int(src_piece.color);

		if (src_piece.type == piece_type::PAWN) {
			if (target.y == 0 || target.y == 7)
			{
				if (win != nullptr)
					src_piece.type = promote_message(win);
				else
					src_piece.type = piece_type::QUEEN;
			}
			if (target == board_pos{ selected_piece_pos.x, double_move_y })
				en_passant = selected_piece_pos.x;
			else 
				en_passant = std::nullopt;
			if (target == board_pos{ selected_piece_pos.x + 1, next_y } &&
				!board[target.y][target.x].has_value())
			{
				board[selected_piece_pos.y][selected_piece_pos.x + 1] = std::nullopt;
				en_passant = std::nullopt;
			}
			if(target == board_pos{ selected_piece_pos.x - 1, next_y } &&
				!board[target.y][target.x].has_value())
			{
				board[selected_piece_pos.y][selected_piece_pos.x - 1] = std::nullopt;
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
			if (selected_piece_pos == board_pos{ 4, compare_color } &&
				target == board_pos{ 6, compare_color }) {
				board[target.y][5] = board[target.y][7];
				board[target.y][7] = std::nullopt;
			}
			if (selected_piece_pos == board_pos{ 4, compare_color } &&
				target == board_pos{ 2, compare_color }) {
				board[target.y][3] = board[target.y][0];
				board[target.y][0] = std::nullopt;
			}

			can_castle[int(src_piece.color)] = { false, false };
		}
		board[target.y][target.x] = src_piece;
		board[selected_piece_pos.y][selected_piece_pos.x] = std::nullopt;
	}

	uint64_t get_moves(board_pos src_pos) const {
		switch (board[src_pos.y][src_pos.x].value().type) {
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

	float evaluate(piece_color turn) const{
		float eval = 0;
		float array_of_values[6] = {
			200,
			9.5,
			3.33,
			3.05,
			5.63,
			1,
		};

		
		std::array<std::array<int, 8>, 8> piece_square_king_eg = {
			{
				{100, 70, 30, 30, 30, 30, 70, 100},
				{},
				{},
				{},
				{},
				{},
				{},
				{100, 70, 30, 30, 30, 30, 70, 100},
			}
		};

		std::array<int, 2> mobility_score = targetted_squares_count();
		game_phase phase = get_game_phase();

		std::array<board_pos, 2> king_positions;

		for (int y = 0; y < board.size(); y++) {
			for (int x = 0; x < board[y].size(); x++) {
				if (!board[y][x].has_value())
					continue;

				piece src_piece = board[y][x].value();
				if (src_piece.type == piece_type::KING)
				{
					//if(src_piece.color == piece_color::BLACK)
					//	eval += sqrt(pow(abs(king_positions[0].x - king_positions[1].x), 2) + pow(abs(king_positions[0].y - king_positions[1].y), 2));
					if (phase == game_phase::ENDGAME)
						eval += src_piece.color == piece_color::BLACK ? piece_square_king_eg[y][x] : -piece_square_king_eg[y][x];
					king_positions[src_piece.color == piece_color::BLACK ? 1 : 0] = board_pos{ x, y };
				}
						
				eval += src_piece.color == piece_color::BLACK ? -0.01 * piece_goodness[phase == game_phase::OPENING ? 0 : 1][int(src_piece.type)][7 - y][x] : 0.01 * piece_goodness[phase == game_phase::OPENING ? 0 : 1][int(src_piece.type)][y][x];
				eval += src_piece.color == piece_color::BLACK ? -array_of_values[int(src_piece.type)] : array_of_values[int(src_piece.type)];
			}
		}

		eval += (-2 * bool(turn) + 1) * 0.01 * (mobility_score[turn == piece_color::BLACK ? 1 : 0] - mobility_score[turn == piece_color::BLACK ? 0 : 1]);
		return eval;
	}

	float get_piece_value(piece_type piece) {
		float array_of_values[6] = {
			200,
			9.5,
			3.33,
			3.05,
			5.63,
			1,
		};
		return array_of_values[static_cast<int>(piece)];
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

	void ai_move(piece_color turn) {
		std::vector<move> best_moves;
		int depth = 2;
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
						float eval = -new_position.negamax(INT_MIN + 1, INT_MAX, static_cast<int>(get_game_phase()) + depth - 1, static_cast<piece_color>(!static_cast<bool>(turn)));
						if (eval > max) {
							max = eval;
							best_moves = { {dst_pos, {x, y}} };
						}
						else if (eval == max) {
							best_moves.push_back({ dst_pos, {x, y} });
						}
					}
				}
			}
		}

		int random = std::rand() % best_moves.size();
		make_move(best_moves[random].dst_pos, best_moves[random].src_pos, nullptr);
	}
	float get_piece_value(piece_type type) const{
		float array_of_values[6] = {
			200,
			9.5,
			3.33,
			3.05,
			5.63,
			1,
		};
		return array_of_values[int(type)];
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
			}

			if (move_piece_type == piece_type::PAWN && (new_move.dst_pos.y == 0 || new_move.dst_pos.y == 7)) {
				move_score += 10 * get_piece_value(piece_type::QUEEN);
			}
			new_move.score = move_score;
		}
		//moves.erase(std::remove(moves.begin(), moves.end(), 0), moves.end());
		std::sort(moves.begin(), moves.end(), [](const move& a, const move& b) {
			return a.score > b.score;
		});

	}

	float search_captures(float alpha, float beta, piece_color turn) const {
		float eval = turn == piece_color::BLACK ? -evaluate(turn) : evaluate(turn);
		//float eval = evaluate(turn);
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
					board_pos dst_pos = { static_cast<int>(index) % 8, static_cast<int>(index) / 8 };
					if (board[dst_pos.y][dst_pos.x].has_value())
						moves.push_back({ dst_pos, { x, y }, true });
				}
			}
		}

		order_moves(moves);
		for (int i = 0; i < moves.size(); i++) {
			Position new_position = *this;
			new_position.make_move(moves[i].dst_pos, moves[i].src_pos, nullptr);
			eval = -new_position.search_captures(-beta, -alpha, piece_color(!bool(turn)));
			if (eval >= beta)
				return beta;
			alpha = std::max(eval, alpha);
		}

		return alpha;
	}

	float negamax(float alpha, float beta, int depth, piece_color turn) const {
		if (depth == 0)
			return search_captures(-beta, -alpha, turn);
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