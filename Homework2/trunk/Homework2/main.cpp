#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include "filereader.h"
#include "str.h"

using namespace std;

int main()  {

	cout << "Hello World";
	unsigned year = 974;

    // Save it as text
    ofstream outtxt( "textual.txt" );
    outtxt << year << flush;
    outtxt.close();

    // Save it as binary
    ofstream outbin( "binary.bin", ios::binary );
    outbin.write( reinterpret_cast <const char*> (&year), sizeof( year ) );
    outbin.close();
	return 0;
}
