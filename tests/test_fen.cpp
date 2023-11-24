#define BOOST_TEST_MODULE FenTests
#include <boost/test/included/unit_test.hpp>

#include "../src/common.hpp"
#include "../src/draw.hpp"
#include "../src/transposition_table.hpp"
#include "../src/piece_goodness.hpp"
#include "../src/piece_moves.hpp"
#include "../src/position.hpp"
#include "../src/fen.hpp"
static Position pos = fen_to_position("r1bqkbnr/ppppp1pp/2n5/4Pp2/8/8/PPPP1PPP/RNBQKBNR w Kkq f6 0 3");

BOOST_AUTO_TEST_CASE(fen_en_passant_test)
{
    if(pos.en_passant.has_value())
        BOOST_TEST(pos.en_passant.value() == 5);
    else
        BOOST_FAIL("Failed to generate correct en passant value from fen");

}
BOOST_AUTO_TEST_CASE(fen_castling_test) {
    BOOST_TEST(!pos.can_castle[int(piece_color::WHITE)][int(castle_side::LONG)]);
    BOOST_TEST(pos.can_castle[int(piece_color::WHITE)][int(castle_side::SHORT)]);
    BOOST_TEST(pos.can_castle[int(piece_color::BLACK)][int(castle_side::LONG)]);
    BOOST_TEST(pos.can_castle[int(piece_color::BLACK)][int(castle_side::SHORT)]);
}

BOOST_AUTO_TEST_CASE(fen_pieces_count_test) {
    int pieces_count = 0;
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            if(!pos.board[y][x].has_value())
                continue;
            pieces_count++;
        }
    }
    BOOST_TEST(pieces_count == 32);
}


BOOST_AUTO_TEST_CASE(fen_game_phase_test) {
    BOOST_TEST((pos.get_game_phase() == game_phase::OPENING));
}