#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include "filereader.h"
#include "str.h"

using std::cin;
using std::cout;

int main()  {

	char buffer[ 1024 ] = {};		// Initialize empty buffer
	int count;						// Holds number of records to read
	filereader fp;					// File pointer
	long offset = 0;				// offset into file we are reading

	// Some hardcoded test data
	string name = "test.in";
	string line1 = "712412913|Healey|Christopher|CSC";
	string line2 = "123454321|Glenmoore|Alexander|MKT";
	int size = line1.len();
	fp.open(name, 'w');				// open file for writing
	fp.write_raw( (char*) &size, sizeof(int));		// write record length
	fp.write_raw(line1, line1.len());				// write record
	size = line2.len();
	fp.write_raw( (char*) &size, sizeof(int));		// write record length
	fp.write_raw(line2, line2.len());				// write record

	fp.close();										// close file

	fp.open(name, 'r');								// open file for reading
	fp.seek( offset, BEGIN);						// start at beginning
	while (1)  {
		fp.read_raw( (char *) &count, sizeof( int ));	// read next record size
		fp.read_raw(buffer, count);						// read record to buffer
		if (fp.eof())									// to prevent last line from printing twice
			break;										//   the lazy way
		cout << buffer << "\n";							// print buffer contents
	}

	return 0;
}