all: receiver sender
receiver: receiver.c	
	gcc -Wall -o receiver receiver.c
sender: sender.c	
	gcc -Wall -o sender sender.c
.PHONY: clean
clean:
	rm receiver sender
