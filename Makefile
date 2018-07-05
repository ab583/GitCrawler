INC=-I/usr/include -I/usr/include/cppconn
LIB=-lmysqlcppconn -lcurlpp -lcurl -pthread
CPPFLAGS=$(INC) -c -std=c++11

all: config.o databaseio.o gitcrawler.o handler.o langs_file.o language.o main.o misc.o statistics.o tokenizer.o
	g++ config.o databaseio.o gitcrawler.o handler.o langs_file.o language.o main.o misc.o statistics.o tokenizer.o $(LIB) -o langsTest

config.o: config.cpp config.h
	g++ config.cpp $(CPPFLAGS)

databaseio.o: databaseio.cpp databaseio.h
	g++ databaseio.cpp $(CPPFLAGS)

gitcrawler.o: gitcrawler.cpp gitcrawler.h
	g++ gitcrawler.cpp $(CPPFLAGS)

handler.o: handler.cpp handler.h
	g++ handler.cpp $(CPPFLAGS)

langs_file.o: langs_file.cpp langs_file.h
	g++ langs_file.cpp $(CPPFLAGS)

language.o: language.cpp language.h
	g++ language.cpp $(CPPFLAGS)

main.o: main.cpp
	g++ main.cpp $(CPPFLAGS)

misc.o: misc.cpp misc.h
	g++ misc.cpp $(CPPFLAGS)

statistics.o: statistics.cpp statistics.h
	g++ statistics.cpp $(CPPFLAGS)

tokenizer.o: tokenizer.cpp tokenizer.h
	g++ tokenizer.cpp $(CPPFLAGS)

clean:
	rm -f *.o
