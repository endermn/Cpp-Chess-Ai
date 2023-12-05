struct thread_sync {
	Position position;
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

	Move best_move = sync->position.ai_move();
	std::lock_guard<std::mutex> lock(sync->mutex);
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

void play_engine(thread_sync& sync, std::thread& ai_move_thread) {
	// ai_thread_func(&sync);
	sync.is_thinking = true;
	std::cout << "transposition size: " << transposition_table.table.size() << '\n';
	std::cout << "thinking.. \n";
	ai_move_thread = std::thread(engine_thread_func, &sync);							
	ai_move_thread.detach();
}