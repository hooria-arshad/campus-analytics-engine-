#include "reports.h"
#include "filehandler.h"
#include "student_ops.h"
#include "course_ops.h"
#include "attendance.h"
#include "fee_tracker.h"
#include "grades.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
using namespace std;

void printMeritList() {
    vector<vector<string> > students = listActiveStudents();

    // selection sort by CGPA, highest first
    int n = (int)students.size();
    for (int i = 0; i < n - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < n; j++) {
            double cgpaJ = atof(students[j][ST_CGPA].c_str());
            double cgpaMax = atof(students[maxIdx][ST_CGPA].c_str());
            if (cgpaJ > cgpaMax) maxIdx = j;
        }
        if (maxIdx != i) {
            vector<string> tmp = students[i]; students[i] = students[maxIdx]; students[maxIdx] = tmp;
        }
    }

    cout << "\n================= MERIT LIST =================\n";
    cout << setw(6) << left << "Rank" << setw(14) << left << "Roll No"
         << setw(22) << left << "Name" << setw(6) << left << "CGPA" << "\n";
    cout << "------------------------------------------------\n";
    for (int i = 0; i < n; i++) {
        cout << setw(6) << left << (i + 1)
             << setw(14) << left << students[i][ST_ROLL]
             << setw(22) << left << students[i][ST_NAME]
             << setw(6) << left << students[i][ST_CGPA] << "\n";
    }
    cout << "================================================\n";
}

void printAttendanceDefaulters() {
    vector<vector<string> > shortage = getShortageList();

    cout << "\n============ ATTENDANCE DEFAULTERS ============\n";
    cout << setw(14) << left << "Roll No" << setw(22) << left << "Name"
         << setw(10) << left << "Course" << setw(8) << left << "Pct%" << "\n";
    cout << "------------------------------------------------\n";
    if (shortage.empty()) cout << "(no attendance defaulters found)\n";
    for (size_t i = 0; i < shortage.size(); i++) {
        cout << setw(14) << left << shortage[i][0]
             << setw(22) << left << shortage[i][1]
             << setw(10) << left << shortage[i][2]
             << setw(8) << left << shortage[i][3] << "\n";
    }
    cout << "================================================\n";
}

void printFeeDefaulters() {
    vector<vector<string> > defaulters = getDefaulters();

    cout << "\n=============== FEE DEFAULTERS ================\n";
    cout << setw(14) << left << "Roll No" << setw(12) << left << "Outstanding"
         << setw(14) << left << "Weeks Overdue" << "\n";
    cout << "------------------------------------------------\n";
    if (defaulters.empty()) cout << "(no fee defaulters found)\n";
    for (size_t i = 0; i < defaulters.size(); i++) {
        double totalFee = atof(defaulters[i][FE_TOTAL].c_str());
        double paid = atof(defaulters[i][FE_PAID].c_str());
        double balance = totalFee - paid;

        string dueDate = defaulters[i][FE_DUE];
        int weeksOverdue = 0;
        if (defaulters[i][FE_STATUS] == "paid_late") {
            int diff = daysBetween(dueDate, defaulters[i][FE_PAYDATE]);
            weeksOverdue = diff / 7;
        }

        cout << setw(14) << left << defaulters[i][FE_ROLL]
             << setw(12) << left << fixed << setprecision(2) << balance
             << setw(14) << left << weeksOverdue << "\n";
    }
    cout << "================================================\n";
}

void printSemesterResult(const string &roll, int semester) {
    vector<string> studentRow = searchByRoll(roll);
    if (studentRow.empty()) {
        cout << "Error: student " << roll << " not found.\n";
        return;
    }

    ostringstream semStream; semStream << semester;
    vector<vector<string> > grades = readTXT(GRADES_FILE);
    double gpa = computeGPA(roll, semester);

    cout << "\n================ SEMESTER RESULT ================\n";
    cout << "Roll No : " << roll << "\n";
    cout << "Name    : " << studentRow[ST_NAME] << "\n";
    cout << "Semester: " << semester << "\n";
    cout << "--------------------------------------------------\n";
    cout << setw(10) << left << "Course" << setw(8) << left << "Total"
         << setw(8) << left << "Grade" << setw(10) << left << "Attend%" << "\n";

    for (size_t i = 0; i < grades.size(); i++) {
        if (grades[i][GR_ROLL] == roll && grades[i][GR_SEM] == semStream.str()) {
            double pct = getAttendancePct(roll, grades[i][GR_COURSE]);
            cout << setw(10) << left << grades[i][GR_COURSE]
                 << setw(8) << left << grades[i][GR_TOTAL]
                 << setw(8) << left << grades[i][GR_LETTER]
                 << setw(10) << left << fixed << setprecision(2) << pct << "\n";
        }
    }
    cout << "--------------------------------------------------\n";
    cout << "Semester GPA: " << fixed << setprecision(2) << gpa << "\n";
    cout << "==================================================\n";
}

void printDepartmentSummary() {
    vector<vector<string> > students = readTXT(STUDENTS_FILE);

    // parallel arrays group students by department (no map allowed)
    string deptNames[50];
    int deptCount[50];
    double deptCgpaSum[50];
    int deptPassCount[50];
    int numDepts = 0;

    for (size_t i = 0; i < students.size(); i++) {
        string dept = students[i][ST_DEPT];
        double cgpa = atof(students[i][ST_CGPA].c_str());

        int foundIdx = -1;
        for (int d = 0; d < numDepts; d++) if (deptNames[d] == dept) { foundIdx = d; break; }

        if (foundIdx == -1) {
            deptNames[numDepts] = dept;
            deptCount[numDepts] = 0;
            deptCgpaSum[numDepts] = 0.0;
            deptPassCount[numDepts] = 0;
            foundIdx = numDepts;
            numDepts++;
        }

        deptCount[foundIdx]++;
        deptCgpaSum[foundIdx] += cgpa;
        if (cgpa >= 2.0) deptPassCount[foundIdx]++;
    }

    cout << "\n============ DEPARTMENT SUMMARY ============\n";
    cout << setw(28) << left << "Department" << setw(8) << left << "Count"
         << setw(10) << left << "AvgCGPA" << setw(10) << left << "Pass%" << "\n";
    cout << "----------------------------------------------\n";
    for (int d = 0; d < numDepts; d++) {
        double avgCgpa = (deptCount[d] > 0) ? (deptCgpaSum[d] / deptCount[d]) : 0.0;
        double passRate = (deptCount[d] > 0) ? ((double)deptPassCount[d] / deptCount[d] * 100.0) : 0.0;
        cout << setw(28) << left << deptNames[d]
             << setw(8) << left << deptCount[d]
             << setw(10) << left << fixed << setprecision(2) << avgCgpa
             << setw(10) << left << fixed << setprecision(2) << passRate << "\n";
    }
    cout << "================================================\n";
}

void exportReportToFile(int reportChoice, const string &outFilename) {
    ofstream fout(outFilename.c_str());
    if (!fout.is_open()) {
        cout << "Error: could not open " << outFilename << " for writing.\n";
        return;
    }

    streambuf *originalCout = cout.rdbuf();
    cout.rdbuf(fout.rdbuf()); // redirect cout to the file

    switch (reportChoice) {
        case 1: printMeritList(); break;
        case 2: printAttendanceDefaulters(); break;
        case 3: printFeeDefaulters(); break;
        case 4: printDepartmentSummary(); break;
        default: cout << "Invalid report choice.\n"; break;
    }

    cout.rdbuf(originalCout); // back to normal console
    fout.close();
    cout << "Report exported to " << outFilename << "\n";
}
