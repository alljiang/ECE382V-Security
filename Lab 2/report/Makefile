all: main.ps
pdf: main.pdf

main.ps: *.tex *.eps *.bib 
	rubber --ps main

clean:
	rm -f main.dvi main.ps main.pdf main.aux main.log main.blg main.bbl

main.pdf: main.ps
	rubber --ps --pdf main
