/*  Software Rendered Demo Engine In C
    Copyright (C) 2024 https://github.com/aurb

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef UTILS_H
#define UTILS_H

RGB_PIXEL utils_RGB_2_pix(VEC_3 *c);
FLOAT utils_pix_2_L(RGB_PIXEL p);
const char *runtime_file_path(const char *argv0, const char* relative_file_path);
//Pseudo random number generator macros
//Pseudo random number maximum
#define PRN_MAX (0x7FFFFFFF)
//Generate pseudo random number
#define PRN(S) ((S) = (1103515245 * (S) + 12345) & 0x7FFFFFFF)
//Fast limit macros for pseudo random number
#define LIMIT_255(N) ((N)>>23)
#define LIMIT_65535(N) ((N)>>15)
//Universal limit macro for pseudo random number
#define LIMIT_N(N, M) ((M)*(FLOAT)(N)/(FLOAT)0x7FFFFFFF)

// Conversion macros for turning 3 and 4 element arrays into compound lists
#define COMPOUND_3(a) {(a)[0], (a)[1], (a)[2]}
#define COMPOUND_4(a) {(a)[0], (a)[1], (a)[2], (a)[3]}

#endif