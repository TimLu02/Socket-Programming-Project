all: client.cpp serverM.cpp serverS.cpp serverL.cpp serverH.cpp
	g++ -std=c++11 -o client client.cpp
	g++ -std=c++11 -o serverM serverM.cpp
	g++ -std=c++11 -o serverL serverL.cpp
	g++ -std=c++11 -o serverS serverS.cpp
	g++ -std=c++11 -o serverH serverH.cpp
clean: 
	$(RM) client
	$(RM) serverH
	$(RM) serverL
	$(RM) serverS
	$(RM) serverM