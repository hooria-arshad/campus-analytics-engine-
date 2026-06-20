#ifndef FILEHANDLER_H
#define FILEHANDLER_H
#include <string>
#include <vector>
using namespace std;
// M1 - filehandler.h
// functions instead of opening files directly.
// Reads a whole CSV-style txt file. Skips the header row.
// Each row becomes a vector<string> of fields. Returns all rows as a
// vector of vector<string> (a simple 2D table).
vector<vector<string> > readTXT(const string &filename);

// Overwrites the file: writes the header row first, then every row in
// comma, it is wrapped in double quotes.
void writeTXT(const string &filename, const vector<string> &header,
              const vector<vector<string> > &data);

// Appends one row to the end of the file without re-reading/re-writing
// the whole file.
void appendTXT(const string &filename, const vector<string> &row);

// Linear search: returns the first row where row[colIndex] == value.
// Returns an empty vector<string> if nothing is found.
vector<string> findRow(const string &filename, int colIndex, const string &value);

// Returns true if any row in the file has row[colIndex] == value.
bool rowExists(const string &filename, int colIndex, const string &value);

// ---- small string helpers used across modules ----

// Splits one CSV line into fields using a character-by-character loop
// (handles quoted fields that contain commas). No getline-based split.
vector<string> splitLine(const string &line);

// Joins fields back into one CSV line, quoting fields that contain commas.
string joinFields(const vector<string> &fields);

// Trims \r and surrounding spaces from a string (txt files from Windows
// often have trailing \r).
string trimStr(const string &s);

#endif
