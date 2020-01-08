//
// Test Minimal Tuple Library
// by BOT Man, 2017
//

#include "tuple.h"  // test dependency

#include <functional>
#include <string>
#include <tuple>

#include <assert.h>
#include <stddef.h>

// tuple in libc++:
// https://llvm.org/svn/llvm-project/libcxx/trunk/include/tuple

// tuple in libstdc++:
// https://gcc.gnu.org/svn/gcc/trunk/libstdc++-v3/include/std/tuple

// More about universal reference:
// https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers

// More about 'const &&':
// https://codesynthesis.com/~boris/blog/2012/07/24/const-rvalue-references

int main (int argc, char *argv[])
{
    // helpers

    using namespace std::string_literals;

    int ref_int = 1;
    const int cref_int = 1;
    auto rValueGen = [] (const auto &val) -> auto { return val; };
    auto constRValueGen = [] (const auto &val) -> const auto { return val; };

    /// tuple

    // default ctor
    std::tuple<> std_empty_default_ctor_tuple;
    bot::tuple<> bot_empty_default_ctor_tuple;

    std::tuple<int> std_single_default_ctor_tuple;
    bot::tuple<int> bot_single_default_ctor_tuple;

    std::tuple<int, double> std_double_default_ctor_tuple;
    bot::tuple<int, double> bot_double_default_ctor_tuple;

    std::tuple<int, double, std::string> std_triple_default_ctor_tuple;
    bot::tuple<int, double, std::string> bot_triple_default_ctor_tuple;

    //std::tuple<int &> std_ref_default_ctor_tuple;
    //bot::tuple<int &> bot_ref_default_ctor_tuple;

    // value-direct ctor
    std::tuple<> std_empty_tuple {};
    bot::tuple<> bot_empty_tuple {};

    std::tuple<int> std_single_tuple (1);
    bot::tuple<int> bot_single_tuple (1);

    std::tuple<int, double> std_double_tuple (2, 2.2);
    bot::tuple<int, double> bot_double_tuple (2, 2.2);

    std::tuple<int, double, std::string> std_triple_tuple (3, 3.3, "str"s);
    bot::tuple<int, double, std::string> bot_triple_tuple (3, 3.3, "str"s);

    std::tuple<int &> std_ref_tuple (ref_int);
    bot::tuple<int &> bot_ref_tuple (ref_int);

    std::tuple<const int &> std_cref_tuple (cref_int);
    bot::tuple<const int &> bot_cref_tuple (cref_int);

    // value-convert ctor
    std::tuple<int &&> std_rref_tuple (rValueGen (1));
    bot::tuple<int &&> bot_rref_tuple (rValueGen (1));

    std::tuple<const int &&> std_crref_tuple (constRValueGen (1));
    bot::tuple<const int &&> bot_crref_tuple (constRValueGen (1));

    std::tuple<int &&> std_rref_conv_tuple (1);
    bot::tuple<int &&> bot_rref_conv_tuple (1);

    std::tuple<std::string, std::string> std_value_conv_tuple ("s1", "s2");
    bot::tuple<std::string, std::string> bot_value_conv_tuple ("s1", "s2");

    // tuple-convert ctor
    std::tuple<int16_t> std_conv_move_ctor_tuple (std::tuple<int8_t> (2));
    bot::tuple<int16_t> bot_conv_move_ctor_tuple (bot::tuple<int8_t> (2));
    std::tuple<int32_t> std_conv_copy_ctor_tuple (std_conv_move_ctor_tuple);
    bot::tuple<int32_t> bot_conv_copy_ctor_tuple (bot_conv_move_ctor_tuple);

    std::tuple<int16_t> std_conv_move_assign_tuple;
    bot::tuple<int16_t> bot_conv_move_assign_tuple;
    std::tuple<int32_t> std_conv_copy_assign_tuple;
    bot::tuple<int32_t> bot_conv_copy_assign_tuple;

    std_conv_move_assign_tuple = std::tuple<int8_t> (3);
    bot_conv_move_assign_tuple = bot::tuple<int8_t> (3);
    std_conv_copy_assign_tuple = std_conv_move_assign_tuple;
    bot_conv_copy_assign_tuple = bot_conv_move_assign_tuple;

    // move & copy ctor / assignment
    std::tuple<int> std_move_ctor_tuple (std::tuple<int> (2));
    bot::tuple<int> bot_move_ctor_tuple (bot::tuple<int> (2));
    std::tuple<int> std_copy_ctor_tuple (std_move_ctor_tuple);
    bot::tuple<int> bot_copy_ctor_tuple (bot_move_ctor_tuple);

    std::tuple<int> std_move_assign_tuple;
    bot::tuple<int> bot_move_assign_tuple;
    std::tuple<int> std_copy_assign_tuple;
    bot::tuple<int> bot_copy_assign_tuple;

    std_move_assign_tuple = std::tuple<int> (3);
    bot_move_assign_tuple = bot::tuple<int> (3);
    std_copy_assign_tuple = std_move_assign_tuple;
    bot_copy_assign_tuple = bot_move_assign_tuple;

    std::tuple<int &> std_move_ctor_ref_tuple { std::tuple<int &> (ref_int) };
    bot::tuple<int &> bot_move_ctor_ref_tuple { bot::tuple<int &> (ref_int) };
    std::tuple<int &> std_copy_ctor_ref_tuple (std_move_ctor_ref_tuple);
    bot::tuple<int &> bot_copy_ctor_ref_tuple (bot_move_ctor_ref_tuple);

    std::tuple<int &> std_move_assign_ref_tuple (ref_int);
    bot::tuple<int &> bot_move_assign_ref_tuple (ref_int);
    std::tuple<int &> std_copy_assign_ref_tuple (ref_int);
    bot::tuple<int &> bot_copy_assign_ref_tuple (ref_int);

    std_move_assign_ref_tuple = std::tuple<int &> (ref_int);
    bot_move_assign_ref_tuple = bot::tuple<int &> (ref_int);
    std_copy_assign_ref_tuple = std_move_assign_ref_tuple;
    bot_copy_assign_ref_tuple = bot_move_assign_ref_tuple;

    /// operators

    // empty
    assert (std_empty_tuple == std::tuple<> ());
    assert (bot_empty_tuple == bot::tuple<> ());

    assert (!(std_empty_tuple < std::tuple<> ()));
    assert (!(bot_empty_tuple < bot::tuple<> ()));

    // equal
    assert (std_double_tuple == (std::tuple<int, double> (2, 2.2)));
    assert (bot_double_tuple == (bot::tuple<int, double> (2, 2.2)));

    assert (std_double_tuple != (std::tuple<int, double> (0, 0.0)));
    assert (bot_double_tuple != (bot::tuple<int, double> (0, 0.0)));

    // less
    assert (std_double_tuple < (std::tuple<int, double> (3, 2.2)));
    assert (bot_double_tuple < (bot::tuple<int, double> (3, 2.2)));

    assert (std_double_tuple < (std::tuple<int, double> (2, 3.3)));
    assert (bot_double_tuple < (bot::tuple<int, double> (2, 3.3)));

    assert (std_double_tuple > (std::tuple<int, double> (1, 2.2)));
    assert (bot_double_tuple > (bot::tuple<int, double> (1, 2.2)));

    assert (std_double_tuple > (std::tuple<int, double> (2, 1.1)));
    assert (bot_double_tuple > (bot::tuple<int, double> (2, 1.1)));

    assert (std_double_tuple <= (std::tuple<int, double> (2, 2.2)));
    assert (bot_double_tuple <= (bot::tuple<int, double> (2, 2.2)));

    assert (std_double_tuple >= (std::tuple<int, double> (2, 2.2)));
    assert (bot_double_tuple >= (bot::tuple<int, double> (2, 2.2)));

    assert (std_double_tuple <= (std::tuple<int, double> (3, 2.2)));
    assert (bot_double_tuple <= (bot::tuple<int, double> (3, 2.2)));

    assert (std_double_tuple <= (std::tuple<int, double> (2, 3.3)));
    assert (bot_double_tuple <= (bot::tuple<int, double> (2, 3.3)));

    assert (std_double_tuple >= (std::tuple<int, double> (1, 2.2)));
    assert (bot_double_tuple >= (bot::tuple<int, double> (1, 2.2)));

    assert (std_double_tuple >= (std::tuple<int, double> (2, 1.1)));
    assert (bot_double_tuple >= (bot::tuple<int, double> (2, 1.1)));

    /// tuple_size

    static_assert(
        std::tuple_size<decltype (std_empty_tuple)>::value ==
        bot::tuple_size_v<decltype (bot_empty_tuple)>, "");

    static_assert(
        std::tuple_size<decltype (std_single_tuple)>::value ==
        bot::tuple_size_v<decltype (bot_single_tuple)>, "");

    static_assert(
        std::tuple_size<decltype (std_double_tuple)>::value ==
        bot::tuple_size_v<decltype (bot_double_tuple)>, "");

    static_assert(
        std::tuple_size<decltype (std_triple_tuple)>::value ==
        bot::tuple_size_v<decltype (bot_triple_tuple)>, "");

    /// tuple_element

    // empty
    //static_assert(std::is_same<
    //    std::tuple_element_t<0, decltype (std_empty_tuple)>,
    //    bot::tuple_element_t<0, decltype (bot_empty_tuple)>
    //>::value, "");

    // normal
    static_assert(std::is_same<
        std::tuple_element_t<0, decltype (std_single_tuple)>,
        bot::tuple_element_t<0, decltype (bot_single_tuple)>
    >::value, "");

    static_assert(std::is_same<
        std::tuple_element_t<0, decltype (std_double_tuple)>,
        bot::tuple_element_t<0, decltype (bot_double_tuple)>
    >::value, "");
    static_assert(std::is_same<
        std::tuple_element_t<1, decltype (std_double_tuple)>,
        bot::tuple_element_t<1, decltype (bot_double_tuple)>
    >::value, "");

    static_assert(std::is_same<
        std::tuple_element_t<0, decltype (std_triple_tuple)>,
        bot::tuple_element_t<0, decltype (bot_triple_tuple)>
    >::value, "");
    static_assert(std::is_same<
        std::tuple_element_t<1, decltype (std_triple_tuple)>,
        bot::tuple_element_t<1, decltype (bot_triple_tuple)>
    >::value, "");
    static_assert(std::is_same<
        std::tuple_element_t<2, decltype (std_triple_tuple)>,
        bot::tuple_element_t<2, decltype (bot_triple_tuple)>
    >::value, "");

    static_assert(std::is_same<
        std::tuple_element_t<0, decltype (std_ref_tuple)>,
        bot::tuple_element_t<0, decltype (bot_ref_tuple)>
    >::value, "");
    static_assert(std::is_same<
        std::tuple_element_t<0, decltype (std_cref_tuple)>,
        bot::tuple_element_t<0, decltype (bot_cref_tuple)>
    >::value, "");

    // const
    const auto std_const_single_tuple = std_single_tuple;
    const auto bot_const_single_tuple = bot_single_tuple;

    static_assert(std::is_same<
        std::tuple_element_t<0, decltype (std_const_single_tuple)>,
        bot::tuple_element_t<0, decltype (bot_const_single_tuple)>
    >::value, "");

    /// get (by index)

    // empty
    //std::get<0> (std_empty_tuple);
    //bot::get<0> (bot_empty_tuple);

    // normal
    assert (
        std::get<0> (std_single_tuple) ==
        bot::get<0> (bot_single_tuple)
    );

    assert (
        std::get<0> (std_double_tuple) ==
        bot::get<0> (bot_double_tuple)
    );
    assert (
        std::get<1> (std_double_tuple) ==
        bot::get<1> (bot_double_tuple)
    );

    assert (
        std::get<0> (std_triple_tuple) ==
        bot::get<0> (bot_triple_tuple)
    );
    assert (
        std::get<1> (std_triple_tuple) ==
        bot::get<1> (bot_triple_tuple)
    );
    assert (
        std::get<2> (std_triple_tuple) ==
        bot::get<2> (bot_triple_tuple)
    );

    // ref-value
    ref_int = 2;
    assert (
        std::get<0> (std_ref_tuple) ==
        bot::get<0> (bot_ref_tuple)
    );

    std::get<0> (std_ref_tuple) = 3;
    assert (
        std::get<0> (std_ref_tuple) ==
        bot::get<0> (bot_ref_tuple)
    );

    bot::get<0> (bot_ref_tuple) = 4;
    assert (
        std::get<0> (std_ref_tuple) ==
        bot::get<0> (bot_ref_tuple)
    );

    std::get<0> (std::tuple<int &> (ref_int)) = 5;
    assert (
        std::get<0> (std_ref_tuple) ==
        bot::get<0> (bot_ref_tuple)
    );

    bot::get<0> (bot::tuple<int &> (ref_int)) = 6;
    assert (
        std::get<0> (std_ref_tuple) ==
        bot::get<0> (bot_ref_tuple)
    );

    // cref
    assert (
        std::get<0> (std_const_single_tuple) ==
        bot::get<0> (bot_const_single_tuple)
    );

    // rref
    assert (
        std::get<0> (std::make_tuple (5)) ==
        bot::get<0> (bot::make_tuple (5))
    );

    // crref
    assert (
        std::get<0> (constRValueGen (std_single_tuple)) ==
        bot::get<0> (constRValueGen (bot_single_tuple))
    );

    /// get (by type)

    // empty
    //std::get<int> (std::tuple<> ());
    //bot::get<int> (bot::tuple<> ());

    // normal
    assert (
        std::get<int> (std_triple_tuple) ==
        bot::get<int> (bot_triple_tuple)
    );
    assert (
        std::get<double> (std_triple_tuple) ==
        bot::get<double> (bot_triple_tuple)
    );
    assert (
        std::get<std::string> (std_triple_tuple) ==
        bot::get<std::string> (bot_triple_tuple)
    );

    // ref-value
    ref_int = 2;
    assert (
        std::get<int &> (std_ref_tuple) ==
        bot::get<int &> (bot_ref_tuple)
    );

    std::get<int &> (std_ref_tuple) = 3;
    assert (
        std::get<int &> (std_ref_tuple) ==
        bot::get<int &> (bot_ref_tuple)
    );

    bot::get<int &> (bot_ref_tuple) = 4;
    assert (
        std::get<int &> (std_ref_tuple) ==
        bot::get<int &> (bot_ref_tuple)
    );

    std::get<int &> (std::tuple<int &> (ref_int)) = 5;
    assert (
        std::get<int &> (std_ref_tuple) ==
        bot::get<int &> (bot_ref_tuple)
    );

    bot::get<int &> (bot::tuple<int &> (ref_int)) = 6;
    assert (
        std::get<int &> (std_ref_tuple) ==
        bot::get<int &> (bot_ref_tuple)
    );

    // missing or duplicate
    //std::get<double> (std::tuple<int, int> (1, 2));
    //bot::get<double> (bot::tuple<int, int> (1, 2));
    //std::get<int> (std::tuple<int, int> (1, 2));
    //bot::get<int> (bot::tuple<int, int> (1, 2));

    // cref
    assert (
        std::get<int> (std_const_single_tuple) ==
        bot::get<int> (bot_const_single_tuple)
    );

    // rref
    assert (
        std::get<int> (std::make_tuple (5)) ==
        bot::get<int> (bot::make_tuple (5))
    );

    // crref
    assert (
        std::get<int> (constRValueGen (std_single_tuple)) ==
        bot::get<int> (constRValueGen (bot_single_tuple))
    );

    /// swap

    std::tuple<int> std_tuple_swap_1 (1);
    bot::tuple<int> bot_tuple_swap_1 (1);
    std::tuple<int> std_tuple_swap_2 (2);
    bot::tuple<int> bot_tuple_swap_2 (2);

    std_tuple_swap_1.swap (std_tuple_swap_2);
    bot_tuple_swap_1.swap (bot_tuple_swap_2);

    assert (
        std::get<0> (std_tuple_swap_1) ==
        bot::get<0> (bot_tuple_swap_1)
    );
    assert (
        std::get<0> (std_tuple_swap_2) ==
        bot::get<0> (bot_tuple_swap_2)
    );

    std::swap (std_tuple_swap_1, std_tuple_swap_2);
    bot::swap (bot_tuple_swap_1, bot_tuple_swap_2);

    assert (
        std::get<0> (std_tuple_swap_1) ==
        bot::get<0> (bot_tuple_swap_1)
    );
    assert (
        std::get<0> (std_tuple_swap_2) ==
        bot::get<0> (bot_tuple_swap_2)
    );

    /// make_tuple

    auto std_empty_made_tuple = std::make_tuple ();
    auto bot_empty_made_tuple = bot::make_tuple ();

    auto std_single_made_tuple = std::make_tuple (1);
    auto bot_single_made_tuple = bot::make_tuple (1);

    auto std_double_made_tuple = std::make_tuple (2, 2.2);
    auto bot_double_made_tuple = bot::make_tuple (2, 2.2);

    auto std_triple_made_tuple = std::make_tuple (3, 3.3, "str"s);
    auto bot_triple_made_tuple = bot::make_tuple (3, 3.3, "str"s);

    auto std_ref_made_tuple = std::make_tuple (std::ref (ref_int));
    auto bot_ref_made_tuple = bot::make_tuple (std::ref (ref_int));

    auto std_non_ref_made_tuple = std::make_tuple (ref_int);
    auto bot_non_ref_made_tuple = bot::make_tuple (ref_int);

    static_assert(std::is_same<
        decltype(std_empty_made_tuple),
        std::tuple<>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_empty_made_tuple),
        bot::tuple<>
    >::value, "");

    static_assert(std::is_same<
        decltype(std_single_made_tuple),
        std::tuple<int>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_single_made_tuple),
        bot::tuple<int>
    >::value, "");

    static_assert(std::is_same<
        decltype(std_double_made_tuple),
        std::tuple<int, double>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_double_made_tuple),
        bot::tuple<int, double>
    >::value, "");

    static_assert(std::is_same<
        decltype(std_triple_made_tuple),
        std::tuple<int, double, std::string>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_triple_made_tuple),
        bot::tuple<int, double, std::string>
    >::value, "");

    static_assert(std::is_same<
        decltype(std_ref_made_tuple),
        std::tuple<int &>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_ref_made_tuple),
        bot::tuple<int &>
    >::value, "");

    static_assert(std::is_same<
        decltype(std_non_ref_made_tuple),
        std::tuple<int>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_non_ref_made_tuple),
        bot::tuple<int>
    >::value, "");

    // check ref made
    ref_int = 2;
    assert (
        std::get<0> (std_ref_made_tuple) ==
        bot::get<0> (bot_ref_made_tuple)
    );

    std::get<0> (std_ref_made_tuple) = 3;
    assert (
        std::get<0> (std_ref_made_tuple) ==
        bot::get<0> (bot_ref_made_tuple)
    );

    bot::get<0> (bot_ref_made_tuple) = 4;
    assert (
        std::get<0> (std_ref_made_tuple) ==
        bot::get<0> (bot_ref_made_tuple)
    );

    // check non ref made
    ref_int = 2;
    assert (
        std::get<0> (std_non_ref_made_tuple) ==
        bot::get<0> (bot_non_ref_made_tuple)
    );

    std::get<0> (std_non_ref_made_tuple) = 3;
    assert (
        std::get<0> (std_non_ref_made_tuple) !=
        bot::get<0> (bot_non_ref_made_tuple)
    );

    bot::get<0> (bot_non_ref_made_tuple) = 4;
    assert (
        std::get<0> (std_non_ref_made_tuple) !=
        bot::get<0> (bot_non_ref_made_tuple)
    );

    /// tie & ignore

    int std_int, bot_int;
    double std_double, bot_double;
    std::string std_string, bot_string;

    // tie-ref
    std::tie (std_int, std_double, std_string) = std_triple_tuple;
    bot::tie (bot_int, bot_double, bot_string) = bot_triple_tuple;

    // 0 -> 3
    assert (std_int == bot_int);
    assert (std_double == bot_double);
    assert (std_string == bot_string);

    // tie-rref
    std::tie (std_int, std_double, std_string) = std::make_tuple (1, .1, ""s);
    bot::tie (bot_int, bot_double, bot_string) = bot::make_tuple (1, .1, ""s);

    // 3 -> 1
    assert (std_int == bot_int);
    assert (std_double == bot_double);
    assert (std_string == bot_string);

    // tie-ignore
    std::tie (std_int, std::ignore) = std::make_tuple (2, ""s);
    bot::tie (bot_int, bot::ignore) = bot::make_tuple (2, ""s);

    // 1 -> 2
    assert (std_int == bot_int);

    // check types
    static_assert (std::is_same<
        decltype(std::tie (std_int, std_double, std_string, std::ignore)),
        std::tuple<int &, double &, std::string &, decltype(std::ignore) &>
    >::value, "");
    static_assert (std::is_same<
        decltype(bot::tie (bot_int, bot_double, bot_string, bot::ignore)),
        bot::tuple<int &, double &, std::string &, decltype(bot::ignore) &>
    >::value, "");

    /// forward_as_tuple

    static_assert (std::is_same<decltype(
        std::forward_as_tuple (ref_int, cref_int, 1.1, constRValueGen (""s))),
        std::tuple<int &, const int &, double &&, const std::string &&>
    >::value, "");
    static_assert (std::is_same<decltype(
        std::forward_as_tuple (ref_int, cref_int, 1.1, constRValueGen (""s))),
        std::tuple<int &, const int &, double &&, const std::string &&>
    >::value, "");

    // forward as tie
    std::forward_as_tuple (std_int, std_double, std_string) = std_triple_tuple;
    bot::forward_as_tuple (bot_int, bot_double, bot_string) = bot_triple_tuple;

    // 2 -> 3
    assert (std_int == bot_int);
    assert (std_double == bot_double);
    assert (std_string == bot_string);

    static_assert (std::is_same<
        decltype(std::forward_as_tuple (std_int, std_double, std_string)),
        decltype(std::tie (std_int, std_double, std_string))
    >::value, "");
    static_assert (std::is_same<
        decltype(bot::forward_as_tuple (bot_int, bot_double, bot_string)),
        decltype(bot::tie (bot_int, bot_double, bot_string))
    >::value, "");

    /// tuple_cat

    // value cat
    auto std_cat_tuple = std::tuple_cat (std_empty_tuple,
        std_single_tuple, std_double_tuple, std_triple_tuple);
    auto bot_cat_tuple = bot::tuple_cat (bot_empty_tuple,
        bot_single_tuple, bot_double_tuple, bot_triple_tuple);

    static_assert(std::is_same<decltype(std_cat_tuple),
        std::tuple<int, int, double, int, double, std::string>
    >::value, "");
    static_assert(std::is_same<decltype(bot_cat_tuple),
        bot::tuple<int, int, double, int, double, std::string>
    >::value, "");

    assert (
        std::get<0> (std_cat_tuple) ==
        bot::get<0> (bot_cat_tuple)
    );
    assert (
        std::get<1> (std_cat_tuple) ==
        bot::get<1> (bot_cat_tuple)
    );
    assert (
        std::get<2> (std_cat_tuple) ==
        bot::get<2> (bot_cat_tuple)
    );
    assert (
        std::get<3> (std_cat_tuple) ==
        bot::get<3> (bot_cat_tuple)
    );
    assert (
        std::get<4> (std_cat_tuple) ==
        bot::get<4> (bot_cat_tuple)
    );
    assert (
        std::get<5> (std_cat_tuple) ==
        bot::get<5> (bot_cat_tuple)
    );

    // ref cat
    auto std_ref_cat_tuple = std::tuple_cat (std_empty_tuple, std_ref_tuple);
    auto bot_ref_cat_tuple = bot::tuple_cat (bot_empty_tuple, bot_ref_tuple);

    static_assert(std::is_same<
        decltype(std_ref_cat_tuple),
        std::tuple<int &>
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_ref_cat_tuple),
        bot::tuple<int &>
    >::value, "");

    ref_int = 2;
    assert (
        std::get<0> (std_ref_cat_tuple) ==
        bot::get<0> (bot_ref_cat_tuple)
    );

    std::get<0> (std_ref_cat_tuple) = 3;
    assert (
        std::get<0> (std_ref_cat_tuple) ==
        bot::get<0> (bot_ref_cat_tuple)
    );

    bot::get<0> (bot_ref_cat_tuple) = 4;
    assert (
        std::get<0> (std_ref_cat_tuple) ==
        bot::get<0> (bot_ref_cat_tuple)
    );

    // empty cat
    auto std_empty_cat_tuple = std::tuple_cat (std_empty_tuple);
    auto bot_empty_cat_tuple = bot::tuple_cat (bot_empty_tuple);

    auto std_null_cat_tuple = std::tuple_cat ();
    auto bot_null_cat_tuple = bot::tuple_cat ();

    static_assert(std::is_same<
        decltype(std_empty_cat_tuple),
        decltype(std_null_cat_tuple)
    >::value, "");
    static_assert(std::is_same<
        decltype(bot_empty_cat_tuple),
        decltype(bot_null_cat_tuple)
    >::value, "");

    return 0;
}
