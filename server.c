#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>
#include <arpa/inet.h>

struct _client
{
        char ipAddress[40];
        int port;
        char name[40];
} tcpClients[4];

int nbClients;
int fsmServer;
int deck[13]={0,1,2,3,4,5,6,7,8,9,10,11,12};
int tableCartes[4][8];
char *nomcartes[] = {	"Sebastian Moran",
						"Irene Adler",
						"Inspector Lestrade",
						"Inspector Gregson",
						"Inspector Baynes",
						"Inspector Bradstreet",
						"Inspector Hopkins",
						"Sherlock Holmes",
						"John Watson",
						"Mycroft Holmes",
						"Mrs. Hudson",
						"Mary Morstan",
						"James Moriarty"
					};
int joueurCourant;
int listeJoueurs[4]={1, 1, 1, 1};
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void melangerDeck()
{
    int i;
    int index1,index2,tmp;

    for (i=0;i<1000;i++)
    {
        index1=rand()%13;
        index2=rand()%13;

        tmp=deck[index1];
        deck[index1]=deck[index2];
        deck[index2]=tmp;
    }
}

void createTable()
{
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5 
	// Le joueur 2 possede les cartes d'indice 6,7,8 
	// Le joueur 3 possede les cartes d'indice 9,10,11 
	// Le coupable est la carte d'indice 12
	int i,j,c;

	for (i=0;i<4;i++)
		for (j=0;j<8;j++)
			tableCartes[i][j]=0;

	for (i=0;i<4;i++)
	{
		for (j=0;j<3;j++)
		{
			c=deck[i*3+j];
			switch (c)
			{
				case 0: // Sebastian Moran
					tableCartes[i][7]++;	// crane
					tableCartes[i][2]++;	// poing
					break;
				case 1: // Irene Adler
					tableCartes[i][7]++;	// crane
					tableCartes[i][1]++;	// ampoule
					tableCartes[i][5]++;	// collier
					break;
				case 2: // Inspector Lestrade
					tableCartes[i][3]++;	// couronne
					tableCartes[i][6]++;	// oeil
					tableCartes[i][4]++;	// carnet
					break;
				case 3: // Inspector Gregson 
					tableCartes[i][3]++;	// couronne
					tableCartes[i][2]++;	// poing
					tableCartes[i][4]++;	// carnet
					break;
				case 4: // Inspector Baynes 
					tableCartes[i][3]++;	// couronne
					tableCartes[i][1]++;	// ampoule
					break;
				case 5: // Inspector Bradstreet 
					tableCartes[i][3]++;	// couronne
					tableCartes[i][2]++;	// poing
					break;
				case 6: // Inspector Hopkins 
					tableCartes[i][3]++;	// couronne
					tableCartes[i][0]++;	// pipe
					tableCartes[i][6]++;	// oeil
					break;
				case 7: // Sherlock Holmes 
					tableCartes[i][0]++;	// pipe
					tableCartes[i][1]++;	// ampoule
					tableCartes[i][2]++;	// poing
					break;
				case 8: // John Watson 
					tableCartes[i][0]++;	// pipe
					tableCartes[i][6]++;	// oeil
					tableCartes[i][2]++;	// poing
					break;
				case 9: // Mycroft Holmes 
					tableCartes[i][0]++;	// pipe
					tableCartes[i][1]++;	// ampoule
					tableCartes[i][4]++;	// carnet
					break;
				case 10: // Mrs. Hudson 
					tableCartes[i][0]++;	// pipe
					tableCartes[i][5]++;	// collier
					break;
				case 11: // Mary Morstan 
					tableCartes[i][4]++;	// carnet
					tableCartes[i][5]++;	// collier
					break;
				case 12: // James Moriarty 
					tableCartes[i][7]++;	// crane
					tableCartes[i][1]++;	// ampoule
					break;
			}
		}
	}
} 

void printDeck()
{
        int i,j;

        for (i=0;i<13;i++)
                printf("%d %s\n",deck[i],nomcartes[deck[i]]);

	for (i=0;i<4;i++)
	{
		for (j=0;j<8;j++)
			printf("%2.2d ",tableCartes[i][j]);
		puts("");
	}
}

void printClients()
{
        int i;

        for (i=0;i<nbClients;i++)
                printf("%d: %s %5.5d %s\n",i,tcpClients[i].ipAddress,
                        tcpClients[i].port,
                        tcpClients[i].name);
}

int findClientByName(char *name)
{
        int i;

        for (i=0;i<nbClients;i++)
                if (strcmp(tcpClients[i].name,name)==0)
                        return i;
        return -1;
}

void sendMessageToClient(char *clientip,int clientport,char *mess)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname(clientip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(clientport);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
                printf("ERROR connecting\n");
                exit(1);
        }

        sprintf(buffer,"%s\n",mess);
        n = write(sockfd,buffer,strlen(buffer));

    close(sockfd);
}

void broadcastMessage(char *mess)
{
        int i;

        for (i=0;i<nbClients;i++)
                sendMessageToClient(tcpClients[i].ipAddress,
                        tcpClients[i].port,
                        mess);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
	int i;
	
    char com;
    char clientIpAddress[256], clientName[256];
    int clientPort;
    int id;
    char reply[256];


    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
     
    if (sockfd < 0) 
        error("ERROR opening socket");
     
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
     
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
     
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

	printDeck();
	melangerDeck();
	createTable();
	printDeck();
	joueurCourant=0;

	for (i=0;i<4;i++)
	{
        	strcpy(tcpClients[i].ipAddress,"localhost");
        	tcpClients[i].port=-1;
        	strcpy(tcpClients[i].name,"-");
	}

     while (1)
     {    
     	newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     	
     	if (newsockfd < 0) 
          	error("ERROR on accept");

     	bzero(buffer,256);
     	n = read(newsockfd,buffer,255);
     	
     	if (n < 0) 
			error("ERROR reading from socket");

        printf("Received packet from %s:%d\nData: [%s]\n\n",
                inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

        char* temp;
        if (fsmServer==0)
        {
        	switch (buffer[0])
        	{
                case 'C':
                	sscanf(buffer,"%c %s %d %s", &com, clientIpAddress, &clientPort, clientName);
                	printf("COM=%c ipAddress=%s port=%d name=%s\n",com, clientIpAddress, clientPort, clientName);

                	// fsmServer==0 alors j'attends les connexions de tous les joueurs
                	strcpy(tcpClients[nbClients].ipAddress,clientIpAddress);
                	tcpClients[nbClients].port=clientPort;
                	strcpy(tcpClients[nbClients].name,clientName);
                	nbClients++;

                	printClients();

					// rechercher l'id du joueur qui vient de se connecter

                	id=findClientByName(clientName);
                	printf("id=%d\n",id);

					// lui envoyer un message personnel pour lui communiquer son id

                	sprintf(reply,"I %d",id);
                	sendMessageToClient(tcpClients[id].ipAddress,
                        tcpClients[id].port,
                        reply);

					// Envoyer un message broadcast pour communiquer a tout le monde la liste des joueurs actuellement
					// connectes

                	sprintf(reply,"L %s %s %s %s", tcpClients[0].name, tcpClients[1].name, tcpClients[2].name, tcpClients[3].name);
                	broadcastMessage(reply);
	
					// Si le nombre de joueurs atteint 4, alors on peut lancer le jeu

	                if (nbClients==4)
					{
						// Envoie les cartes à tout le monde
						for (int i = 0; i < 4; ++i)		// Pour chaque joueur
						{
							sprintf(reply,"D");
					
							for (int j = 3*i; j < 3*i+3; ++j) // On envoie 3 cartes
							{
								sprintf(reply + strlen(reply), " %d", deck[j]);
							}
	
							for (int j = 0; j < 8; ++j)					// On envoie sa ligne tableCartes
							{
								sprintf(reply + strlen(reply), " %d", tableCartes[i][j]);
							}

	                        sendMessageToClient(tcpClients[i].ipAddress,
	                               tcpClients[i].port,
	                               reply);
						}

                        printf("%s est le premier a jouer\n", tcpClients[joueurCourant].name);

						sprintf(reply,"M %d", joueurCourant);
	                	broadcastMessage(reply);


                		fsmServer=1;
					}

					break;
            }
		}
		else if (fsmServer==1)
		{

            printf("%s vient de jouer\n", tcpClients[buffer[2] - '0'].name);


			switch (buffer[0])
			{
                	case 'G':
                    // Un joueur essaye de deviner le coupable
                    // Structure du message:
                	// O X Y  :  x-> id_client ; y-> coupable
                	// 01234

                	if ((buffer[4] - '0') == deck[12])
                	{
                		// gagne
                		printf("%s a gagne le jeu\n", tcpClients[buffer[2] - '0'].name);

                	}else
                	{
                		//perdu
                		printf("%s a accusé un inocent.\nIl a perdu et ne peut plus jouer\n", tcpClients[buffer[2] - '0'].name);
                		listeJoueurs[joueurCourant] = 0; // le joueur ne peut plus jouer

                	}

				break;
                	case 'O':
                    // Un joueur demande qui a un certain objet
                    // Structure du message:
                	// O X Y  :  x-> id_client ; y-> objet
                	// 01234

                	for (int i = 0; i < 4; ++i)
                	{
                		if (i != buffer[2] - '0') // tous repondent sauf celui qui demande
                		{
                			if (tableCartes[i][(buffer[4] - '0')])
                				sprintf(reply, "V %d %d 9", i, (buffer[4] - '0'));
                			else
                				sprintf(reply, "V %d %d 0", i, (buffer[4] - '0'));
			                broadcastMessage(reply);
                		}
                	}


				break;
			case 'S':
                    // Un joueur demande a quelqu'un combien il a de cartes avec un certain objet
					// O X Y Z  :  x-> id_client_qui_demande; y-> id_client_qui_repond ; z-> objet
                	// 0123456

    			sprintf(reply, "V %d %d %d", (buffer[4] - '0'), (buffer[6] - '0'), tableCartes[(buffer[4] - '0')][(buffer[6] - '0')]);
                sendMessageToClient(tcpClients[buffer[2] - '0'].ipAddress,
                       tcpClients[buffer[2] - '0'].port,
                       reply);                	
				break;
                	default:
                        	break;
			}

			if (++joueurCourant > 3)
				joueurCourant = 0;

			while(!listeJoueurs[joueurCourant]){
                if (++joueurCourant > 3)
					joueurCourant = 0;
			}

            printf("C'est au tour de %s\n", tcpClients[joueurCourant].name);
			sprintf(reply,"M %d", joueurCourant);
           	broadcastMessage(reply);
        }
     	close(newsockfd);
    }
    close(sockfd);
    return 0; 
}
