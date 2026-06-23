#ifndef REPORTS_H
#define REPORTS_H

#include <string>
using namespace std;

// M7: pulls grades/attendance/fee data into formatted console reports

void printMeritList();                 // active students sorted by CGPA desc
void printAttendanceDefaulters();      // any course attendance below 75%
void printFeeDefaulters();             // outstanding fees, sorted by amount
void printSemesterResult(const string &roll, int semester);
void printDepartmentSummary();         // count/avgCGPA/passRate per dept
void exportReportToFile(int reportChoice, const string &outFilename);

#endif
