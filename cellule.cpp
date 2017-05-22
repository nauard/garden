// Une cellule du terrain : implémentation

#include "cellule.h"


cellule::cellule() {
    this->last_eat = -REGEN_SPEED;
    this->grass = false;
}
