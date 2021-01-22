sshell: sshell.c
	gcc -o sshell sshell.c -Wall -Werror -Wextra

clean:
	-rm -f *.o sshell
