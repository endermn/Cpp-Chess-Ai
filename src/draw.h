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

void draw(SDL_Renderer* rend, array<array<optional<piece>, 8>, 8> const &board, uint64_t bitboard) {
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
	SDL_RenderPresent(rend);
}