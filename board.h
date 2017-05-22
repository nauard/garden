// Le Terrain : description


#include "cellule.h"


class board {
 public:
	
    board(int);
    void actualise();
    bool get_cell(int x, int y);
    void reset_cell(int x, int y, int round);
    void regen(int round);
    bool out_of_bounds(int ,int);
    void stop_regen();
    void start_regen();

 private:
    int nb_larg, nb_haut;
    bool grass_growing;
    cellule *map[LARG_TERRAIN][LARG_TERRAIN];
};
