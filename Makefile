all:
	gcc -g -shared -fPIC drprobe.c -o libdrprobe.so

test:
	gcc -L. -ldrprobe -g -shared -fPIC drprobe.c -o libdrprobe.so

doc:
	echo "<html><head></head><body>" > README.html
	markdown README >> README.html; 
	echo "</body></html>" >> README.html

cleandoc:
	rm README.html

clean:
	rm libdrprobe.so

cleantest:
	rm drp_test

cleanall:
	-rm README.html
	-rm libdrprobe.so
	-rm drp_test
