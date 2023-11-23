#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>

#include "../src/common.hpp"
#include "../src/draw.hpp"
#include "../src/transposition_table.hpp"
#include "../src/piece_goodness.hpp"
#include "../src/piece_moves.hpp"
#include "../src/position.hpp"
#include "../src/fen.hpp"


BOOST_AUTO_TEST_CASE(fen_test)
{
    Position pos = fen_to_position("r1bqkbnr/ppppp1pp/2n5/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    if(pos.en_passant.has_value())
        BOOST_TEST(pos.en_passant.value() == 5);
    else
        BOOST_FAIL("Failed to generate correct en passant value from fen");
}