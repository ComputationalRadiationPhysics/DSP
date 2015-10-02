/*
@copyright Louis Dionne 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#include <meta/meta.hpp>


struct f {
    template <typename, typename>
    struct apply;
};

template <int> struct x;

struct state;

using list = meta::list<
    <%= (1..input_size).map { |i| "x<#{i}>" }.join(', ') %>
>;

using result = meta::reverse_fold<list, state, f>;

int main() {

}
