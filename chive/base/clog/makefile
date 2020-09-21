vlog:test.o LogControl.o
	g++ test.o -g LogControl.o -o vlog -pthread -std=c++17
test.o:test.cc 
	g++ -c -g test.cc -o test.o -std=c++17
LogControl.o:LogControl.cc
	g++ -c -g LogControl.cc -o LogControl.o -std=c++17
clean:
	rm *.o  -f
	rm vlog -f