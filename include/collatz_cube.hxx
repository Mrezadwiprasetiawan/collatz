/*
  cpp-playground - C++ experiments and learning playground
  Copyright (C) 2025 M. Reza Dwi Prasetiawan


  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


#pragma once

#include <concepts>
#include <integer_pow.hxx>
#include <vector>

template <std::integral I>
struct Pos3 {
  I z, y, x;
  Pos3(I z, I y, I x) : z(z), y(y), x(x) {}
};

template <int p>
int v_adic(int val) {
  if constexpr (p == 2) {
    return __builtin_ctz(val);
  } else {
    int res = 0;
    while (!(val % p)) {
      ++res;
      val /= p;
    }
    return res;
  }
}

template <std::integral Id, std::integral V>
struct CollatzCube {
  static V get_value_from_index(Id z, Id y, Id x) {
    Id z_seed;
    if (!(z & 1)) z_seed = z * 3 + 1;
    else z_seed = z * 3 + 2;
    return z_seed * int_pow<V>(2, y) * int_pow<V>(3, x);
  }

  static std::vector<Pos3<Id>> get_path_index_from_seed(V k) {
    std::vector<Pos3<Id>> res;
    while (k != 1) {
      Id v2k   = v_adic<2>(k);
      Id v3k   = v_adic<3>(k);
      Id v2pow = int_pow<Id>(2, v2k);
      Id v3pow = int_pow<Id>(3, v3k);

      Id row    = v2k;
      Id col    = v3k;
      Id z_seed = k / v2pow / v3pow;
      Id depth  = z_seed / 3;

      while (row) {
        res.push_back(Pos3<Id>(depth, row, col));
        --row;
        ++col;
      }
      res.push_back(Pos3<Id>(depth, row, col));

      k     = z_seed * int_pow<Id>(3, col + 1) - 1;
      v2k   = v_adic<2>(k);
      k   >>= v2k;
      ++k;
      k >>= 1;
    }
    res.push_back(Pos3<Id>(0, 0, 0));
    return res;
  }
};
