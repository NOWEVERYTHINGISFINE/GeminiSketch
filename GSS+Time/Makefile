all : gss-time

.PHONY : all
CXX = g++
CFLAGS = -lpthread -static-libstdc++ -std=c++11


gss-time : gss-time.o
	$(CXX) -o gss-time gss-time.o $(CFLAGS)

gss-time.o : main.cpp QueryFunction.h HashFunction.h Gsstime.h params.h
	$(CXX) -o gss-time.o -c main.cpp -D HINT
	

.PHONY:clean
clean:
	-$(RM) gss-time
	-$(RM) gss-time.o