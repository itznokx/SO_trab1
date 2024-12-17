all: main.out
	@echo "compiled"
main.out:
	clear
	g++ atendimento.cpp -Wall -Wextra -o3
clean:
	rm *.out