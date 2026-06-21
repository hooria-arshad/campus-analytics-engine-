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

// BONUS feature is built right into this function: instead of asking for a
// full name and searching once, it reads one character at a time and
// reprints the matching list after every keystroke (live filtering).
// Returns the list of students that matched when the user finished typing.
vector<vector<string> > searchByName();

bool updateStudent(const string &roll, int colIndex, const string &newValue);
bool softDelete(const string &roll);              // sets status to inactive
vector<vector<string> > listActiveStudents();     // sorted by roll (selection sort)

bool studentExists(const string &roll);
bool isStudentActive(const string &roll);

#endif
