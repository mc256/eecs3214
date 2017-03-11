main: server.hpp server.cpp client.hpp client.cpp mc_util.hpp mc_util.cpp
	gcc server.hpp server.cpp mc_util.hpp mc_util.cpp -lstdc++ -lpthread -o server.out
	gcc client.hpp client.cpp mc_util.hpp mc_util.cpp -lstdc++ -lpthread -o client.out
	#gcc test.cpp -lstdc++ -lpthread -o test.out