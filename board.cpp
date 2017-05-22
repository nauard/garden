// Le Terrain : implémentation

#include "board.h"
#include "textInterface.h"
#include <stdio.h>

/**
 * Construction du terrain d'évolution
 */
board::board(int larg) {
    int i,j;
    int x,y;
    bool succeeded;
    cellule *cell;

    nb_larg = larg;
    nb_haut = larg;
    grass_growing = true;

    // on génère les larg^2 cellules du damier
    for ( i = 0 ; i < nb_larg ; i++ ) {
        for ( j = 0 ; j < nb_haut ; j++ ) {
            cell = new cellule();
            map[i][j] = cell;
        }
    }

    // génération du terrain de départ
    for ( i = 0 ; i < START_CELL ; i++ ) {
        succeeded = false;
        while (!succeeded) {
            x = (int) ( (LARG_TERRAIN - 1) * (rand() / (RAND_MAX + 1.0))); // de 0 a LARG_TERRAIN - 1
            y = (int) ( (LARG_TERRAIN - 1) * (rand() / (RAND_MAX + 1.0))); // de 0 a LARG_TERRAIN - 1
            cell = map[x][y];
	    // S'il n'y a pas déjà de l'herbe ici, onn en fait pousser
            if (!cell->grass) {
                cell->grass = true;
                succeeded = true;
            }
        }
    }

}

/**
 * retourne la présence d'herbe pour cette case
 */
bool board::get_cell(int x, int y) {
    //printf("in board::get_cell, x(%d) y(%d)\n",x, y);
    cellule *cell = map[x][y];
    return cell->grass;
};


/**
 * vide l'herbe de la case en question
 */
void board::reset_cell(int x, int y, int round) {
    cellule *cell;

    cell = map[x][y];
    cell->grass = false;
    cell->last_eat = round;
}


/**
 * fait pousser l'herbe aléatoirement
 */
void board::regen(int round) {
    int i,j;
    cellule *cell;
    char log_msg[256];

    for (int regrown = 0 ; regrown < NB_CELLS_REGROW ; regrown++) {
	i = (int) (1.0 * nb_larg * rand()/(RAND_MAX + 1.0));
	j = (int) (1.0 * nb_haut * rand()/(RAND_MAX + 1.0));
	cell = map[i][j];
	if (!cell->grass) {
	    // the cell is empty
	    if (grass_growing && (round - cell->last_eat > REGEN_SPEED) ) {
		cell->grass = true;
		sprintf(log_msg, "grass is growing in (%d,%d)", i, j);
		infos_log(log_msg);
	    }
	}
    }
}


/**
 * détermine si la position donnée est en dehors du terrain
 */
bool board::out_of_bounds(int x, int y) {
    if (x < 0 || y < 0 || x >= LARG_TERRAIN || y >= LARG_TERRAIN) {
	return true;
    } else {
	return false;
    }
}

/**
 * stoppe la repopusse de l'herbe
 */
void board::stop_regen() {
    grass_growing = false;
}

/**
 * active la repopusse de l'herbe
 */
void board::start_regen() {
    grass_growing = true;
}
