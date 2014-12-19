
AOCL_COMPILE_CONFIG=$(shell aocl compile-config)  
AOCL_LINK_CONFIG=$(shell aocl link-config)

host_prog: main.o 
	g++ -o host_prog main.o $(AOCL_LINK_CONFIG)

main.o: main.cpp
	g++ $(AOCL_COMPILE_CONFIG) -g -o $@ -c $< -ldl

dfa.aocx: dfa.cl
	aoc -v --report -march=emulator $<


run: dfa.aocx host_prog
	LD_PRELOAD=~/altera/14.0/hld/host/linux64/lib/libacl_emulator_kernel_rt.so LD_LIBRARY_PATH=~/altera/14.0/hld/linux64/lib/:/home/ec2-user/altera/14.0/hld/host/linux64/lib:/home/ec2-user/altera/14.0/hld/board/s5_ref/linux64/lib CL_CONTEXT_EMULATOR_DEVICE_ALTERA=s5_ref ./host_prog

clean:
	rm host_prog *.o

.PHONY: run clean
