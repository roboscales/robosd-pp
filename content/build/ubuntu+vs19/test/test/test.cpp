#include <iostream>
#include "core/robosd_app.hpp"
using namespace std;

int main(int argc, char *argv[])
{
	robo::app::machine::begin("a.ini");
	char sz[] = "Hello, World!";	//Hover mouse over "sz" while debugging to see its contents
	cout << sz << endl;	//<================= Put a breakpoint here
	return 0;
}