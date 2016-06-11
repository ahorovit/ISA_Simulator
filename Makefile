

# compiler 
CC=gcc

# this flag directs gcc to look in
# current directory
CFLAGS=-I.

# macro for object files
OBJ = ISS.o 

# general rule: all object files depend
# on their corresponding .c file
%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

# general rule: executable myBitApp depends on
# all files listed in OBJ: "%@" is macro for 
# left of colon, $^ is macro for right of :
simpleISS: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

# remove all files listed in OBJ as well as 
# executable
clean:
	rm -f $(OBJ) simpleISS
