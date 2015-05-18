
python: clean gentext.out

gentext.out: GenText.c
	g++ $< -o $@ -g -loolm -ldstruct -lmisc -lz -lgomp -lflm -llattice -lpthread -L/share/tools/srilm-1.7.0/lib/i686-m64 -I/share/tools/srilm-1.7.0/lm/src -I/share/tools/srilm-1.7.0/include
clean:
	\rm -fr *.out
