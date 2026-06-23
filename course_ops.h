#ifndef COURSE_OPS_H
#define COURSE_OPS_H

#include <string>
#include <vector>
using namespace std;

// M3: courses.txt -> code(0), name(1), credits(2), instructor(3), capacity(4), enrolled(5), prereq(6)
// enrollments.txt -> id(0), roll(1), course(2), semester(3), date(4), status(5)

const string COURSES_FILE = "data/courses.txt";
const string ENROLLMENTS_FILE = "data/enrollments.txt";

const int CR_CODE = 0;
const int CR_NAME = 1;
const int CR_CREDITS = 2;
const int CR_INSTRUCTOR = 3;
const int CR_CAPACITY = 4;
const int CR_ENROLLED = 5;
const int CR_PREREQ = 6;

const int EN_ID = 0;
const int EN_ROLL = 1;
const int EN_COURSE = 2;
const int EN_SEM = 3;
const int EN_DATE = 4;
const int EN_STATUS = 5;

const int MAX_CREDIT_LOAD = 21;

enum EnrollResult {
    ENROLL_SUCCESS,
    ENROLL_STUDENT_INACTIVE,
    ENROLL_COURSE_NOT_FOUND,
    ENROLL_NO_SEATS,
    ENROLL_ALREADY_ENROLLED,
    ENROLL_CREDIT_OVERLOAD,
    ENROLL_PREREQ_NOT_MET
};

string enrollResultMessage(EnrollResult result);
bool courseExists(const string &code);
int getCreditLoad(const string &roll, int semester);
bool checkPrerequisite(const string &roll, const string &code);
EnrollResult enrollStudent(const string &roll, const string &code, int semester);
bool dropCourse(const string &roll, const string &code, int semester);
vector<vector<string> > listEnrolledStudents(const string &code);

// BONUS feature for course codes: types one character at a time and
// reprints the matching course list live (by code OR name, substr/length
// only). Returns whatever was typed when Enter is pressed, "" if Esc.
string liveTypeCourseCode(const string &label);

#endif
