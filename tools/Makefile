all: write_mbr ext2_reader

write_mbr: write_mbr.c
	gcc write_mbr.c -o write_mbr

ext2_reader: ext2/*.c
	gcc -g $< -o $@
