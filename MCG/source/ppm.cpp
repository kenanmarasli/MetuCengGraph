// Copyright (C) <year> <name of author>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ppm.h"
#include <stdexcept>

void MCG::write_ppm(const char *filename, unsigned char *data,
                              int width, int height) {
    FILE *outfile;

    if ((outfile = fopen(filename, "w")) == NULL) {
        throw std::runtime_error(
            "Error: The ppm file cannot be opened for writing.");
    }

    (void)fprintf(outfile, "P3\n%d %d\n255\n", width, height);

    unsigned char color;
    for (size_t j = 0, idx = 0; j < height; ++j) {
        for (size_t i = 0; i < width; ++i) {
            for (size_t c = 0; c < 3; ++c, ++idx) {
                color = data[idx];

                if (i == width - 1 && c == 2) {
                    (void)fprintf(outfile, "%d", color);
                } else {
                    (void)fprintf(outfile, "%d ", color);
                }
            }
        }

        (void)fprintf(outfile, "\n");
    }

    (void)fclose(outfile);
}
