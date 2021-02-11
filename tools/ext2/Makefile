all: ext2_reader

ext2_reader: *.c
	gcc -g $< -o $@

run: ext2_reader
	./ext2_reader

clean:
	rm -rf *.o
