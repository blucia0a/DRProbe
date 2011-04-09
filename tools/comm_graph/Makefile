.PHONY: all clean cleantest cleanall

LIBS=-L../../drprobe -ldrprobe -lpthread 
TESTLIBS=-L. -L../../drprobe -ldrprobe -lpthread -ldrptcg

all:
	gcc -I../../drprobe $(LIBS) -g -shared -fPIC cg.c -o libdrptcg.so

test: all
	gcc cg_test.c $(TESTLIBS) -g -o cg_test 

clean:
	rm -f libdrptcg.so

cleantest:
	rm -f cg_test

cleanall: clean cleantest
