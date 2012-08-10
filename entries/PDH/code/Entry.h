#ifndef ENTRY_H
#define ENTRY_H

#include <vector>
#include <list>
#include <deque>

struct xyLoc {
  int16_t x;
  int16_t y;
};

struct grupo {
	int id;
	int borde;
	int tamanyo;
};

struct thread_data {
	std::vector< std::vector<int> >* mini_mapag;
	int contador;
	int width;
	int height;
	int tope; 
	int * mini_grupos_conseguidos;
};

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename);
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path);
const char *GetName();

void Preprocess_Part1(std::vector<bool> &bits, int width, int height, std::vector< std::list<xyLoc> > &puntos_frontera, std::vector< std::list<xyLoc> > &puntos_grupo, std::vector< xyLoc > &puntos_conexion);
void Preprocess_Part2(int width, int height, std::vector< std::list<xyLoc> > &puntos_frontera, std::vector< std::list<xyLoc> > &puntos_grupo, std::vector< xyLoc > &puntos_conexion);
void Preprocess_Part3(int width, int height, const char *filename);

int function_Manhattan_Steps(xyLoc a, xyLoc b, int currCost);
int ManhattanDistance(xyLoc a, xyLoc b);

#endif // ENTRY_H