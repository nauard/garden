// Les entités

#include "spec.h"

class individu {
 public:
    int indice;
    individu *prev;
    individu *next;
    individu *first;
    individu *last;

    
    individu(int);
    void walk();
    void eat();
    int get_x_pos(void);
    int get_y_pos(void);
    void set_x_pos(int xpos);
    void set_y_pos(int ypos);
    void set_points(int p);
    int get_points(void);
    
 private:
    int x, y;
    int points;
    void rebond(void);
};
