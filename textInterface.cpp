#include "textInterface.h"
#include <stdlib.h>
#include <string.h>

FILE *rounds_log_fd = NULL;
FILE *commands_log_fd = NULL;
FILE *errors_log_fd = NULL;
FILE *infos_log_fd = NULL;


/* affiche un message dans le fichier de sortie de commandes */
void dump_out(char *msg) {
    FILE *fic_out = NULL;

    fic_out = fopen(OUT_CMD_FIC_NAME, "a");
    if (fic_out != NULL) {
	fprintf(fic_out, "%s\n", msg);
	fclose(fic_out);
    }
}

/* lit un message dans le fichier d'entrée des commandes */
char *read_input(void) {
    char *cmd = NULL;

    FILE *fic_cmd = NULL;
    fic_cmd = fopen(INP_CMD_FIC_NAME, "r");
    if (fic_cmd != NULL) {
        cmd = (char *) malloc(32);
        memset(cmd, 0, 32);
        fgets(cmd, 32, fic_cmd);
        fclose(fic_cmd);
    }
    return cmd;
}

/* efface le fichier d'entrée des commandes
   pour débloquer le shell de saisie des commandes */
void clear_input(void) {
    if ( remove(INP_CMD_FIC_NAME) < 0 ) {
        printf("Cannot remove file %s\n", INP_CMD_FIC_NAME);
    }
}

/* loggue un message dans un fichier de sortie */
void log(FILE *log_fd, char *msg) {
    if (log_fd != NULL) {
	fprintf(log_fd, "%s\n", msg);
	fflush(log_fd);
    }
}

/* loggue un message dans le fichier de rounds */
void rounds_log(char *msg) {
    log(rounds_log_fd, msg);
}

/* loggue un message dans le fichier de commandes */
void commands_log(char *msg) {
    log(commands_log_fd, msg);
}

/* loggue un message dans le fichier d'erreurs */
void errors_log(char *msg) {
    log(errors_log_fd, msg);
}

/* loggue un message dans le fichier d'infos */
void infos_log(char *msg) {
    log(infos_log_fd, msg);
}

/* ouvre tous les fichiers de log */
int open_log_files(void) {

    rounds_log_fd = fopen(ROUNDS_LOG_FIC_NAME, "a");
    if (rounds_log_fd == NULL)
        return false;
    else
        rounds_log((char*) "Application starting");

    commands_log_fd = fopen(COMMANDS_LOG_FIC_NAME, "a");
    if (commands_log_fd == NULL)
        return false;
    else
        commands_log((char*) "Application starting");

    errors_log_fd = fopen(ERRORS_LOG_FIC_NAME, "a");
    if (errors_log_fd == NULL)
        return false;
    else
        errors_log((char*) "Application starting");

    infos_log_fd = fopen(INFOS_LOG_FIC_NAME, "a");
    if (infos_log_fd == NULL)
        return false;
    else
        infos_log((char*) "Application starting");

    return true;
}

/* ferme tous les fichiers de log */
void close_log_files(void) {
    rounds_log((char*) "Application ending");
    commands_log((char*) "Application ending");
    errors_log((char*) "Application ending");
    infos_log((char*) "Application ending");
    fclose(rounds_log_fd);
    fclose(commands_log_fd);
    fclose(errors_log_fd);
    fclose(infos_log_fd);
}
