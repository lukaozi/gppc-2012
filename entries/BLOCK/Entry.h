#ifndef ENTRY_H
#define ENTRY_H

#include <vector>

struct xyLoc {
  int16_t x;
  int16_t y;
};

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename);
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path);
const char *GetName();

int GetIndex(xyLoc s);

#endif // ENTRY_H
