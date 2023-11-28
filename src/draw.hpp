piece_type promote_message(SDL_Window* win) {
	int buttonid = 0;

	const SDL_MessageBoxButtonData buttons[4] = {
		{
			.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
			.buttonid = int(piece_type::QUEEN),
			.text = "Queen",
		},
		{
			.flags = 0,
			.buttonid = int(piece_type::ROOK),
			.text = "Rook",
		},
		{
			.flags = 0,
			.buttonid = int(piece_type::BISHOP),
			.text = "Bishop",
		},
		{
			.flags = 0,
			.buttonid = int(piece_type::KNIGHT),
			.text = "Knight",
		}
	};

	SDL_MessageBoxData message_box_data = {
		.flags = 0,
		.window = win,
		.title = "promote a piece",
		.numbuttons = 4,
		.buttons = buttons,

	};
	SDL_ShowMessageBox(&message_box_data, &buttonid);
	return piece_type(buttonid);
}

void draw_digits(float x, float y, SDL_Renderer* rend, SDL_Texture* digits_image, int digit) {
	SDL_Rect src_rect{digit * 100, 0, 100, 200};
	SDL_FRect dst_rect = {x, y, 0.5, 1};
	SDL_RenderCopyF(rend, digits_image, &src_rect, &dst_rect);
}


void draw(std::chrono::seconds b_time, std::chrono::seconds w_time, SDL_Renderer* rend, array<array<optional<piece>, 8>, 8> const &board, SDL_Texture* pieces_image, SDL_Texture* digits_image , uint64_t bitboard, bool is_thinking) {
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
	SDL_RenderClear(rend);
	bool is_white = true;
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			is_white ? SDL_SetRenderDrawColor(rend, 255, 255, 255, 255) : SDL_SetRenderDrawColor(rend, 53, 173, 107, 255);
			if(x != 7)
				is_white = !is_white;
			SDL_Rect dst_rect = {x, y, 1, 1};
			SDL_RenderFillRect(rend, &dst_rect);
			if (bitboard_get(bitboard, { x, y }))
			{
				SDL_SetRenderDrawColor(rend, 200, 100, 100, 130);
				SDL_RenderFillRect(rend, &dst_rect);
			}
			if (board[y][x].has_value()) {
				SDL_Rect src_rect = {int(board[y][x].value().type) * 200, int(board[y][x].value().color) * 200, 200, 200};
				SDL_RenderCopy(rend, pieces_image, &src_rect, &dst_rect);
			}
		}
	}


	
	draw_digits(8, 0, rend, digits_image, int(b_time.count()) / 600);
	draw_digits(8.5, 0, rend, digits_image, int(b_time.count()) / 60 % 10);

	draw_digits(8, 7, rend, digits_image, int(w_time.count()) / 600);
	draw_digits(8.5, 7, rend, digits_image, int(w_time.count()) / 60 % 10);

	draw_digits(9, 0, rend, digits_image, int(b_time.count()) % 60 / 10);
	draw_digits(9.5, 0, rend, digits_image, int(b_time.count()) % 60 % 10);

	draw_digits(9, 7, rend, digits_image, int(w_time.count()) % 60 / 10);
	draw_digits(9.5, 7, rend, digits_image, int(w_time.count()) % 60 % 10);


	SDL_SetRenderDrawColor(rend, 0, 255, 0, 255);
	SDL_Rect dst_rect = {9, is_thinking ? 1 : 6, 1, 1};
	SDL_RenderFillRect(rend, &dst_rect);

	SDL_RenderPresent(rend);
}