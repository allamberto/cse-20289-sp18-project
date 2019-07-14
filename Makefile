CC=		gcc
CFLAGS=		-g -gdwarf-2 -Wall -Werror -std=gnu99
LD=		gcc
LDFLAGS=	-L.
AR=		ar
ARFLAGS=	rcs
TARGETS=	spidey

all:		$(TARGETS)

forking.o: forking.c spidey.h
	@echo "Compiling forking.o..."
	@$(CC) $(CFLAGS) -c -o forking.o forking.c

handler.o: handler.c spidey.h
	@echo "Compiling handler.o..."
	@$(CC) $(CFLAGS) -c -o handler.o handler.c

request.o: request.c spidey.h
	@echo "Compiling request.o..."
	@$(CC) $(CFLAGS) -c -o request.o request.c

single.o: single.c spidey.h
	@echo "Compiling single.o..."
	@$(CC) $(CFLAGS) -c -o single.o single.c

socket.o: socket.c spidey.h
	@echo "Compiling socket.o..."
	@$(CC) $(CFLAGS) -c -o socket.o socket.c

spidey.o: spidey.c spidey.h
	@echo "Compiling spidey.o..."
	@$(CC) $(CFLAGS) -c -o spidey.o spidey.c

utils.o: utils.c spidey.h
	@echo "Compiling utils.o..."
	@$(CC) $(CFLAGS) -c -o utils.o utils.c

spidey: spidey.h forking.o handler.o request.o single.o socket.o spidey.o utils.o
	@echo "Linking spidey..."
	@$(CC) $(LDFLAGS) -o spidey forking.o handler.o request.o single.o socket.o spidey.o utils.o

test: test-thor test-spidey

test-thor:
	@curl -sLO https://gitlab.com/nd-cse-20289-sp18/cse-20289-sp18-project/raw/master/test_thor.sh
	@chmod +x test_thor.sh
	@./test_thor.sh

test-spidey:
	@curl -sLO https://gitlab.com/nd-cse-20289-sp18/cse-20289-sp18-project/raw/master/test_spidey.sh
	@chmod +x test_spidey.sh
	@./test_spidey.sh

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) *.o *.log *.input test*
