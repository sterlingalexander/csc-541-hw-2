#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include "str.h"
#include <list>
#include <algorithm>
#include <vector>
#include <iomanip>

using std::cin;
using std::cout;
using std::getline;
using std::list;
using std::vector;
using std::sort;
using std::ifstream;
using std::fstream;
using std::ios;
using std::ios_base;

const char DELIM = '|';					// Global constant delimiter in written file
std::string INDEX_FILE = "index";	// Global constant for index database
std::string AVAIL_FILE = "avail";	// Global constant for availability database

struct file_index {
	int key;				// record key
	long off;				// offset into main file	
};

struct avail_list {
	int size;			// size of hole in file
	long off;			// offset into main file
};

void openFiles(string name, fstream &fp, fstream &index, fstream &available);
void addToFile(string str, fstream &fp, vector<file_index> &index, vector<avail_list> &available);
void readSupportStructures(fstream &index_file, vector<file_index> &index_list,
			fstream &available_file, vector<avail_list> &available_list);
list<int> findDelimiters(string str, char DELIM);
int findOffset(vector<avail_list> &available, int rlen);
bool fileTest(const char *filename);
int fileSize(const char* name);
int fileSize(fstream &fp);
bool indexCmp(const file_index& a, const file_index& b);
int find(const vector<file_index> &index, const string &target);
void printRecord(const int &v_index, fstream &fp, vector<file_index> index, string str);
void deleteRecord(const int &v_index, fstream &fp, vector<file_index> &index, vector<avail_list> &available_list, string lookup);

int main(int argc, char *argv[])  {

	if (argc == 1)  {
		cout << "Please enter a database name to open";
		exit(0);
	}

	fstream fp;						// reader for database file
	fstream index_file;				// reader for index file
	fstream available_file;			// reader for availability list
	string name = argv[1];				// passed in file name
	vector<file_index> index_list;		// index list for fast PK location
	vector<avail_list> available_list;	// availability list of location of free space in db
	char buffer[ 1024 ] = {};			// Initialize empty buffer
	long offset = 0;					// offset into file we are reading
	string cmd = "";					// string for command
	std::string contents = "";			// std string so we can use "getline" from cin
	std::string n_str = name;
	INDEX_FILE += "_" + n_str + ".db";
	AVAIL_FILE += "_" + n_str + ".db";

	openFiles(name, fp, index_file, available_file);		// open all files
	readSupportStructures(index_file, index_list, available_file, available_list); // read any persistent structures

	while (1)  {							// till we need to quit...
		getline(cin, contents, '\n');		// get line contents
		string c2 = contents.c_str();		// make a non-std string to tokenize
		string token[5] = {};				// array of strings to hold tokens
		c2.token(token,5);					// tokenize the string into the array
		if (token[0] == "add")  {				 
			addToFile(token[1], fp, index_list, available_list);	// add record to db file
		}
		else if (token[0] == "find")  {
			printRecord(find(index_list, token[1]), fp, index_list, token[1]);	// find the requested record
		}
		else if (token[0] == "del")  {
			deleteRecord(find(index_list, token[1]), fp, index_list, available_list, token[1]);	// delete requested record
		}
		else if (token[0] == "end")  {								// quit the program
			break;
		}
		else
			cout << "Please enter a valid command.\n\n";
	}
	
	index_file.seekp(0, ios::beg);					// put pointers at the beginning of file
	available_file.seekp(0, ios::beg);				// this one too

	// Now we need to write the contents of the index file.  We can use the same loop construct to 
	//		write the availability vector to the file.  Since we are writing them in binary, when we 
	//		read in the index we can just read the key, offset pairs without using any delimiters.
	cout << "Index:\n";
	for (vector<file_index>::const_iterator i = index_list.begin(); i != index_list.end(); i++)  {
		index_file.write( (char*) &i->key, sizeof(int));		// write key to file
		index_file.write( (char*) &i->off, sizeof(long));		// write offset to file
		printf( "%d: %ld\n", i->key, i->off );						// write output to screen
	}
	cout << "Availability:\n";
	for (vector<avail_list>::const_iterator i = available_list.begin(); i != available_list.end(); i++)  {
		available_file.write( (char*) &i->off, sizeof(long));	// write offset to file
		available_file.write( (char*) &i->size, sizeof(int));	// write size of hole to file
		printf( "%d: %ld\n", i->off, i->size);						// write output to screen
	}

	// Flush and close all file pointers
	fp.flush();
	fp.close();
	index_file.flush();
	index_file.close();
	available_file.flush();
	available_file.close();
	return 0;
}

void deleteRecord(const int &v_index, fstream &fp, vector<file_index> &index, 
	vector<avail_list> &available_list, string lookup)  {

	avail_list rec;					// struct to hold available memory space info

	if (v_index < 0)				// if there is no index into the vector, there is no record
			cout << "No record with SID=" << lookup << " exists.\n";
	else  {
		int offset = index[v_index].off;		// retreive offset of record
		int rec_len = 0;						// length of record to read
		fp.seekg(offset, ios::beg);				// set pointer to correct file position
		fp.read((char*) &rec_len, sizeof(int));	// read record size
		rec_len += sizeof(int);					// add info for record size storage
		rec.off = offset;						// record offset of hole
		rec.size = rec_len;						// record size of hole
		available_list.push_back(rec);			// add hole to vector
		index.erase(index.begin() + v_index);	// erase index entry for erased record
	}
}

void printRecord(const int &v_index, fstream &fp, vector<file_index> index, string lookup)  {

	if (v_index < 0)							// if vector index is negative, the record is not there
			cout << "No record with SID=" << lookup << " exists\n";
	else  {
		int rec_len = 0;							// length of record to read
		long offset = (long) index[v_index].off;	// retreive offset of record
		char str[512] = {};							// char buffer for text
		fp.seekg(offset, ios::beg);					// set pointer to correct file position
		fp.read((char*) &rec_len, sizeof(int));		// read record size
		fp.read(str, rec_len);						// read stored string from file
		cout << str << "\n";						// print record to screen
	}
}

int find(const vector<file_index> &index, const string &target)  {

	// Standard Binary Search Implementation
	int left = 0;
	int right = index.size() - 1;
	int mid = right / 2;
	int t_int = atoi(target);
	while ( left <= right )  {
		if (t_int == index[mid].key)  {
			return mid;
		}
		else if (t_int > index[mid].key)  {
			left = mid + 1;
			mid = left + ((right - left) / 2);
		}
		else if (t_int < index[mid].key)  {
			right = mid - 1;
			mid = left + ((right - left) / 2);
		}
	}
	return -1;		// return impossible value if key doesn't exist
}

void addToFile(string str, fstream &fp, vector<file_index> &index, vector<avail_list> &available) {
	
	list<int> offsets = findDelimiters(str, DELIM);		// get list of delimiter positions in the current string
	int str_len = str.len();							// get length of the string
	string id;
	if (offsets.empty())								// if there are no delimiters in the string
		id = str;										// it must be a record ID
	else												// otherwise
		id = str.substr(0, offsets.front() - 1);		// get ID field from passed in string
	if (find(index, id) != -1)  {						// if the key already exists, just return.
		return;						// THE SPEC SPECIFICALLY STATES THAT THERE ARE NO UPDATES
	}								//	TO UPDATE, THE USER MUST DO A DELETE AND ADD
	file_index add;										// create file_index to add to vector
	int rlen = sizeof (int) + str_len;					// get total size of record to add to file
	if ( index.empty() || available.empty() )  {		// if both vectors are empty
		fp.write( (char*) &str_len, sizeof (int));		// we will add to the end of the file because we are in append mode
		fp.write(str, str_len);							// do raw writes
		add.off = (int) fp.tellp() - rlen;				// set values of struct
		add.key = id;									// set values of struct
		index.push_back(add);							// add struct to in memory index vector
		sort(index.begin(), index.end(), indexCmp);		// keep index sorted
	}
	else {												// if index and avail vector info exists
		int new_off = findOffset(available, rlen);		// calculate offset
		if (new_off > 0)  {								// if there is a memory hole large enough for the record
		fp.seekp(new_off, ios::beg);					// seek to offset to write to
		fp.write( (char*) &str_len, sizeof (int) );		// perform raw writes
		fp.write(str, str_len);							// perform raw writes
		add.off = new_off;								// set values of struct
		add.key = id;									// set values of struct
		index.push_back(add);							// add struct to in memory availablity vector
		sort(index.begin(), index.end(), indexCmp);		// sort index for fast lookups
		}
		else  {												// if there is no hole large enough, write to the end
			add.off = fileSize(fp);							// set values of struct
			add.key = id;									// set values of struct
			fp.seekp(0, ios::end);							// seek to offset to write to
			fp.write( (char*) &str_len, sizeof (int) );		// perform raw writes
			fp.write(str, str_len);							// perform raw writes
			index.push_back(add);							// add struct to in memory availablity vector
			sort(index.begin(), index.end(), indexCmp);		// sort index for fast lookups
		}
	}
}

list<int> findDelimiters(string str, char DELIM)  {

	list<int> offsets;						// list of delimiter offsets
	for (int i = 0; i < str.len(); i++)  {	// iterate string
		if (str[i] == DELIM)				// find delimiters
			offsets.push_back(i);			// add them to list
	}
	return offsets;							// return list
}

int findOffset(vector<avail_list> &available, int rlen) {

	for (int i = 0; i < available.size(); i++)  {		// iterate over the vector
		if (available[i].size == rlen)  {				// if this record equals the needed size
			int off = available[i].off;					// record the offset
			available.erase(available.begin() + i);		// erase the availablity record
			return off;									// return the offset
		}
		else if (available[i].size > rlen)  {			// if the record is larger than needed
			int off = available[i].off;					// record offset
			int residual = available[i].size - rlen;	// record residual space
			long residual_offset = off + rlen;			// record residual offset
			avail_list rec;								// create struct for residual record
			rec.off = residual_offset;					// set struct data
			rec.size = residual;						// set struct data
			available.erase(available.begin() + i);		// erase original hole
			available.push_back(rec);					// add residual
			return off;									// return offset
		}
		else {}				// memory hold is smaller than record to add, go on to the next one 
	}
	return -1;				// failure returns impossible value
}

bool fileTest(const char *filename)  {
  ifstream file(filename);			// test file existence
  return file;
}

void openFiles(string name, fstream &fp, fstream &index, fstream &available)  {

	if (fileTest(name))									// if the file exists
		fp.open(name, ios::binary|ios::in|ios::out);	// open it
	else  {									// otherwise
		fstream t(name);					// create temporary fstream object
		t.open(name, ios::out);				// cause file to be created
		t.flush();							// flush to disk
		t.close();							// close temp file object
		fp.open(name, ios::binary|ios::in|ios::out);	// then open the file for I/O
	}

	if (fileTest(INDEX_FILE.c_str()))							// if the file exists
		index.open(INDEX_FILE, ios::binary|ios::in|ios::out);	// open it
	else  {									// otherwise
		fstream t(INDEX_FILE);				// create temporary fstream object
		t.open(INDEX_FILE, ios::out);		// cause file to be created
		t.flush();							// flush to disk
		t.close();							// close temp file object
		index.open(INDEX_FILE, ios::binary|ios::in|ios::out);	// then open file for I/O
	}

	if (fileTest(AVAIL_FILE.c_str()))								// if file exists
		available.open(AVAIL_FILE, ios::binary|ios::in|ios::out);	// open it
	else  {									// otherwise
		fstream t(AVAIL_FILE);				// create temporary fstream object
		t.open(AVAIL_FILE, ios::out);		// cause file to be created
		t.flush();							// flush to disk
		t.close();							// close temp file object
		available.open(AVAIL_FILE), ios::binary|ios::in|ios::out;	// then open file for I/O
	}
}

void readSupportStructures(fstream &index_file, vector<file_index> &index_list, 
	fstream &available_file, vector<avail_list> &available_list)  {

	file_index findex;							// structure to add to vector
	avail_list alist;							// structure to add to vector
	int index_end = fileSize(INDEX_FILE.c_str());		// file size of index file
	int avail_end = fileSize(AVAIL_FILE.c_str());		// file size of availability vector file
	int growth = sizeof(int) + sizeof(long);	// offset growth of two data elements.
	int key = 0;								// key value to read
	int off = 0;								// offset value to read
	int size = 0;								// size value to read

	for (int i = 0; i < index_end; i += growth)  {		// iterate over index.db
		index_file.read((char*) &key, sizeof(int));		// read key value
		index_file.read((char*) &off, sizeof(long));	// read offset value
		findex.key = key;								// set struct value for in memory vector
		findex.off = off;								// set struct value for in memory vector
		index_list.push_back(findex);					// add struct to vector
	}

	for (int i = 0; i < avail_end; i += growth)  {			// iterate over avail.db
		available_file.read((char*) &off, sizeof(int));		// read offset value
		available_file.read((char*) &size, sizeof(long));	// read size value
		alist.off = off;										// set struct value for in memory vector
		alist.size = size;										// set struct value for in memory vector
		available_list.push_back(alist);						// add struct to vector
	}
}

int fileSize(fstream &fp)  {
	fp.seekg(0, ios::beg);								// otherwise go to beginning
	ifstream::pos_type begin_pos = fp.tellg();			// record position
	fp.seekg(0, ios::end);								// seek to end
	return static_cast<int>(fp.tellg() - begin_pos);	// return difference of positions
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

bool indexCmp(const file_index& a, const file_index& b)  {
	// This method defines the "<" operator for our index struct and allows for "sort" 
	//		to be called on vectors.  If we wanted to sort the avail list, we would need to do the same
	//		but I don't think we are supposed to sort avail.
	return a.key < b.key;		
}
