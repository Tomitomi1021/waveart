TARGET=main.exe
SRC=main.c
OBJS=$(subst .c,.o,$(SRC))

LIBS=-lSDL2 -lSDL2main -lm
LINKOPTIONS= -L/mingw64/lib 
COMPILEOPTIONS= -O3 -I/mingw64/include

$(TARGET):$(OBJS)
	gcc  $(LINKOPTIONS) $^ $(LIBS) -o $@

.c.o:
	gcc $(COMPILEOPTIONS) $< -c -o $@

run:$(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJS)
