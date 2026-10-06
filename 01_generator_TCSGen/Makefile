CC = g++ -std=c++17 -fPIC
ROOT_CFLAGS = $(shell root-config --cflags)
ROOT_LIBS   = $(shell root-config --libs)

TARGET = Test1.exe
LIBDIR = lib
LIB = $(LIBDIR)/libTest1.so

all: $(TARGET)

$(LIB): TTCSKine.o KinFuncs.o TTCSCrs.o GPDs.o
	mkdir -p $(LIBDIR)
	$(CC) -shared -o $(LIB) TTCSKine.o KinFuncs.o TTCSCrs.o GPDs.o

$(TARGET): src/MonteCarlo_GammaRho_Generator.cpp $(LIB)
	$(CC) src/MonteCarlo_GammaRho_Generator.cpp -I ./include -L./lib -lTest1 -Wl,-rpath,'$$ORIGIN/lib' $(ROOT_CFLAGS) $(ROOT_LIBS) -o $(TARGET)

TTCSKine.o:
	$(CC) -c src/TTCSKine.cc -I ./include $(ROOT_CFLAGS)

KinFuncs.o:
	$(CC) -c src/KinFunctions.cc -I ./include $(ROOT_CFLAGS) -o KinFuncs.o

TTCSCrs.o:
	$(CC) -c src/TTCSCrs.cc -I ./include $(ROOT_CFLAGS)

GPDs.o:
	$(CC) -c src/GPDs.cc -I ./include $(ROOT_CFLAGS)

clean:
	rm -f *.o Test1.exe
	rm -rf lib
