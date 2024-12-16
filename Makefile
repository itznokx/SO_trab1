all: main.out
	@echo "compiled"
main.out:
	make clean
	clear
	g++ atendimento.cpp -Wall -Wextra
clean:
	rm *.out