struct thread_sync {
	Position position;
	std::mutex mutex;	
	std::atomic<bool> is_thinking;
};

void engine_thread_func(thread_sync *sync) {

	auto start = std::chrono::high_resolution_clock::now();

	move best_move = sync->position.ai_move();
	std::lock_guard<std::mutex> lock(sync->mutex);
	sync->position.make_move(best_move.dst_pos, best_move.src_pos, nullptr);
	sync->is_thinking = false;
	
	auto stop = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << '\n';
} 
void rollback_position(std::vector<Position>& last_positions, uint64_t& possible_moves, optional<board_pos>& src_pos, thread_sync& sync) {
	if(last_positions.size() > 0) {
		possible_moves = 0;
		src_pos = std::nullopt;
		sync.position = last_positions.back();
		last_positions.pop_back();
	}
}

void play_engine(thread_sync& sync, std::thread& ai_move_thread) {
	// ai_thread_func(&sync);
	sync.is_thinking = true;
	std::cout << "transposition size: " << transposition_table.size() << '\n';
	std::cout << "thinking.. \n";
	ai_move_thread = std::thread(engine_thread_func, &sync);							
	ai_move_thread.detach();
}