#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <string>
#include <vector>
using namespace std;

// M4: attendance_log.txt -> id(0), roll(1), course(2), date(3), status(4)
// status: P = present, A = absent, L = late

const string ATTENDANCE_FILE = "data/attendance_log.txt";

const int AT_ID = 0;
const int AT_ROLL = 1;
const int AT_COURSE = 2;
const int AT_DATE = 3;
const int AT_STATUS = 4;

void markAttendance(const string &code, int semester, const string &date);
double getAttendancePct(const string &roll, const string &code);
vector<vector<string> > getShortageList();   // students below 75% attendance
bool undoLastSession();                       // restores file from backup
void printDailySheet(const string &code, const string &date);

#endif
