#include "common.h"
#include "draw.h"
#include "transposition_table.h"
#include "piece_goodness.h"
#include "piece_moves.h"
#include "position.h"
#include "fen.h"
#include "threads.h"



int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);

	transposition_table.init_table();


	SDL_Window* win = SDL_CreateWindow("chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SQUARE_SIZE * 8, SQUARE_SIZE * 8, 0);
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);

	pieces_image = IMG_LoadTexture(rend, "pieces.png");

	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_RenderSetLogicalSize(rend, 8, 8);
	SDL_SetTextureScaleMode(pieces_image, SDL_ScaleModeLinear);

	if (!pieces_image) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "failed to load image", "failed to load piece image", nullptr);
		return 1;
	}

	// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
	// thread_sync sync = {.position = fen_to_position("6r1/8/1k6/8/8/8/8/1K6 w - - 0 1")};
	thread_sync sync = {.position = fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")};

	piece_color engine_color = piece_color::BLACK;
	

	SDL_ShowWindow(win);

	std::vector<Position> last_positions;
	std::thread ai_move_thread;
	uint64_t possible_moves = 0;
	optional<board_pos> src_pos;

	if(sync.position.turn == engine_color)
		play_engine(sync, ai_move_thread);

	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type){
			case SDL_QUIT:
				exit(0);
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT && !sync.is_thinking && sync.position.turn != engine_color) {
					board_pos pos = { event.button.x, event.button.y};
					
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
							last_positions.push_back(sync.position);
							sync.position.make_move(pos, src_pos.value(), win);
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
				if (event.key.keysym.sym == SDLK_LEFT && !sync.is_thinking)
					rollback_position(last_positions, possible_moves, src_pos, sync);
				break;
			}
		}
		// std::lock_guard<std::mutex> lock(sync.mutex);
		draw(rend, sync.position.board, possible_moves);
	}

	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	IMG_Quit();
	SDL_Quit();
	return 0;
}