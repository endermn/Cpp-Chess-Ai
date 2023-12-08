

class Canvas {

	SDL_Renderer* rend;
	SDL_Texture* digits_image;
	SDL_Texture* pieces_image;
public:
	Canvas (SDL_Renderer* rend, SDL_Texture* digits_image, SDL_Texture* pieces_image) {
		this->rend = rend;

		if (!pieces_image || !digits_image) {
			messagebox_error("FAILED TO LOAD SPRITES", "Failed to load sprites");
			exit(1);
		}

		this->digits_image = digits_image;
		this->pieces_image = pieces_image;
	}

	void draw_digits(float x, float y, int digit, float height) {
		SDL_Rect src_rect{digit * 100, 0, 100, 112};
		SDL_FRect dst_rect = {x , y + .25f, 0.5, height / 2};
		SDL_RenderCopyF(rend, digits_image, &src_rect, &dst_rect);
	}

	void draw_timers(seconds b_time, seconds w_time) {

		draw_digits(8, 0, int(b_time.count()) / 600, 1);
		draw_digits(8.5, 0, int(b_time.count()) / 60 % 10, 1);

		draw_digits(8, 7, int(w_time.count()) / 600, 1);
		draw_digits(8.5, 7, int(w_time.count()) / 60 % 10, 1);

		draw_digits(9, 0, int(b_time.count()) % 60 / 10, 0.9);
		draw_digits(9.5, 0, int(b_time.count()) % 60 % 10, 0.9);

		draw_digits(9, 7, int(w_time.count()) % 60 / 10, 0.9);
		draw_digits(9.5, 7, int(w_time.count()) % 60 % 10, 0.9);

	}

	void draw(
		seconds b_time, 
		seconds w_time,
		array<array<optional<piece>, 8>, 8> const &board, 
		Bitboard bitboard, 
		piece_color turn
	) {

		SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
		SDL_RenderClear(rend);

		bool is_white = true;

		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				is_white ? SDL_SetRenderDrawColor(rend, 0xFB, 0xF7, 0xE9, 255) : SDL_SetRenderDrawColor(rend, 0xA6, 0xCF, 0xCA, 255);
				if(x != 7)
					is_white = !is_white;
				SDL_Rect dst_rect = {x, y, 1, 1};
				SDL_RenderFillRect(rend, &dst_rect);
				if (bitboard.bitboard_get({ x, y })) {
					SDL_SetRenderDrawColor(rend, 200, 100, 100, 130);
					SDL_RenderFillRect(rend, &dst_rect);
				}
				if (board[y][x].has_value()) {
					SDL_Rect src_rect = {int(board[y][x].value().type) * 200, int(board[y][x].value().color) * 200, 200, 200};
					SDL_RenderCopy(rend, pieces_image, &src_rect, &dst_rect);
				}
			}
		}


		


		SDL_SetRenderDrawColor(rend, 173, 235, 173, 255);
		SDL_Rect black_rect = {8, turn == piece_color::BLACK ? 0 : 7, 2, 1};
		SDL_RenderFillRect(rend, &black_rect);
		SDL_SetRenderDrawColor(rend, 0xE0, 0xE0, 0xE0, 255);
		SDL_Rect white_rect = {8, turn == piece_color::BLACK ? 7 : 0, 2, 1};
		SDL_RenderFillRect(rend, &white_rect);
		draw_timers(b_time, w_time);

		SDL_RenderPresent(rend);
	}

};

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