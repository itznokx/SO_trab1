all: main.out
	@echo "compiled"
main.out:
	clear
	gcc -o3 atendimento.c -o atendimento.out
clean:
	rm *.out