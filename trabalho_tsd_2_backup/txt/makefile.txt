all: temp

temp: temp.o smpl.o rand.o cisj.o
	$(LINK.c) -o $@ -Bstatic temp.o smpl.o rand.o cisj.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

cisj.o: cisj.c cisj.h
	$(COMPILE.c)  -g cisj.c

nbndD12.o: temp.c smpl.h
	$(COMPILE.c) -g  temp.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o temp relat saida

