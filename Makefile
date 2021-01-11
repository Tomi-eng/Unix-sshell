all: sshell.c
	cc -c sshell.c -Wall -Werror -Wextra

clean:
	-rm -f *.o sshell