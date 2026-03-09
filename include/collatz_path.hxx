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
std::vector<I> collatz_get_path(I seed) {
  using namespace std;
  vector<I> result;
  while (true) {
    while (!(seed & 1)) seed >>= 1;
    result.push_back(seed);
    if (seed == 1) return result;
    ++seed;
    I v2 = 0, v3 = 0;
    while (!(seed & 1)) {
      seed >>= 1;
      ++v2;
    }
    seed = seed * int_pow<I>(3, v2) - 1;
  }
  return result;
}