/**
 * textInterface
 * module for file-based requests
 */

#include <stdio.h>

#define OUT_CMD_FIC_NAME "cmd_output"
#define INP_CMD_FIC_NAME "cmd_input"
#define ROUNDS_LOG_FIC_NAME "rounds.log"
#define COMMANDS_LOG_FIC_NAME "commands.log"
#define ERRORS_LOG_FIC_NAME "errors.log"
#define INFOS_LOG_FIC_NAME "infos.log"

/* affiche un message dans le fichier de sortie de commandes */
void dump_out(char *msg);

/* lit un message dans le fichier d'entréedes commandes */
char *read_input(void);

/* efface le fichier d'entrée des commandes */
void clear_input(void);

/* loggue un message dans le fichier de rounds */
void rounds_log(char *msg);

/* loggue un message dans le fichier de commandes */
void commands_log(char *msg);

/* loggue un message dans le fichier d'erreurs */
void errors_log(char *msg);

/* loggue un message dans le fichier d'infos */
void infos_log(char *msg);

/* ouvre tous les fichiers de log */
int open_log_files(void);

/* ferme tous les fichiers de log */
void close_log_files(void);
