#include "fen.hpp"


struct thread_sync {
	Position position;
	SDL_AudioDeviceID audio_device;
	uint8_t* audio_buffer;
	uint32_t audio_size;
	std::mutex mutex;	
	std::atomic<bool> is_thinking;
};

struct position_times {
	Position position;
	seconds time_black;
	seconds time_white;
};

void engine_thread_func(thread_sync *sync) {

	auto start = high_resolution_clock::now();

	move best_move = sync->position.get_best_moves();
	std::lock_guard<std::mutex> lock(sync->mutex);
	SDL_QueueAudio(sync->audio_device, sync->audio_buffer, sync->audio_size);
	sync->position.make_move(best_move.dst_pos, best_move.src_pos, nullptr);
	sync->is_thinking = false;
	
	auto stop = high_resolution_clock::now();
	std::cout << duration_cast<milliseconds>(stop - start).count() << '\n';
} 

void rollback_position(
	std::vector<position_times>& last_positions, 
	uint64_t& possible_moves, optional<board_pos>& src_pos, 
	thread_sync& sync, 
	seconds& time_black, 
	seconds& time_white
) {
	if(last_positions.size() > 0) {
		possible_moves = 0;
		src_pos = std::nullopt;
		auto back = last_positions.back();
		sync.position = back.position;
		time_white = back.time_white;
		time_black = back.time_black;
		last_positions.pop_back();
	}
}

void play_engine(thread_sync& sync, std::thread& ai_move_thread, auto audio_device, auto audio_buffer, auto audio_size) {
	// ai_thread_func(&sync);
	sync.is_thinking = true;
	sync.audio_device = audio_device;
	sync.audio_buffer = audio_buffer;
	sync.audio_size = audio_size;
	std::cout << "transposition size: " << transposition_table.table.size() << '\n';
	std::cout << "thinking.. \n";
	ai_move_thread = std::thread(engine_thread_func, &sync);
	ai_move_thread.detach();
}
