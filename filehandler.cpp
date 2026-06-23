#include "filehandler.h"
#include <fstream>
#include <iostream>
using namespace std;

// removes spaces, \r, \n, \t from start and end
string trimStr(const string &s) {
    int start = 0;
    int end = (int)s.length() - 1;
    while (start <= end && (s[start] == ' ' || s[start] == '\r' || s[start] == '\n' || s[start] == '\t'))
        start++;
    while (end >= start && (s[end] == ' ' || s[end] == '\r' || s[end] == '\n' || s[end] == '\t'))
        end--;
    if (end < start) return "";
    return s.substr(start, end - start + 1);
}

// splits a CSV line into fields, char by char (handles "quoted, commas")
vector<string> splitLine(const string &lineRaw) {
    string line = trimStr(lineRaw);
    vector<string> fields;
    string current = "";
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

// joins fields with commas, wraps a field in quotes if it has a comma inside
string joinFields(const vector<string> &fields) {
    string result = "";
    for (size_t i = 0; i < fields.size(); i++) {
        string f = fields[i];
        bool needsQuotes = false;
        for (size_t j = 0; j < f.length(); j++) {
            if (f[j] == ',') { needsQuotes = true; break; }
        }
        if (needsQuotes) result += "\"" + f + "\"";
        else result += f;
        if (i != fields.size() - 1) result += ",";
    }
    return result;
}

// reads whole file into rows of fields, skips header row
vector<vector<string> > readTXT(const string &filename) {
    vector<vector<string> > rows;
    ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        cout << "Error: could not open file " << filename << " for reading.\n";
        return rows;
    }

    string line;
    bool isHeader = true;
    while (getline(fin, line)) {
        if (trimStr(line).empty()) continue;
        if (isHeader) { isHeader = false; continue; }
        rows.push_back(splitLine(line));
    }
    fin.close();
    return rows;
}

// overwrites the file with header + all given rows
void writeTXT(const string &filename, const vector<string> &header,
              const vector<vector<string> > &data) {
    ofstream fout(filename.c_str());
    if (!fout.is_open()) {
        cout << "Error: could not open file " << filename << " for writing.\n";
        return;
    }
    fout << joinFields(header) << "\n";
    for (size_t i = 0; i < data.size(); i++) fout << joinFields(data[i]) << "\n";
    fout.close();
}

// adds one row to the end of the file
void appendTXT(const string &filename, const vector<string> &row) {
    ofstream fout(filename.c_str(), ios::app);
    if (!fout.is_open()) {
        cout << "Error: could not open file " << filename << " for appending.\n";
        return;
    }
    fout << joinFields(row) << "\n";
    fout.close();
}

// linear search for the first row matching value at colIndex
vector<string> findRow(const string &filename, int colIndex, const string &value) {
    vector<vector<string> > rows = readTXT(filename);
    for (size_t i = 0; i < rows.size(); i++) {
        if (colIndex < (int)rows[i].size() && rows[i][colIndex] == value) return rows[i];
    }
    vector<string> empty;
    return empty;
}

bool rowExists(const string &filename, int colIndex, const string &value) {
    vector<vector<string> > rows = readTXT(filename);
    for (size_t i = 0; i < rows.size(); i++) {
        if (colIndex < (int)rows[i].size() && rows[i][colIndex] == value) return true;
    }
    return false;
}
