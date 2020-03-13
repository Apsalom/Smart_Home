all:
	gcc interface.c application.c control.c decision.c -o execute -lpthread -lrt

