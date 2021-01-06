CC = gcc
CCFLAGS = -I/usr/include/SDL2 
LIBFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lpthread
CLIENT = sh13
SERVEUR = server

all: $(SERVEUR) $(CLIENT)

$(CLIENT):
	$(CC) -o $@ $(CCFLAGS) $@.c $(LIBFLAGS)  

$(SERVEUR):
	$(CC) -o $@ $@.c  

full: clean all
	gnome-terminal -x sh -c "./sh13 localhost 32000 localhost 32001 client_1; bash"
	gnome-terminal -x sh -c "./sh13 localhost 32000 localhost 32002 client_2; bash"
	gnome-terminal -x sh -c "./sh13 localhost 32000 localhost 32003 client_3; bash"
	gnome-terminal -x sh -c "./sh13 localhost 32000 localhost 32004 client_4; bash"
	./server 32000
	
	
clean:
	rm -f $(CLIENT) $(SERVEUR)
