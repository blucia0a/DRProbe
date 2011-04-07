.PHONY: all doc cleandoc clean cleantest cleanall
all:
	gcc -g -shared -fPIC drprobe.c -o libdrprobe.so

test: all
	gcc -L. drp_test.c -ldrprobe -g -o drp_test

doc:
	echo "<html><head></head><body>" > README.html
	markdown README >> README.html; 
	echo "</body></html>" >> README.html

cleandoc:
	rm -f README.html

clean:
	rm -f libdrprobe.so

cleantest:
	rm -f drp_test

cleanall: cleandoc clean cleantest
