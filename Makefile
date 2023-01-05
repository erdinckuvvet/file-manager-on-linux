all:
	gcc -o file_manager -lpthread file_manager.c
	gcc -o file_client  file_client.c