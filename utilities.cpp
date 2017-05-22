/**********
 * Utilities
 ***********/

#include "individu.h"

/* ajout à la liste chainee */
void addToList(individu *liste, individu* elem) {
    elem->next = NULL;

    if (liste->first == NULL) {
	liste->first = elem;
    } else {
	liste->last->next = elem;
	elem->prev = liste->last;
    }
    liste->last = elem;
}

/* retrait de la liste chainee */
void removeFromList(individu *liste, individu* elem) {
    // on lie le précédent ver le suivant
    if (elem->prev == NULL) {
	// Si c'est le 1er de la liste
	liste->first = elem->next;
    } else {
	elem->prev->next = elem->next;
    }
    // on lie le suivant ver le précédent
    if (elem->next == NULL) {
	// Si c'est le dernier de la liste
	liste->last = elem->prev;
    } else {
	elem->next->prev = elem->prev;
    }
}

/* enlève l'individu dont l'indice est num */
bool remove_ind_num(individu *liste, int num) {
    individu *current_ind = NULL;

    current_ind = liste->first;
    while (current_ind) {
	if (current_ind->indice == num) {
	    removeFromList(liste, current_ind);
            return true;
	}
	current_ind = current_ind->next;
    }

    return false;
}

