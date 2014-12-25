
all:
	g++ -g -L ./leveldb-1.9.0/ -lleveldb  -I./leveldb-1.9.0/include -lzmq zedis.cpp -o zedis
	g++ -g -L ./leveldb-1.9.0/ -lleveldb  -I./leveldb-1.9.0/include -lzmq zedis_cli.cpp -o zedis_cli
clean:
	rm zedis_cli zedis
