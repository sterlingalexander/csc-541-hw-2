#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "filereader.h"
#include "str.h"

using std::cin;
using std::cout;
using std::getline;

struct file_index {
	int key;			// record key
	long off;			// offset into main file
};

struct avail_list {
	long off;			// offset of hole in the file
	int size;			// size of hole in file
};

struct record {
	int sid;
	string l_name;
	string f_name;
	string dept;
};

int main(int argc, char *argv[])  {

	string name = argv[1];
	filereader fp;
	fp.open(name, 'w');
	cout << name << " was opened for writing\n";

	char buffer[ 1024 ] = {};		// Initialize empty buffer
	int count;						// Holds number of records to read
	long offset = 0;				// offset into file we are reading
	string cmd = "";
	std::string contents = "";
	while (1)  {
		cout << "Please input command:  ";
		getline(cin, contents, '\n');
		string c2 = contents.c_str();
		string token[5] = {};
		c2.token(token,5);
		if (token[0] == "add")  {
			cout << "ADD -- You entered " << token[1] << "\n";
		}
		else if (token[0] == "find")  {
			cout << "FIND -- You entered " << token[1] << "\n";
		}
		else if (token[0] == "del")  {
			cout << "DEL -- You entered " << token[1] << "\n";
		}
		else if (token[0] == "end")  {
			cout << "Terminating program\n\n";
				break;
		}
		else
			cout << "Please enter a valid command.\n\n";
	}

/*
	// Some hardcoded test data
	//string name = "test.in";
	string line1 = "712412913|Healey|Christopher|CSC";
	string line2 = "123454321|Glenmoore|Alexander|MKT";
	int size = line1.len();
//	fp.open(name, 'w');				// open file for writing
	fp.write_raw( (char*) &size, sizeof(int));		// write record length
	fp.write_raw(line1, line1.len());				// write record
	size = line2.len();
	fp.write_raw( (char*) &size, sizeof(int));		// write record length
	fp.write_raw(line2, line2.len());				// write record

	fp.close();
	fp.open(name, 'r');
	fp.seek( offset, BEGIN);						// start at beginning
	bool flag = true;
	while (1)  {
		if (fp.read_raw( (char *) &count, sizeof( int )))	// read next record size
			;	
		else
			break;
		if (fp.read_raw(buffer, count))						// read record to buffer
			;
		else
			break;
		cout << buffer << "\n";							// print buffer contents
	}

	fp.close();
*/
	return 0;
}
