build:
	mpic++ tema.cpp -o main -lm
run:
	mpirun -oversubscribe -np 5 main text.txt
clear:
	rm main
