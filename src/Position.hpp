#include "PieceChecks.hpp"

class Position : public PieceChecks {
public:
  // Returns the absolute evaluation of the current chess position as a floating
  // point
  float evaluate() const {
    float score = 0;

    game_phase phase = get_game_phase();

    std::array<optional<board_pos>, 2> king_positions = {std::nullopt,
                                                         std::nullopt};

    for (int i = 0; i < 64; i++) {
      int y = i / 8;
      int x = i % 8;

      if (!board[y][x].has_value())
        continue;

      piece src_piece = board[y][x].value();
      int color_value = get_color_value(src_piece.color);

      switch (src_piece.type) {
      case piece_type::KING:
        for (int i = -1; i < 2; i++) {
          board_pos dst_pos = {.x = x + i, .y = y - color_value};
          if (!dst_pos.is_valid())
            continue;
          if (board[dst_pos.y][dst_pos.x].has_value() &&
              board[dst_pos.y][dst_pos.x]->color == src_piece.color)
            score += color_value * 0.1;
          if (!board[dst_pos.y][x].has_value() ||
              board[dst_pos.y][x]->color != src_piece.color)
            score -= color_value * 0.1;
        }
        king_positions[src_piece.color == piece_color::BLACK ? 1 : 0] =
            board_pos{x, y};
        break;
      case piece_type::PAWN:
        if (x < 7 && x > 0) {
          if (!(board[y - color_value][x + 1]->type == piece_type::PAWN ||
                board[y - color_value][x - 1]->type == piece_type::PAWN))
            score -= color_value * 0.01f;
          else
            score += color_value * 0.01f;
        }
        if (board[y - color_value][x]->type == piece_type::PAWN &&
            board[y - color_value][x]->color == board[y][x]->color)
          score += color_value * 0.01f;
        break;
      default:
        break;
      }

      int goodness_y = src_piece.color == piece_color::BLACK ? 7 - y : y;
      float piece_goodness =
          piece_goodnesses[phase == game_phase::ENDGAME][int(src_piece.type)]
                          [goodness_y][x];
      score += color_value * 0.01 * piece_goodness;

      score += color_value * get_piece_value(src_piece.type);
    }

    if (phase == game_phase::ENDGAME) {
      std::array<int, 2> mobility_score = targetted_squares_count();
      score += 0.001 * (mobility_score[0] - mobility_score[1]);
      float delta_x = king_positions[0]->x - king_positions[1]->x;
      float delta_y = king_positions[0]->y - king_positions[1]->y;
      score *= 3.f / sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + 1;
    }

    return score;
  }

  bool search_at_depth(std::vector<move> &old_best_moves, int depth,
                       steady_clock::time_point then) const {
    std::vector<move> best_moves;
    size_t best_moves_size = 0;
    float max = INT_MIN;
    for (int i = 0; i < 64; i++) {
      int y = i / 8;
      int x = i % 8;
      if (!board[y][x].has_value() || board[y][x].value().color != turn)
        continue;
      if (old_best_moves.size() > 0 &&
          duration_cast<seconds>(steady_clock::now() - then).count() >= 3)
        return true;

      Bitboard move_bitboard = get_moves(board_pos{x, y});

      while (move_bitboard.bits != 0) {
        unsigned long index = std::countr_zero(move_bitboard.bits);
        move_bitboard.bits &= move_bitboard.bits - 1;
        board_pos dst_pos = {int(index) % 8, int(index) / 8};
        Position new_position = *this;
        new_position.make_move(dst_pos, {x, y}, nullptr);

        if (new_position.is_under_check(turn))
          continue;

        float evaluation =
            -new_position.negamax(-1e9, 1e9, depth, piece_color(!bool(turn)));
        std::cout << depth << '\n';

        if (evaluation > max) {
          max = evaluation;
          best_moves.clear();
        }

        if (evaluation >= max) {
          best_moves.push_back({dst_pos, {x, y}});
          best_moves_size = std::max(best_moves.size(), best_moves_size);
        }
      }
    }

    old_best_moves = best_moves;
    return false;
  }

  move get_best_moves() const {
    std::vector<move> best_moves;
    best_moves.reserve(4);
    int current_depth = 2;
    float last_eval;
    auto then = steady_clock::now();
    while (true) {
      if (search_at_depth(best_moves, current_depth, then))
        break;
      current_depth++;
    }
    // search_at_depth(best_moves, 4, then);
    // TODO: On checkmate implementation check if best_moves.size() > 0
    return best_moves[best_moves.size() == 1 ? 0
                                             : std::rand() % best_moves.size()];
  }

  void order_moves(std::vector<move> &captures) const {
    for (move &current_move : captures) {
      float move_score = 0;
      piece_type move_piece_type =
          board[current_move.src_pos.y][current_move.src_pos.x]->type;

      if (board[current_move.dst_pos.y][current_move.dst_pos.x].has_value()) {
        piece_type capture_piece_type =
            board[current_move.dst_pos.y][current_move.dst_pos.x]->type;
        move_score += 10 * (get_piece_value(capture_piece_type) -
                            get_piece_value(move_piece_type));

        if (capture_piece_type == piece_type::KING)
          move_score += 1'000'000;
      }

      if (move_piece_type == piece_type::PAWN &&
          (current_move.dst_pos.y == 0 || current_move.dst_pos.y == 7)) {
        move_score += 10 * get_piece_value(piece_type::QUEEN);
      }
      current_move.score = move_score;
    }
    // captures.erase(std::remove(captures.begin(), captures.end(), 0),
    // captures.end());
    std::sort(captures.begin(), captures.end(),
              [](const move &a, const move &b) { return a.score > b.score; });
  }

  float search_captures(float alpha, float beta, int depth) const {
    float score = get_color_value(turn) * evaluate();
    if (score >= beta)
      return beta;
    alpha = std::max(score, alpha);
    if (depth == 0)
      return alpha;
    std::vector<move> captures;

    for (int i = 0; i < 64; i++) {
      int y = i / 8;
      int x = i % 8;
      if (!board[y][x].has_value())
        continue;

      Bitboard move_bitboard = get_moves(board_pos{x, y});
      while (move_bitboard.bits != 0) {
        unsigned long index = std::countr_zero(move_bitboard.bits);
        move_bitboard.bits &= move_bitboard.bits - 1;
        board_pos dst_pos = {int(index) % 8, int(index) / 8};
        if (board[dst_pos.y][dst_pos.x].has_value())
          captures.push_back({dst_pos, {x, y}, true});
      }
    }

    order_moves(captures);
    for (int i = 0; i < captures.size(); i++) {
      Position new_position = *this;
      new_position.make_move(captures[i].dst_pos, captures[i].src_pos, nullptr);
      score = -new_position.search_captures(-beta, -alpha, depth - 1);
      if (score >= beta)
        return beta;
      alpha = std::max(score, alpha);
    }

    return alpha;
  }

  float evaluate_negamax(float alpha, float beta, int depth) {
    for (int i = 0; i < 64; i++) {
      int y = i / 8;
      int x = i % 8;
      if (!board[y][x].has_value() || board[y][x]->color != turn)
        continue;

      Bitboard move_bitboard = get_moves(board_pos{x, y});
      while (move_bitboard.bits != 0) {
        unsigned long index = std::countr_zero(move_bitboard.bits);
        move_bitboard.bits &= move_bitboard.bits - 1;
        board_pos dst_pos = {int(index) % 8, int(index) / 8};

        if (board[dst_pos.y][dst_pos.x].has_value() &&
            board[dst_pos.y][dst_pos.x]->type == piece_type::KING)
          return 1'000'000.f + depth * 1'000'000.f;

        Position new_position = *this;
        new_position.make_move(dst_pos, board_pos{x, y}, nullptr);
        float score = -new_position.negamax(-beta, -alpha, depth - 1,
                                            piece_color(!bool(turn)));
        if (score >= beta)
          return beta;
        alpha = std::max(alpha, score);
      }
    }

    return alpha;
  }

  float negamax(float alpha, float beta, int depth, piece_color turn) {
    float best_score = -1'000'000.f;
    uint64_t current_hash = hash();

    if (auto it = transposition_table.table.find(current_hash);
        it != transposition_table.table.end() && it->second.depth >= depth)
      return it->second.evaluation;

    float evaluation = depth == 0 ? search_captures(alpha, beta, 5)
                                  : evaluate_negamax(alpha, beta, depth);
    transposition_table.table[current_hash] = {.evaluation = evaluation,
                                               .depth = depth};

    return evaluation;
  }
};
