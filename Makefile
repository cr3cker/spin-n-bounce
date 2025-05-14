spin: spin.c
	gcc spin.c -o spin -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

clean:
	rm -rf spin 
