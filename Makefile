SUBDIRS=debug_mod drprobe

all:
	for f in $(SUBDIRS); do make -C $$f; done
clean:
	for f in $(SUBDIRS); do make -C $$f clean; done
test:
	for f in $(SUBDIRS); do make -C $$f test; done
doc:
	echo "<html><body>" > README.html
	markdown README >> README.html
	echo "</body></html>" >> README.html
	
