#include "common.hpp"
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
	
	transposition_table.init_table();
	SDL_Window* win = SDL_CreateWindow("chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SQUARE_SIZE * 10, SQUARE_SIZE * 8, 0);
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_Texture* pieces_image = IMG_LoadTexture(rend, "./sprites/pieces.png");
	SDL_Texture* digits_image = IMG_LoadTexture(rend, "./sprites/digits.png");

	if (!pieces_image || !digits_image) {
		messagebox_error("FAILED TO LOAD SPRITES", "Failed to load spritesheets");
		exit(1);
	}

	SDL_SetTextureScaleMode(digits_image, SDL_ScaleModeBest);
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_RenderSetLogicalSize(rend, 10, 8);
	SDL_SetTextureScaleMode(pieces_image, SDL_ScaleModeLinear);


	// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
	thread_sync sync = {.position = fen_to_position("8/1K4P1/8/8/8/8/k7/8 w - - 0 1")};
	// thread_sync sync = {.position = fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")};

	piece_color engine_color = piece_color::WHITE;
	
	std::chrono::seconds time_black{10 * 60s};
	std::chrono::seconds time_white{10 * 60s};
	auto now = std::chrono::steady_clock::now();

	SDL_ShowWindow(win);

	std::vector<position_times> old_positions;
	std::vector<position_times> new_positions;	
	std::thread ai_move_thread;
	uint64_t possible_moves = 0;
	optional<board_pos> src_pos;

	if(sync.position.turn == engine_color)
		play_engine(sync, ai_move_thread);

	while (true) {
		if (time_white.count() <= 0 || time_black.count() <= 0)
			exit(0);

		if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - now).count() >= 1) {
			sync.position.turn == piece_color::WHITE ? time_white-- : time_black--;
			now = std::chrono::steady_clock::now();
		}
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type){
			case SDL_QUIT:
				exit(0);
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT && !sync.is_thinking && sync.position.turn != engine_color) {
					board_pos pos = { event.button.x, event.button.y};
					if(!pos.is_valid())
						break;
					
					if (!src_pos.has_value())
					{
						std::lock_guard<std::mutex> lock(sync.mutex);
						if (sync.position.board[pos.y][pos.x].has_value())
						{
							if (sync.position.turn == sync.position.board[pos.y][pos.x]->color)
							{
								src_pos = pos;
								possible_moves = sync.position.get_moves(pos);
							}
						}
					}
					else {
						if (bitboard_get(possible_moves, pos))
						{
							std::lock_guard<std::mutex> lock(sync.mutex);
							old_positions.push_back({sync.position, time_black, time_white});
							sync.position.make_move(pos, src_pos.value(), win);
							new_positions.clear();
							// position.ai_move(piece_color(!bool(position.turn)));
							play_engine(sync, ai_move_thread);

							//turn = piece_color(!bool(turn));
						}

						possible_moves = 0;
						src_pos = std::nullopt;
					}
				}
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_LEFT && !sync.is_thinking && old_positions.size() > 0) {
					new_positions.push_back({sync.position, time_black, time_white});
					rollback_position(old_positions, possible_moves, src_pos, sync, time_black, time_white);
				} else if (event.key.keysym.sym == SDLK_RIGHT && !sync.is_thinking && new_positions.size() > 0){
					old_positions.push_back({sync.position, time_black, time_white});
					rollback_position(new_positions, possible_moves, src_pos, sync, time_black, time_white);
				}
				break;
			}
		}
		// std::lock_guard<std::mutex> lock(sync.mutex);
		draw(time_black ,time_white , rend, sync.position.board, pieces_image, digits_image,possible_moves, sync.is_thinking);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	IMG_Quit();
	SDL_Quit();
	return 0;
}