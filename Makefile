spin: spin.c
	gcc spin.c -o spin -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wall -Wextra

clean:
	rm -rf spin 
