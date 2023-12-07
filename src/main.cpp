#include "common.hpp"
#include "pieces.hpp"
#include "bitboard.hpp"
#include "draw.hpp"
#include "transposition_table.hpp"
#include "piece_goodness.hpp"
#include "piece_moves.hpp"
#include "position.hpp"
#include "fen.hpp"
#include "threads.hpp"

using namespace std::literals;


int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);

	SDL_Window* win = SDL_CreateWindow("chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SQUARE_SIZE * 10, SQUARE_SIZE * 8, SDL_WINDOW_RESIZABLE);
	SDL_ShowWindow(win);

	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC );
	
	SDL_Texture* pieces_image = IMG_LoadTexture(rend, "./sprites/pieces.png");
	SDL_Texture* digits_image = IMG_LoadTexture(rend, "./sprites/digits.png");


	if (!pieces_image || !digits_image) {
		messagebox_error("FAILED TO LOAD SPRITES", "Failed to load sprites");
		exit(1);
	}

	SDL_SetTextureScaleMode(digits_image, SDL_ScaleModeBest);
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_RenderSetLogicalSize(rend, 10, 8);
	SDL_SetTextureScaleMode(pieces_image, SDL_ScaleModeLinear);

// Test Fens:
	// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
	// thread_sync sync = {.position = fen_to_position("4kb1r/p2n1ppp/4q3/4p1B1/4P3/1Q6/PPP2PPP/2KR4 w k - 1 0")};
	// thread_sync sync = {.position = fen_to_position("r2qkb1r/pp2nppp/3p4/2pNN1B1/2BnP3/3P4/PPP2PPP/R2bK2R w KQkq - 1 0")};
	thread_sync sync = {.position = fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")};

	piece_color engine_color = piece_color::WHITE;
	
	seconds time_black{1 * 1s};
	seconds time_white{10 * 60s};

	steady_clock::time_point now = steady_clock::now();


	std::vector<position_times> old_positions;
	std::vector<position_times> new_positions;

	std::thread ai_move_thread;
	uint64_t possible_moves = 0;
	optional<board_pos> src_pos;

	if(sync.position.turn == engine_color)
		play_engine(sync, ai_move_thread);

	Canvas canvas{rend, digits_image, pieces_image};

	while (true) {
		if (time_white.count() <= 0 || time_black.count() <= 0) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Ended", "Game ended, time ran out", nullptr);
			exit(0);
		}

		if (duration_cast<seconds>(steady_clock::now() - now).count() >= 1) {
			sync.position.turn == piece_color::WHITE ? time_white-- : time_black--;
			now = steady_clock::now();
		}

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				exit(0);
			case SDL_MOUSEBUTTONDOWN: {
				if (event.button.button != SDL_BUTTON_LEFT || sync.is_thinking || sync.position.turn == engine_color)
					break;

				board_pos pos = {event.button.x, event.button.y};
				if(!pos.is_valid())
					break;
				
				if (!src_pos.has_value()) {
					std::lock_guard<std::mutex> lock(sync.mutex);
					optional<piece> src_piece = sync.position.board[pos.y][pos.x];
					if (src_piece.has_value() && sync.position.turn == src_piece->color) {
						src_pos = pos;
						possible_moves = sync.position.get_moves(pos);
					}
				} else {
					if (bitboard_get(possible_moves, pos)) {
						std::lock_guard<std::mutex> lock(sync.mutex);
						old_positions.push_back({sync.position, time_black, time_white});
						sync.position.make_move(pos, src_pos.value(), win);
						new_positions.clear();
						play_engine(sync, ai_move_thread);
					}

					possible_moves = 0;
					src_pos = std::nullopt;
				}
			} break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_LEFT:
					if(!sync.is_thinking && old_positions.size() > 0) {
						new_positions.push_back({sync.position, time_black, time_white});
						rollback_position(old_positions, possible_moves, src_pos, sync, time_black, time_white);
					}
					break;
				case SDLK_RIGHT:
					if (event.key.keysym.sym == SDLK_RIGHT && !sync.is_thinking && new_positions.size() > 0){
						old_positions.push_back({sync.position, time_black, time_white});
						rollback_position(new_positions, possible_moves, src_pos, sync, time_black, time_white);
					}
					break;
				case SDLK_F11:
					auto flag = SDL_GetWindowFlags(win);
					bool is_fullscreen = flag & SDL_WINDOW_FULLSCREEN;
					SDL_SetWindowFullscreen(win, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
				}
				break;
			default:
				break;
			}
		}
		// std::lock_guard<std::mutex> lock(sync.mutex);
		canvas.draw(time_black ,time_white, sync.position.board, possible_moves, sync.position.turn);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	IMG_Quit();
	SDL_Quit();
	return 0;
}