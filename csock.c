#include "csock.h"

char err_msg[256];

int create_socket(const char *service, const char *transport, struct sockaddr_in *sin) {
	struct servent *iservice;
	struct protoent *itransport;
	int s, type, proto;
	
	/* initialisation de la structure socket */
	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
	
	/* recherche du numero de port */
	sin->sin_port = htons(atoi(service));
	if (!sin->sin_port) {
		/* si le service n'était pas un numéro, on le recherche dans /etc/services */
		iservice = getservbyname(service, transport);
		if (!service) {
                    sprintf(err_msg, "Service non enregistre: %s", service);
                    perror(err_msg);
                }
		sin->sin_port = iservice->s_port;
	}
	
	/* recherche du numéro de protocole */
	itransport = getprotobyname(transport);
	if (!itransport) {
            sprintf(err_msg, "Protocole non enregistre: %s", transport);
            perror(err_msg);
        }
	proto = itransport->p_proto;
	
	if (!strcmp(transport, "udp"))
		type = SOCK_DGRAM; // protocole UDP
	else
		type = SOCK_STREAM; // protocole TCP
		
	/* création de la socket */
	s = socket(AF_INET, type, proto);
	if (s < 0)
		perror("creation de socket impossible");
		
	return s;
}


int client_socket(const char *host, const char *service, const char *transport) {
	struct sockaddr_in sin;
	int s;
	
	/* création d'une socket */
	s = create_socket(service, transport, & sin);
	
	sin.sin_addr.s_addr = inet_addr(host);
	if (sin.sin_addr.s_addr == INADDR_NONE) {
            sprintf(err_msg, "host inconnu: %s", host);
            perror(err_msg);
        }
		
	/* connection  de la socket */
	if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            sprintf(err_msg, "impossible de se connecter a %s[%s]", host, service);
            perror(err_msg);
            return -1;
        }
		
	return s;
}


int server_socket(const char *service, const char *transport, int lqueue) {
	struct sockaddr_in sin;
	int s;
	
	/* création d'une socket */
	s = create_socket(service, transport, &sin);
	
	sin.sin_addr.s_addr = INADDR_ANY;
	
	/* allocation du numéro de port */
	if (bind(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
            sprintf(err_msg, "impossible de lier la socket a %s", service);
            perror(err_msg);
        }
		
	/* place la socket en mode passif si transport != udp */
	if (strcmp(transport, "udp") && listen(s, lqueue) < 0) {
            sprintf(err_msg, "mode passif impossible sur %s", service);
            perror(err_msg);
        }
	return s;
}

