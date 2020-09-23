clog:test.o chiveLog.o
	g++ test.o -g chiveLog.o -o clog -pthread -std=c++17
	rm *.o -f
test.o:test.cc 
	g++ -c -g test.cc -o test.o -std=c++17
chiveLog.o:chiveLog.cc
	g++ -c -g chiveLog.cc -o chiveLog.o -std=c++17
clean:
	rm clog -f
