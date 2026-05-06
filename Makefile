SRC = $(wildcard *.c)

watch-run:
	ls *.c | entr -r sh -c 'clear && make run'

watch-build:
	ls *.c | entr -r sh -c 'clear && make build'

run: build
	./out

build: $(wildcard *.c)
	gcc -o out $(SRC)

clean:
	rm out

install:
	cp out /usr/local/bin/ttype
