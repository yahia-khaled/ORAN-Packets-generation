#include <iostream>
#include <fstream>
#include "Ethernet.h"
#include "parser.h"


using namespace std;



int main(int argc, char ** argv)
{
    Ethernet_ORAN net;

    // net.ReadConfig("second_milstone.txt");
    // net.CreateOutput("output.txt");

    parse_file("output.txt");

    return 0;
}
