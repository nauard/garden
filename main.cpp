// Micro-Monde

#include <unistd.h>
#include <X11/Xlib.h>
#include <pthread.h>
#include <sys/time.h>
// libs
#include "csock.h"
// objects
#include "board.h"
#include "individu.h"
#include "utilities.h"
#include "textInterface.h"

#define WIDTH  250
#define HEIGHT 200
#define LARG_CELL 4

#define STATE_CLEAR 0
#define STATE_EXIT 1
#define STATE_KILL 2

using namespace std;

/* GLOBALS */
/* main purpose */
int living = 0;		// nb entites vivantes
int round = 0;          //nb de tour de jeu
individu *List_ind = new individu(-1);
int last_indice = 0; // indice du dernier individu
board *Terrain;
struct timeval start_tv; // top de reference de strat du prg
int sleep_time = 2000000;

/* threading needs */
pthread_mutex_t mutex;

/* network needs */
const char *service;
struct sockaddr_in sin; // structure décrivant  la socket active
socklen_t lsin = sizeof(sin);
int msock;
int cli_sock[10];


/* graphical needs */
bool enable_x11 = true;
Colormap	cmap;
int		screen;
Window		root;
unsigned long	white_pixel, black_pixel;

#define NB_COLORS 8

Display		*dpy;
GC		gc;
Window		monitor, map;

typedef enum {
    col_white,
    col_red,
    col_green,
    col_yellow,
    col_blue,
    col_black,
    col_cyan,
    col_grey
} color_types;

static const char *color_names[] = {
  "white",
  "red",
  "green",
  "yellow",
  "blue",
  "black",
  "cyan",
  "dim grey"
};


static XColor x_colors[NB_COLORS];

/* Pending mode */
bool redraw = false;



void init_graphics(void) {
    int i;
    char log_msg[256];
    XColor	exact;

    dpy = XOpenDisplay(0);
    if (dpy == NULL) {
        errors_log((char *) "No DISPLAY is available");
        exit (1);
    }
    cmap = DefaultColormap (dpy, screen);
    for (i = 0; i != NB_COLORS; i++) {
        if ( !XAllocNamedColor (dpy, cmap, color_names[i], &x_colors[i], &exact)) {
            sprintf(log_msg, "can't alloc color %s", color_names[i]);
            errors_log(log_msg);
            exit(2);
        }
    }
    gc = DefaultGC (dpy, screen);
    screen = DefaultScreen (dpy);
    root = RootWindow (dpy, screen);
    white_pixel = WhitePixel (dpy, screen);
    black_pixel = BlackPixel (dpy, screen);

    monitor = XCreateSimpleWindow (dpy, root, 0, 0, WIDTH, HEIGHT, 2, white_pixel, black_pixel);
    XStoreName (dpy, monitor, "livings");
    XMapWindow (dpy, monitor);

    map = XCreateSimpleWindow (dpy, root, 0, 0, LARG_TERRAIN * LARG_CELL, LARG_TERRAIN * LARG_CELL, 2, white_pixel, black_pixel);
    XStoreName (dpy, map, "map");
    XMapWindow (dpy, map);

    /* SendEvent mode */
    // if (!XInitThreads()) {
    //     fprintf(stderr, "can't initialize X11 threads\n");
    //     exit(3);
    // }
}


void init_server_side_commands() {
    service = "5656"; // notre n° de port
}


void shutdown_and_exit(char *msg) {
    int i;

    commands_log(msg);
    for (i = 0 ; i < 10 ; i++) {
        if (cli_sock[i] != 0)
            close(cli_sock[i]);
        cli_sock[i] = 0;
    }
    close(msock);
    close_log_files();
    if (enable_x11)
        XCloseDisplay(dpy);
    exit(0);
}


void YDrawString(int x, int y, char *msg, int color){
    XSetForeground(dpy, gc, x_colors[ color ].pixel);
    XDrawString (dpy, monitor, gc, x, y, msg, strlen(msg) );
}


void showClick(void) {
    XClearWindow(dpy, monitor);
    XDrawString(dpy, monitor, gc, 20, 50, "C L I C K", 9);
    XFlush(dpy);
    sleep(1);
}

void draw_monitor(void) {
    char msg[256];
    int points;

    //XLockDisplay(dpy);
    // efface la fenetre
    XClearWindow(dpy, monitor);

    // affiche résumé
    XSetForeground(dpy, gc, x_colors[ col_red ].pixel);
    sprintf(msg, "round : %d", round);
    XDrawString (dpy, monitor, gc, 10, 30, msg, strlen(msg) );
    sprintf(msg, "living : %d", living);
    YDrawString (100, 30, msg, col_green);

    // affiche tableau
    int color_ind, decal=0;
    individu *ind = List_ind->first;
    while (ind) {
        points = ind->get_points();
        sprintf(msg, "ind(%d) pos(%d,%d) points(%d)",
                ind->indice, ind->get_x_pos(), ind->get_y_pos(), points);
        if (points < 20)
            color_ind = col_blue;
        else
            color_ind = col_white;
        YDrawString( 10, 50+decal, msg, color_ind);
        ind = ind->next;
        decal += 15;
    }

    //XUnlockDisplay(dpy);
    XFlush(dpy);
}

void draw_map(void) {
    int x, y;
    int points, color_ind;

    // efface la fenetre
    XClearWindow(dpy, map);
    // affiche les individus
    individu *ind = List_ind->first;
    while (ind) {
        x = ind->get_x_pos();
        y = ind->get_y_pos();
        points = ind->get_points();
        if (points < 20)
            color_ind = col_blue;
        else
            color_ind = col_white;
        XSetForeground(dpy, gc, x_colors[ color_ind ].pixel);
        XFillRectangle(dpy, map, gc, LARG_CELL*x, LARG_CELL*y, LARG_CELL, LARG_CELL);
        ind = ind->next;
    }
    // affiche le terrain
    XSetForeground(dpy, gc, x_colors[ col_green ].pixel);
    for ( x = 0 ; x < LARG_TERRAIN ; x++ )
        for ( y = 0 ; y < LARG_TERRAIN ; y++ )
            if ( Terrain->get_cell(x, y) ) {
                XFillRectangle(dpy, map, gc, LARG_CELL*x, LARG_CELL*y, LARG_CELL, LARG_CELL);
                XFlush(dpy);
            }
}

void expose(void) {
    draw_monitor();
    draw_map();
}

/* GUI event loop */
void *WaitEvent(void *) {
    XEvent ev;
    int nbEvent;
    char log_msg[256];

    XSelectInput (dpy, monitor, ButtonPressMask | KeyPressMask | ExposureMask);
    XMapWindow (dpy, monitor);
    while (enable_x11) {
        /* Pending mode */
        if (redraw) {
            redraw = false;
            expose();
        }
        nbEvent = XPending(dpy);
        if (nbEvent) {
        /* end of Pending mode */
            XNextEvent(dpy, &ev);
            switch (ev.type) {

            /* MOUSE EVENTS */
            case ButtonPress:
                if (ev.xbutton.button == 1) {
                    // Left mouse button pressed
                    showClick();
                }
                if (ev.xbutton.button == 3) {
                    /* Right mouse button pressed */
                    shutdown_and_exit((char *) "exit by right-click");
                }
                break;

            /* KEY EVENTS */
            case KeyPress:
                switch (ev.xkey.keycode) {
                case 38 :
                    /* q -> quit */
                    shutdown_and_exit((char *) "exit by q key");
                    break;
                case 40 :
                    /* d -> detach X11 */
                    commands_log((char *) "detaching from display");
                    XCloseDisplay(dpy);
                    enable_x11 = false;
                    break;
		case 86 :
		    /* + -> accelerate time */
		    sleep_time = sleep_time / 2;
		    sprintf(log_msg, "sleep_time is set to %d ms", sleep_time);
		    commands_log(log_msg);
		    break;
		case 82 :
		    /* - > deccelerate time */
		    sleep_time = sleep_time * 2;
		    sprintf(log_msg, "sleep_time is set to %d ms", sleep_time);
		    commands_log(log_msg);
		    break;
                default:
                    break;
                }
                break;

            case Expose:
                expose();
                break;

            default:
                break;
            }
        /* Pending mode */
        }
        usleep(100000);
    }

    return NULL;
}



/*** Commands management ***/

/**
 * affiche un statut de tous les individus dans le fichier de sortie de commandes
 * @param List_ind : liste des individus
 * @result answer : la réponse sous forme de chaine
 */
void get_ind_status(char *answer) {
    individu *current_ind = NULL;
    char log_msg[256];
    int nbInd = 0;

    current_ind = List_ind->first;
    while (current_ind) {
        nbInd++;
        sprintf(answer, "%sind(%d) pos(%d,%d) points(%d)\n", answer,
                current_ind->indice, current_ind->get_x_pos(), current_ind->get_y_pos(),
                current_ind->get_points() );
        current_ind = current_ind->next;
    }
    sprintf(log_msg, "nb indiv listed : %d", nbInd);
    commands_log(log_msg);
}

/* compare 2 chaines entre elles et renvoie true si elles sont égales */
bool cmd_is(char *cmd, const char *order) {
    return !(strncmp(order, cmd, strlen(order)));
}

/* interpret a command, process chosen action and return a message (result) */
char *interpret_command(char *msg, int *state) {
    int res;
    char log_msg[256];
    pthread_t thread;
    char *answer = (char *) malloc(ANSWER_SIZE);
    // actions
    if (cmd_is(msg, "stop")) {
        // shutdown requested
        commands_log((char *) "stop requested");
        sprintf(answer, "stopping");
        *state = STATE_EXIT;

    } else if (cmd_is(msg, "detach")) {
        commands_log((char *) "detaching from display");
        if (enable_x11)
            XCloseDisplay(dpy);
        enable_x11 = false;
        sprintf(answer, "detaching from display");

    } else if (cmd_is(msg, "attach")) {
        commands_log((char *) "reattaching X11 display");
        enable_x11 = true;
        init_graphics();
        // launch the graphical part thread
        res = pthread_create( &thread, NULL, WaitEvent, NULL);
        if (res != 0) errors_log((char *) "error creating thread WaitEvent");
        sprintf(answer, "reattaching X11 display");

    } else if (cmd_is(msg, "dryness")) {
        commands_log((char *) "stop grass regeneration");
        Terrain->stop_regen();
        sprintf(answer, "grass regeneration stopped");

    } else if (cmd_is(msg, "droprain")) {
        commands_log((char *) "activate grass regeneration");
        Terrain->start_regen();
        sprintf(answer, "grass regeneration started");

    } else if (cmd_is(msg, "uptime")) {
        struct timeval stop_tv;
        gettimeofday(&stop_tv, NULL);
        int uptime = stop_tv.tv_sec - start_tv.tv_sec;
        int days = uptime / 86400;
        int x = uptime % 86400;
        int hour = x / 3600;
        x = x % 3600;
        int min = x / 60;
        int sec = x % 60;
        sprintf(log_msg, "uptime of %d seconds", uptime);
        commands_log(log_msg);
        sprintf(answer, "program is running for %d seconds", uptime);
        sprintf(answer, "%s\n%djours %dh%dm%ds", answer, days, hour, min, sec);

    } else if (cmd_is(msg, "status")) {
        sprintf(answer, "round(%d) living(%d)\n", round, living);
        get_ind_status(answer);
        sprintf(log_msg, "answer length is : %u", strlen(answer));
        infos_log(log_msg);

    } else if (cmd_is(msg, "?")) {
        sprintf(answer, "possible commands are : ? help stop status kill attach detach dryness droprain uptime");

    } else if (cmd_is(msg, "help")) {
        sprintf(answer, "commands detail:");
        sprintf(answer, "%s\n?      : list of possible commands", answer);
        sprintf(answer, "%s\nhelp   : this help page", answer);
        sprintf(answer, "%s\nstop   : stop the main server program and quit", answer);
        sprintf(answer, "%s\nstatus : gives a status of all individuals", answer);
        sprintf(answer, "%s\nkill   : shoot an individual off", answer);
        sprintf(answer, "%s\nattach : activate x11 windows (monitor & map)", answer);
        sprintf(answer, "%s\ndetach : close x11 windows (monitor & map)", answer);
        sprintf(answer, "%s\ndryness: stop the grow of the grass", answer);
        sprintf(answer, "%s\ndroprain: reactivate the grow of the grass", answer);
        sprintf(answer, "%s\nuptime : display how long the program is running", answer);

    } else if (cmd_is(msg, "kill")) {
        // le 1er on s'en fiche, on sait deja que c'est 'kill'
        strtok(msg, " ");
        char *sPid = strtok(NULL, " ");
        if (sPid == NULL) {
            sprintf(answer, "you do not have provided an individual id");
        } else {
            bool success = remove_ind_num( List_ind, atoi(sPid) );
            if (success) {
                sprintf(answer, "killing individual : %s", sPid);
                living--;
            } else {
                sprintf(answer, "no such individual : %s", sPid);
            }
        }
    } else if (strlen(msg) != 0) {
        sprintf(answer, "unknown command : %s", msg);
        sprintf(answer, "%s? gives a list of understood commands", answer);
    }

    return answer;
}



/* Main squequential thread for parsing individuals */
void thread_entities_loop(void) {
    int i;
    char log_msg[256];
    individu *current_ind;
    int x, y;

    // initialisations globales
    srand(time(0));

    // création du terrain
    Terrain = new board(LARG_TERRAIN);
	
    // création des individus
    for ( i = 0 ; i < START_NB_IND ; i++ ) {
	individu *Ind = new individu(i);
	addToList(List_ind, Ind);
    }
    last_indice = i;
	
    living = START_NB_IND;

    // simulation loop
    while (living > 0) {
	current_ind = List_ind->first;
	// new round
        round++;
        // on ne le print que tous les 10 tours seulement
        //if (round % 10 == 0) {
        sprintf(log_msg, "round : %d      living : %d", round, living);
        rounds_log(log_msg);
            //}

        // capture le mutex de List_ind pour les evolutions
        pthread_mutex_lock(&mutex);
	// pour chaque individu
	while (current_ind) {
	    // on le fait évoluer
	    current_ind->walk();
            x = current_ind->get_x_pos();
            y = current_ind->get_y_pos();
            sprintf(log_msg, "ind %d is now at (%d,%d)", current_ind->indice, x, y);
            rounds_log(log_msg);

            /* Normalement ne sert plus grâce au rebond() des individus */
	    // test des bordures
	    // if (Terrain->out_of_bounds(x, y)) {
	    //     printf("ind(%d) has fallen : %d, %d\n", current_ind->indice, x, y);
	    //     exit(1);
	    //     // on le sort de la liste
	    //     removeFromList(List_ind, current_ind);
	    //     living--;
	    //     printf("living : %d\n", living);
	    //     // on passe au suivant
	    //     current_ind = current_ind->next;
	    //     continue;
	    // }

	    // Est-ce qu'il y a de l'herbe à cette case ?
            if (Terrain->get_cell(x, y) ) {
		// Si oui, l'individu mange
                current_ind->eat();
                sprintf(log_msg, "ind(%d) eating !!", current_ind->indice);
		rounds_log(log_msg);
                Terrain->reset_cell(x, y, round);
            }

            int points = current_ind->get_points();
            // Is current is reproducing ? XD
            if (points >= 100) {
                sprintf(log_msg, "ind(%d) is duplicating :-)", current_ind->indice);
                rounds_log(log_msg);
                // duplicate
                individu *Ind = new individu(++last_indice);
                Ind->set_x_pos( x );
                Ind->set_y_pos( y );
                Ind->set_points( points / 2 );
                addToList(List_ind, Ind);
                living++;
                current_ind->set_points( points / 2 );
            }

	    // Is current living ?
	    if (points <= 0) {
		sprintf(log_msg, "ind(%d) is over : %d", current_ind->indice, current_ind->get_points());
                rounds_log(log_msg);
		// on le sort de la liste
		removeFromList(List_ind, current_ind);
		living--;
		sprintf(log_msg, "living : %d", living);
                rounds_log(log_msg);
	    }

	    // individu suivant
	    current_ind = current_ind->next;
	}

        // Libère le mutex de List_ind pour les commandes externes
        pthread_mutex_unlock(&mutex);

	// On laisse l'herbe repousser
	Terrain->regen(round);
	// printf("\n");

	if (round % 5 == 0) {
            // traitement des commandes
            char *cmd = read_input();
            if (cmd != NULL && strlen(cmd) != 0) {
                clear_input();
                int state = STATE_CLEAR;
                char *answer = interpret_command(cmd, &state);
                dump_out(answer);
                free(answer);
                if (state == STATE_EXIT) {
                    shutdown_and_exit((char *) "exit by file request");
                } 
                free(cmd);
            }
        }

        /* ecoute des requetes du moniteur */
        
        /* Pending mode */
        redraw = true;
            
        /* SendEvent mode */
        // XLockDisplay(dpy);
        // XClientMessageEvent dummyEvent;
        // memset(&dummyEvent, 0, sizeof(XClientMessageEvent));
        // dummyEvent.type = Expose;
        // dummyEvent.window = monitor;
        // dummyEvent.format = 32;
        // XSendEvent(dpy, monitor, True, ExposureMask, (XEvent*)&dummyEvent);
        // XFlush(dpy); // for window refresh
        // XUnlockDisplay(dpy);

	// pause
        usleep(sleep_time);
    }

    shutdown_and_exit((char *) "exit cause nobody is there");
}


/* thread to manage the connection with one client */
void *thread_instance_client(void *arg) {
    //  ~/local/network/net/serveur.c
    int nbRead, nbWritten;
    int client_talking = 1;
    char msg[256];
    char banner[1024];
    char hostname[64];

    int * sock = (int*) arg;

    gethostname(hostname, 64);
    sprintf(banner, "Welcome on %s\nserver pid is %d\nWe are hosting %d individuals", hostname, getpid(), living);
    nbWritten = write(*sock, banner, strlen(banner));
    if (nbWritten < 0) {
        perror("ERROR writing to socket");
        return NULL;
    }

    while (client_talking) {
        // waiting for client query
        nbRead = read(*sock, msg, sizeof(msg));
        if (nbRead < 0) {
            perror("ERROR reading from socket");
            break;
        }
        // client has exited
        if (nbRead == 0) {
            break;
        }

        msg[nbRead] = '\0';

        // waiting for mutex
        pthread_mutex_lock(&mutex);

        int state = STATE_CLEAR;
        char *answer = interpret_command(msg, &state);

        // releasing mutex
        pthread_mutex_unlock(&mutex);
            
        // answering client
        nbWritten = write(*sock, answer, strlen(answer));
        if (nbWritten < 0) {
            perror("ERROR writing to socket");
            break;
        }

        if (state == STATE_EXIT) {
            shutdown_and_exit((char *) "exit by network request");
        } 
    }

    close(*sock);
    *sock = 0;
    infos_log((char *) "network client has diconnected\n");

    return NULL;
}

/* thread for listening to external commands (network) */
void *thread_commands(void *) {
    int i, res;
    char log_msg[256];
    pthread_t thread_network_client;

    // various network settings
    init_server_side_commands();
    for (i = 0 ; i < 10 ; i++) {
        cli_sock[i] = 0;
    }

    // general listening
    msock = server_socket(service, "tcp", 5);

    i=0;
    for (;;) {
        // waiting for a client
        cli_sock[i] = accept(msock, (struct sockaddr *) &sin, &lsin);
        sprintf(log_msg, "connection from: %s\n", inet_ntoa(sin.sin_addr));
        infos_log(log_msg);

        res = pthread_create( &thread_network_client, NULL, thread_instance_client, &cli_sock[i]);
        if (res != 0) printf("error creating thread thread_instance_client\n");
        res = pthread_detach(thread_network_client);
        if (res) printf("error detaching thread thread_instance_client\n");

        i++;
        // wait
        sleep(3*sleep_time);
    }

    return NULL;
}


void usage(char *prg_name) {
    printf("usage :\n");
    printf("%s [-nw]\n", prg_name);
    printf("  -nw : no-window-system\n");
}


int main(int argc, char *argv[]) {
    int res;
    pthread_t thread_network_connections, thread_x11;

    if (argc > 2) {
        usage(argv[0]);
        return 1;
    } else if (argc == 2) {
        if (!strcmp(argv[1], "-nw")) {
            enable_x11 = false;
        } else {
            usage(argv[0]);
            return 1;
        }
    }
    
    if (!open_log_files()) {
        printf("error when creating log files\n");
        exit(1);
    }

    if (enable_x11)
        init_graphics();

    // get start time reference for later uptime
    gettimeofday(&start_tv, NULL);

    // init mutex
    pthread_mutex_init(&mutex, NULL);

    // launch the external commands thread
    res = pthread_create( &thread_network_connections, NULL, thread_commands, NULL);
    if (res != 0) printf("error creating thread thread_commands\n");
    res = pthread_detach(thread_network_connections);
    if (res) printf("error detaching thread thread_commands\n");

    if (enable_x11) {
        // launch the graphical part thread
        res = pthread_create( &thread_x11, NULL, WaitEvent, NULL);
        if (res != 0) printf("error creating thread WaitEvent\n");
        res = pthread_detach(thread_x11);
        if (res) printf("error detaching thread WaitEvent\n");
    }

    // start the main ind loop
    thread_entities_loop();


    // /* reassemble all threads */
    // res = pthread_join( thread_x11, NULL);
    // if (res) fprintf( stderr, "error joining thread WaitEvent\n");
    // res = pthread_join( thread_netclient, NULL);
    // if (res) fprintf( stderr, "error joining thread thread_commands\n");
    // res = pthread_join( thread_entities, NULL);
    // if (res) fprintf( stderr, "error joining thread thread_entities_loop\n");

    return 0;
}
