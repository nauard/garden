// individu : implémentation

#include "individu.h"
#include <stdio.h>

/**
 * génération des caractéristiques d'un individu
 */
individu::individu(int num) {
    // indice
    indice = num;
    // coordonnées de départ
    x = 10;
    y = 10;
    // points de vie de départ
    points = START_POINTS;  //  1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0))); // de 1 a 10
    prev = NULL;
    next = NULL;
    first = NULL;
    last = NULL;
}


/**
 * effectue le calcul du rebond d'un individu,
 * lors d'un déplacement, contre le "bord" du terrain
 */
void individu::rebond() {
    int indiceSup = LARG_TERRAIN - 1;

    //printf("in rebond x(%d) y(%d)\n", x, y);
    if (x < 0) {
	x = -x;
	//printf("rebond\n");
    }
    if (x > indiceSup) {
	x = indiceSup - (x - indiceSup);
	//printf("rebond\n");
    }
    if (y < 0) {
	y = -y;
	//printf("rebond\n");
    }
    if (y > indiceSup) {
	y = indiceSup - (y - indiceSup);
	//printf("rebond\n");
    }
    //printf("new coords x(%d) y(%d)\n", x, y);
}


void individu::walk() {
    // déplacement aléatoire
    int delta_x = -1 + (int) (3.0 * (rand() / (RAND_MAX + 1.0))); // de -1 a 1
    int delta_y = -1 + (int) (3.0 * (rand() / (RAND_MAX + 1.0))); // de -1 a 1
    //printf("ind(%d) x(%d)+(%d) y(%d)+(%d)\n", indice, x, delta_x, y, delta_y);
    x += delta_x;
    y += delta_y;
    // tester le rebond
    this->rebond();
    points--;
}


void individu::eat() {
    points += 5;
}


void individu::set_x_pos(int xpos) {
    x = xpos;
}

int individu::get_x_pos(void) {
    return x;
}

void individu::set_y_pos(int ypos) {
    y = ypos;
}

int individu::get_y_pos(void) {
    return y;
}

void individu::set_points(int p) {
    points = p;
}

int individu::get_points(void) {
    return points;
}
