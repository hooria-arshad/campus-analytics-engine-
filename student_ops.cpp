#include "student_ops.h"
#include "filehandler.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

// roll must look like BSAI-YY-XXX (11 chars total)
bool isValidRollFormat(const string &roll) {
    if (roll.length() != 11) return false;
    if (roll.substr(0, 5) != "BSAI-") return false;
    if (roll[7] != '-') return false;

    string yy = roll.substr(5, 2);
    for (size_t i = 0; i < yy.length(); i++)
        if (yy[i] < '0' || yy[i] > '9') return false;

    string xxx = roll.substr(8, 3);
    for (size_t i = 0; i < xxx.length(); i++)
        if (xxx[i] < '0' || xxx[i] > '9') return false;

    return true;
}

bool nameHasDigits(const string &name) {
    for (size_t i = 0; i < name.length(); i++)
        if (name[i] >= '0' && name[i] <= '9') return true;
    return false;
}

bool studentExists(const string &roll) {
    return rowExists(STUDENTS_FILE, ST_ROLL, roll);
}

bool isStudentActive(const string &roll) {
    vector<string> row = findRow(STUDENTS_FILE, ST_ROLL, roll);
    if (row.empty()) return false;
    return row[ST_STATUS] == "active";
}

bool addStudent(const string &roll, const string &name, const string &dept,
                 int semester, double cgpa) {
    if (!isValidRollFormat(roll)) {
        cout << "Error: roll number must be in format BSAI-YY-XXX (e.g. BSAI-23-031)\n";
        return false;
    }
    if (studentExists(roll)) {
        cout << "Error: a student with roll " << roll << " already exists.\n";
        return false;
    }
    if (nameHasDigits(name)) {
        cout << "Error: name must not contain digits.\n";
        return false;
    }
    if (cgpa < 0.0 || cgpa > 4.0) {
        cout << "Error: CGPA must be between 0.0 and 4.0.\n";
        return false;
    }

    ostringstream cgpaStream;
    cgpaStream << fixed; cgpaStream.precision(2); cgpaStream << cgpa;
    ostringstream semStream; semStream << semester;

    vector<string> row;
    row.push_back(roll);
    row.push_back(name);
    row.push_back(dept);
    row.push_back(semStream.str());
    row.push_back(cgpaStream.str());
    row.push_back("active");

    appendTXT(STUDENTS_FILE, row);
    cout << "Student " << roll << " added successfully.\n";
    return true;
}

vector<string> searchByRoll(const string &roll) {
    return findRow(STUDENTS_FILE, ST_ROLL, roll);
}

bool updateStudent(const string &roll, int colIndex, const string &newValue) {
    if (colIndex == ST_ROLL) {
        cout << "Error: roll number cannot be updated.\n";
        return false;
    }
    vector<vector<string> > allRows = readTXT(STUDENTS_FILE);
    bool found = false;

    for (size_t i = 0; i < allRows.size(); i++) {
        if (allRows[i][ST_ROLL] == roll) {
            allRows[i][colIndex] = newValue;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Error: student " << roll << " not found.\n";
        return false;
    }

    vector<string> header;
    header.push_back("roll_no"); header.push_back("name"); header.push_back("department");
    header.push_back("semester"); header.push_back("cgpa"); header.push_back("status");
    writeTXT(STUDENTS_FILE, header, allRows);
    cout << "Student " << roll << " updated successfully.\n";
    return true;
}

bool softDelete(const string &roll) {
    return updateStudent(roll, ST_STATUS, "inactive");
}

// active students only, sorted by roll using selection sort
vector<vector<string> > listActiveStudents() {
    vector<vector<string> > allRows = readTXT(STUDENTS_FILE);
    vector<vector<string> > active;

    for (size_t i = 0; i < allRows.size(); i++)
        if (allRows[i][ST_STATUS] == "active") active.push_back(allRows[i]);

    int n = (int)active.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++)
            if (active[j][ST_ROLL] < active[minIdx][ST_ROLL]) minIdx = j;
        if (minIdx != i) {
            vector<string> temp = active[i];
            active[i] = active[minIdx];
            active[minIdx] = temp;
        }
    }
    return active;
}

// ---------------- Search By Name (search as you type) ----------------

// turns one character to uppercase manually, no <cctype> needed
static char toUpperChar(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}

// case-insensitive "does text start with prefix" check, substr/length only
static bool startsWithPrefix(const string &text, const string &prefix) {
    if (prefix.length() > text.length()) return false;
    for (size_t i = 0; i < prefix.length(); i++) {
        if (toUpperChar(text[i]) != toUpperChar(prefix[i])) return false;
    }
    return true;
}

// returns active students whose roll OR name starts with prefix
static vector<vector<string> > filterStudentsByPrefix(const string &prefix) {
    vector<vector<string> > active = listActiveStudents();
    vector<vector<string> > matches;

    for (size_t i = 0; i < active.size(); i++) {
        bool rollMatch = startsWithPrefix(active[i][ST_ROLL], prefix);
        bool nameMatch = startsWithPrefix(active[i][ST_NAME], prefix);
        if (rollMatch || nameMatch) matches.push_back(active[i]);
    }
    return matches;
}

static void printMatches(const string &prefix, const vector<vector<string> > &matches) {
    cout << "\nSearch by Name (type to filter live): \"" << prefix << "\"\n";
    cout << "(Backspace = delete, Enter = finish, Esc = cancel)\n";
    cout << "------------------------------------------------------------\n";
    if (matches.empty()) {
        cout << "(no matches)\n";
    } else {
        for (size_t i = 0; i < matches.size(); i++) {
            cout << matches[i][ST_ROLL] << " | " << matches[i][ST_NAME]
                 << " | " << matches[i][ST_DEPT] << " | CGPA " << matches[i][ST_CGPA] << "\n";
        }
    }
    cout << "------------------------------------------------------------\n";
}

#ifndef _WIN32
// reads one key without waiting for Enter (Linux/Mac version)
static int getCharRaw() {
    struct termios oldSettings, newSettings;
    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    return ch;
}
#endif

// this IS the "search by name" feature - it filters live as you type,
// one character at a time, instead of asking for a full name first
vector<vector<string> > searchByName() {
    string prefix = "";
    vector<vector<string> > matches = filterStudentsByPrefix(prefix);
    printMatches(prefix, matches);

    while (true) {
#ifdef _WIN32
        int ch = _getch();
#else
        int ch = getCharRaw();
#endif

        if (ch == 13 || ch == 10) { // Enter finishes the search
            cout << "\nSearch finished.\n";
            break;
        }
        if (ch == 27) { // Esc cancels
            cout << "\nSearch cancelled.\n";
            prefix = "";
            matches = filterStudentsByPrefix(prefix);
            break;
        }
        if (ch == 8 || ch == 127) { // Backspace
            if (!prefix.empty()) prefix = prefix.substr(0, prefix.length() - 1);
        } else if (ch >= 32 && ch <= 126) { // any normal printable character
            prefix += (char)ch;
        } else {
            continue; // ignore arrow keys etc.
        }

        matches = filterStudentsByPrefix(prefix);
        printMatches(prefix, matches);
    }
    return matches;
}
