//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

// Test that header file is self-contained.
#include <boost/url/grammar/detail/copied_strings.hpp>

#include "test_suite.hpp"

namespace boost {
namespace urls {
namespace grammar {

struct copied_strings_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    copied_strings_test,
    "boost.url.grammar.copied_strings");

} // grammar
} // urls
} // boost
