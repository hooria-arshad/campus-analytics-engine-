#ifndef STUDENT_OPS_H
#define STUDENT_OPS_H

#include <string>
#include <vector>
using namespace std;

// M2: students.txt columns -> roll(0), name(1), dept(2), semester(3), cgpa(4), status(5)

const string STUDENTS_FILE = "data/students.txt";

const int ST_ROLL = 0;
const int ST_NAME = 1;
const int ST_DEPT = 2;
const int ST_SEM  = 3;
const int ST_CGPA = 4;
const int ST_STATUS = 5;

bool isValidRollFormat(const string &roll);   // checks BSAI-YY-XXX format
bool nameHasDigits(const string &name);

bool addStudent(const string &roll, const string &name, const string &dept, int semester, double cgpa);
vector<string> searchByRoll(const string &roll);
bool updateStudent(const string &roll, int colIndex, const string &newValue);
bool softDelete(const string &roll);              // sets status to inactive
vector<vector<string> > listActiveStudents();     // sorted by roll (selection sort)

bool studentExists(const string &roll);
bool isStudentActive(const string &roll);

// BONUS feature, used everywhere a roll number needs to be typed in.
// Reads one character at a time and reprints the matching student list
// after every keystroke (live filtering by roll OR name, substr/length
// only). Returns whatever text the user finished typing when they press
// Enter, or "" if they press Esc to cancel.
string liveTypeRoll(const string &label);

// Same live-typing idea, but used only when ADDING a new student: it
// keeps looping and warns immediately if the roll being typed already
// belongs to an existing student, so the same roll can't be added twice.
// Returns a valid, non-duplicate roll, or "" if the user cancels with Esc.
string getNewStudentRoll();

// Search by Name menu option - shows the full live-typing list (by name
// or roll), used purely for browsing/searching, no duplicate checking.
vector<vector<string> > searchByName();

#endif
