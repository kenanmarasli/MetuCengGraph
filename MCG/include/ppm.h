#ifndef MCG_PPM_H
#define MCG_PPM_H

namespace MCG {
void write_ppm(const char *filename, unsigned char *data, int width,
               int height);
}
#endif
