#include "common.h"
#include "piece_goodness.h"
#include "position.h"
#include "fen.h"

struct thread_sync {
	Position position;
	std::mutex mutex;	
	std::atomic<bool> is_thinking;
};

void ai_thread_func(thread_sync *sync) {

	auto start = std::chrono::high_resolution_clock::now();

	move best_move = sync->position.ai_move();
	std::lock_guard<std::mutex> lock(sync->mutex);
	sync->position.make_move(best_move.dst_pos, best_move.src_pos, nullptr);
	sync->is_thinking = false;
	
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << '\n';
} 

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);


	static constexpr std::array<piece_type, 8> start_row = {
		piece_type::ROOK, piece_type::KNIGHT, piece_type::BISHOP, piece_type::QUEEN,
		piece_type::KING, piece_type::BISHOP, piece_type::KNIGHT, piece_type::ROOK,
	};

	auto make_row = [](piece_color color) {
		array<optional<piece>, 8> row;
		for (int i = 0; i < 8; i++)
		{
			row[i] = piece{ color, start_row[i] };
		}
		return row;
	};

	std::thread ai_move_thread;

	auto make_pawn_row = [](piece_color color) {
		array<optional<piece>, 8> row;	
		for (auto &i : row)
			i = piece{ color, piece_type::PAWN};
		return row;
	};
	// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
	// thread_sync sync = {.position = fen_to_position("6r1/8/1k6/8/8/8/8/1K6 w - - 0 1")};
	thread_sync sync = {.position = fen_to_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")};
	Position last_position = sync.position;

	auto win = SDL_CreateWindow("chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		SQUARE_SIZE * 8, SQUARE_SIZE * 8, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
	auto rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_RenderSetLogicalSize(rend, 8, 8);
	pieces_image = IMG_LoadTexture(rend, "pieces.png");
	SDL_SetTextureScaleMode(pieces_image, SDL_ScaleModeLinear);

	if (!pieces_image) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "failed to load image", "failed to load piece image", nullptr);
		return 1;
	}

	SDL_ShowWindow(win);
	bool run = true;
	uint64_t possible_moves = 0;
	optional<board_pos> src_pos;

	while (run) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type){
			case SDL_QUIT:
				run = false;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT && !sync.is_thinking) {
					board_pos pos = { event.button.x, event.button.y};
					
					if (!src_pos.has_value())
					{
						std::lock_guard<std::mutex> lock(sync.mutex);
						if (sync.position.board[pos.y][pos.x].has_value())
						{
							if (sync.position.turn == sync.position.board[pos.y][pos.x].value().color)
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
							last_position = sync.position;
							sync.position.make_move(pos, src_pos.value(), win);
							// position.ai_move(piece_color(!bool(position.turn)));
							
							sync.is_thinking = true;

							// ai_thread_func(&sync);

							ai_move_thread = std::thread(ai_thread_func, &sync);							
							ai_move_thread.detach();

							//turn = piece_color(!bool(turn));
						}

						possible_moves = 0;
						src_pos = std::nullopt;
					
					}
				}
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_LEFT && !sync.is_thinking)
				{
					sync.position = last_position;
					//position.turn = piece_color(!bool(position.turn));
				}
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