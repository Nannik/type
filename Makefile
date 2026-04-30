watch-run:
	ls *.c | entr -r sh -c 'clear && make run'

watch-build:
	ls *.c | entr -r sh -c 'clear && make build'

run: build
	./out

build: $(wildcard *.c)
	gcc -o out main.c

clean:
	rm out
