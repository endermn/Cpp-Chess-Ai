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