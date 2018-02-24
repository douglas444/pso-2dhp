# Particle Swarm Optimization 2DHP

### Executable pso-2dhp
###### How to compile
```
$ gcc -Wall -O2  -c ./file.c -o ./obj/Release/file.o
$ gcc -Wall -O2  -c ./main.c -o ./obj/Release/main.o
$ gcc -Wall -O2  -c ./pso.c -o ./obj/Release/pso.o
$ g++  -o ./bin/Release/pso-2dhp ./obj/Release/file.o ./obj/Release/main.o ./obj/Release/pso.o  -s
```
###### How to run
```
$ [sudo] ./bin/Release/pso-2dhp input_file_path sequence_key_on_input_file
```
###### Output
```
iterations|protein|directions|energy|final population average|final population standard deviation|convergence|found on iteration|time|iteration,energy/.../iteration,energy/
```

### Bash script.sh
###### How to run
```
$ [sudo] chmod +x ./script.sh
$ [sudo] ./script.sh
```
###### Output
```
./results/outputlog
./results/summary
./results/figures/sequence[1..8]
./results/tables/sequence[1..8]
./results/energy_evolution/sequence[1..8]/run[1..20]
```
###### Generate pdf/png from latex files:
```
$ pdflatex --shell-escape filename
```
