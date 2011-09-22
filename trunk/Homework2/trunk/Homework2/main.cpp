#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "filereader.h"
#include "str.h"
#include <list>
#include <algorithm>

using std::cin;
using std::cout;
using std::getline;
using std::list;
using std::sort;
using std::ifstream;

const char DELIM = '|';					// Global constant delimiter in written file
const string INDEX_FILE = "index.db";	// Global constant for index database
const string AVAIL_FILE = "avail.db";	// Global constant for availability database

struct file_index {
	int key;				// record key
	long off;				// offset into main file
	
};

struct avail_list {
	int size;			// size of hole in file
	long off;			// offset into main file
};

void openFiles(string name, filereader &fp, filereader &index, filereader &available);
void addToFile(string str, filereader &fp, list<file_index> &index, list<avail_list> &available);
void readSupportStructures(filereader &index_file, list<file_index> &index_list,
			filereader &available_file, list<avail_list> &available_list);
list<int> findDelimiters(string str, char DELIM);
int findOffset(list<avail_list> available, int rlen);
bool file_test(const char *filename);
int fileSize(const char* name);
bool index_cmp(const file_index& a, const file_index& b);

int main(int argc, char *argv[])  {

	filereader fp;						// reader for database file
	filereader index_file;				// reader for index file
	filereader available_file;			// reader for availability list
	string name = argv[1];				// passed in file name
	list<file_index> index_list;		// index list for fast PK location
	list<avail_list> available_list;	// availability list of location of free space in db
	char buffer[ 1024 ] = {};			// Initialize empty buffer
	long offset = 0;					// offset into file we are reading
	string cmd = "";					// string for command
	std::string contents = "";			// std string so we can use "getline" from cin

	openFiles(name, fp, index_file, available_file);
	readSupportStructures(index_file, index_list, available_file, available_list);

	while (1)  {							// till we need to quit...
		cout << "Please input command:  ";
		getline(cin, contents, '\n');		// get line contents
		string c2 = contents.c_str();		// make a non-std string to tokenize
		string token[5] = {};				// array of strings to hold tokens
		c2.token(token,5);					// tokenize the string into the array
		if (token[0] == "add")  {				 
			addToFile(token[1], fp, index_list, available_list);	// add record to db file
		}
		else if (token[0] == "find")  {
			cout << "FIND -- You entered " << token[1] << "\n";		// find the requested record
		}
		else if (token[0] == "del")  {
			cout << "DEL -- You entered " << token[1] << "\n";		// delete the requested record
		}
		else if (token[0] == "end")  {								// quit the program
			cout << "Terminating program\n\n";
				break;
		}
		else
			cout << "Please enter a valid command.\n\n";
	}
	
	index_file.open(INDEX_FILE, 'w');
	available_file.open(AVAIL_FILE, 'w');
	index_file.seek(0, BEGIN);
	available_file.seek(0, BEGIN);
	// Now we need to write the contents of the index file.  We can use the same loop construct to 
	//		write the availability list to the file.  Since we are writing them in binary, when we 
	//		read in the index we can just read the key, offset pairs without using any delimiters.
	for (list<file_index>::const_iterator i = index_list.begin(); i != index_list.end(); i++)  {
		index_file.write_raw( (char*) &i->key, sizeof(int));
		index_file.write_raw( (char*) &i->off, sizeof(long));
	}

	for (list<avail_list>::const_iterator i = available_list.begin(); i != available_list.end(); i++)  {
		available_file.write_raw( (char*) &i->off, sizeof(long));
		available_file.write_raw( (char*) &i->size, sizeof(int));
	}

	// Close all file pointers
	fp.close();
	index_file.close();
	available_file.close();
	return 0;
}

void addToFile(string str, filereader &fp, list<file_index> &index, list<avail_list> &available) {
	
	list<int> offsets = findDelimiters(str, DELIM);			// get list of delimiter positions in the current string
	int str_len = str.len();								// get length of the string
	string id;
	if (offsets.empty())									// if there are no delimiters in the string
		id = str;											// it must be a record ID
	else													// otherwise
		id = str.substr(0, offsets.front() - 1);			// get ID field from passed in string
	file_index add;											// create file_index to add to list
	int rlen = sizeof (int) + str_len;						// get total size of record to add to file
	if ( index.empty() || available.empty() )  {			// if both lists are empty
//		cout << "offset at add position is " << fp.offset() << "\n";	// the file is either empty of there are no holes
		fp.write_raw( (char*) &str_len, sizeof (int));					// we will add to the end of the file because we are in append mode
		fp.write_raw(str, str_len);										// do raw writes
		add.off = fp.offset() - rlen;									// set values of struct
		add.key = id;													// set values of struct
		index.push_back(add);											// add struct to in memory index list
		index.sort(index_cmp);											// keep index sorted
//		cout << "Added to index " << index.back().key << "\n";
//		cout << "Index or Available was empty\n";
	}
	else {													// index and avail list info exists
		int new_off = findOffset(available, rlen);			// calculate offset ****TO IMPLEMENT*****
		fp.seek(new_off, BEGIN);							// seek to offset to write to
		fp.write_raw( (char*) &str_len, sizeof (int) );		// perform raw writes
		fp.write_raw(str, str_len);							// perform raw writes
		add.off = new_off;									// set values of struct
		add.key = id;										// set values of struct
		index.push_back(add);								// add struct to in memory availablity list
		//index.sort();										// sort index for fast lookups
		cout << "Index or Available list were not empty\n";
	}
//	cout << "Leaving addToFile...\n\n";
}

list<int> findDelimiters(string str, char DELIM)  {

	list<int> offsets;
	for (int i = 0; i < str.len(); i++)  {
		if (str[i] == DELIM)
			offsets.push_back(i);
	}
	return offsets;
}

int findOffset(list<avail_list> available, int rlen) {
	// ****TO IMPLEMENT****
	// Must find the "memory hole" that will accomodate rlen
	// and add any remaining available space to the end of the list
	return 1;
}

bool file_test(const char *filename)  {
  ifstream file(filename);			// test file existence
  return file;
}

void openFiles(string name, filereader &fp, filereader &index, filereader &available)  {

	if (file_test(name))  {			// Method for opening files or creating them.
		fp.open(name, 'a');
		cout << name << " was opened for writing\n";
	}
	else  {
		fp.open(name, 'w');
		cout << name << " was CREATED for writing\n";
	}
	
	if (file_test(INDEX_FILE))  {
		index.open(INDEX_FILE, 'r');
		cout << "Index.db was opened.\n";
	}
	else  {
		index.open(INDEX_FILE, 'w');
		cout << "Index.db was CREATED.\n";
	}

	if (file_test(AVAIL_FILE))  {
		available.open(AVAIL_FILE, 'r');
		cout << "Avail.db was opened.\n";
	}
	else  {
		available.open(AVAIL_FILE, 'w');
		cout << "Avail.db was CREATED.\n";
	}
}

void readSupportStructures(filereader &index_file, list<file_index> &index_list, 
	filereader &available_file, list<avail_list> &available_list)  {

	file_index findex;							// structure to add to list
	avail_list alist;							// structure to add to list
	int index_end = fileSize(INDEX_FILE);		// file size of index file
	int avail_end = fileSize(AVAIL_FILE);		// file size of availability list file
	int growth = sizeof(int) + sizeof(long);	// offset growth of two data elements.
	int key = 0;								// key value to read
	int off = 0;								// offset value to read
	int size = 0;								// size value to read

	for (int i = 0; i < index_end; i += growth)  {
		index_file.read_raw((char*) &key, sizeof(int));		// read key value
		index_file.read_raw((char*) &off, sizeof(long));	// read offset value
//		cout << "Adding Key: " << key << "\t\tOffset: " << off << "\n";
		findex.key = key;									// set struct value for in memory list
		findex.off = off;									// set struct value for in memory list
		index_list.push_back(findex);						// add struct to list
	}

	for (int i = 0; i < avail_end; i += growth)  {
		available_file.read_raw((char*) &off, sizeof(int));		// read offset value
		available_file.read_raw((char*) &size, sizeof(long));	// read size value
		alist.off = off;										// set struct value for in memory list
		alist.size = size;										// set struct value for in memory list
		available_list.push_back(alist);						// add struct to list
//		cout << "Added record " << i << " to availability list\n";
	}
	index_file.close();
	available_file.close();

	// DEBUG CODE -- prints values added to the lists
	for (list<file_index>::const_iterator i = index_list.begin(); i != index_list.end(); i++)  {
		cout << "Read Key: " << i->key << "\t\tRead Offset: " << i->off << "\n";
	}

	for (list<avail_list>::const_iterator i = available_list.begin(); i != available_list.end(); i++)  {
		cout << "Read Offset: " << i->off << "\t\tRead Size: " << i->size << "\n";
	}
	// END DEBUG
}

int fileSize(const char* name) {
  ifstream file;								// create stream object
  file.open(name, std::ios_base::binary | std::ios_base::in);	// open file in binary mode
  if (!file.good() || file.eof() || !file.is_open())			// if there is a problem
	  return 0;													// report zero size
  file.seekg(0, std::ios_base::beg);							// otherwise go to beginning
  ifstream::pos_type begin_pos = file.tellg();					// record position
  file.seekg(0, std::ios_base::end);							// seek to end
  return static_cast<int>(file.tellg() - begin_pos);			// return difference of positions
}

bool index_cmp(const file_index& a, const file_index& b)  {
	    return a.key < b.key;
}
