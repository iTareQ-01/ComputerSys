# Usage:
# make        # compile all binary
# make clean  # remove ALL binaries and objects

# target : pre-requisites
# <TAB> recipe

#It is not necessary for the target to be a file; it could be just a name for the recipe, we call these phony targets

# % equal target name
# $< equal pre-requisites
# $@ used to match the target name

.PHONY: all clean

#CC = gcc			# compiler to use

#LINKERFLAG = -lpam

SRCS := $(wildcard *.c)
BINS := $(SRCS:%.c=%)

all: generate ${BINS}

generate:
	@echo "Creating required files..."
	sudo apt-get install libpam0g-dev
	if [ ! -e /etc/pam.d/pam_auth_local_users ]; then \
		touch /etc/pam.d/pam_auth_local_users; \
	fi
	@echo "auth        required       pam_unix.so \naccount     required       pam_unix.so \npassword    required       pam_unix.so" > /etc/pam.d/pam_auth_local_users


#%: %.o
#	@echo "Checking.."
#	${CC} ${LINKERFLAG} $< -o $@

#%.o: %.c
#	@echo "Creating object.."
#	${CC} -c $<

server_socket_bash: server_socket_bash.c
	@echo "Checking server_socket_bash.o .."
	gcc server_socket_bash.c -o server_socket_bash -lpam
	
#server_socket_bash.o: server_socket_bash.c
#	@echo "Creating object server_socket_bash .."
#	${CC} -c server_socket_bash.c
	
client_socket_bash: client_socket_bash.o
	@echo "Checking client_socket_bash.o .."
	gcc client_socket_bash.o -o client_socket_bash
	
client_socket_bash.o: client_socket_bash.c
	@echo "Creating object client_socket_bash .."
	gcc -c client_socket_bash.c

clean:
	@echo "Cleaning up..."
	rm -rvf *.o ${BINS}
