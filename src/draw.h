

void draw(SDL_Renderer* rend, BOARD const &board, uint64_t bitboard) {
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