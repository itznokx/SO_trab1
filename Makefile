all: main.out
	@echo "compiled"
main.out:
	clear
	g++ atendimento.cpp -Wall -Wextra
clean:
	rm *.out