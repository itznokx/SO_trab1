all: main.out
	@echo "compiled"
main.out:
	clear
	g++ -o3 -o a.out atendimento.cpp
clean:
	rm *.out