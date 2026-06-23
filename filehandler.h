#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>
using namespace std;

// M1: handles all reading/writing of txt files

vector<vector<string> > readTXT(const string &filename);
void writeTXT(const string &filename, const vector<string> &header, const vector<vector<string> > &data);
void appendTXT(const string &filename, const vector<string> &row);
vector<string> findRow(const string &filename, int colIndex, const string &value);
bool rowExists(const string &filename, int colIndex, const string &value);

// helper string functions
vector<string> splitLine(const string &line);     // splits one CSV line into fields
string joinFields(const vector<string> &fields);  // joins fields back into one CSV line
string trimStr(const string &s);                  // removes spaces/\r from a string

#endif
