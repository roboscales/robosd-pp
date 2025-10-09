#include <iostream>
#include "core/robosd_app.hpp"
using namespace std;

int main(int argc, char *argv[])
{
	// robo::app::machine::begin("a.ini");
	// void *instance = robo::system::lib::load("+actuator-libactuator.alfa.so-0");
	// const char *nm = "libtest-2.so";
	// const char *nm = "+actuator-libactuator.alfa.so-0";
	const char *nm = "libactuator.alfa.so";
	void *instance = robo::system::lib::load(nm);	
	//void *instance = robo::system::lib::load("libedev.so");
	//void *instance = robo::system::lib::load("librobosd-core++.so");
	char sz[] = "Hello, World!";	//Hover mouse over "sz" while debugging to see its contents
	cout << sz << (uint64_t)instance << nm <<endl;	//<================= Put a breakpoint here
	return 0;
}