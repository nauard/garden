// Une cellule du terrain : impl�mentation

#include "cellule.h"


cellule::cellule() {
    this->last_eat = -REGEN_SPEED;
    this->grass = false;
}
