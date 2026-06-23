#include "attendance.h"
#include "filehandler.h"
#include "course_ops.h"
#include "student_ops.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
using namespace std;

// snapshot of the file taken right before a new session, used to undo
static vector<vector<string> > backupSnapshot;
static bool hasBackup = false;

static vector<string> attendanceHeader() {
    vector<string> header;
    header.push_back("log_id"); header.push_back("roll_no"); header.push_back("course_code");
    header.push_back("session_date"); header.push_back("status");
    return header;
}

void markAttendance(const string &code, int semester, const string &date) {
    vector<vector<string> > enrolled = listEnrolledStudents(code);
    if (enrolled.empty()) {
        cout << "No active students enrolled in " << code << ".\n";
        return;
    }

    backupSnapshot = readTXT(ATTENDANCE_FILE); // back up before writing
    hasBackup = true;

    vector<vector<string> > existing = readTXT(ATTENDANCE_FILE);
    int maxNum = 0;
    for (size_t i = 0; i < existing.size(); i++) {
        string idStr = existing[i][AT_ID];
        if (idStr.length() > 1) {
            int num = atoi(idStr.substr(1).c_str());
            if (num > maxNum) maxNum = num;
        }
    }

    for (size_t i = 0; i < enrolled.size(); i++) {
        string roll = enrolled[i][EN_ROLL];
        vector<string> studentRow = searchByRoll(roll);
        string name = studentRow.empty() ? roll : studentRow[ST_NAME];

        char status = ' ';
        while (status != 'P' && status != 'A' && status != 'L') {
            cout << "Attendance for " << roll << " (" << name << ") in " << code << " [P/A/L]: ";
            cin >> status;
            if (status >= 'a' && status <= 'z') status = status - 'a' + 'A';
        }

        maxNum++;
        ostringstream idStream;
        idStream << "L";
        if (maxNum < 10) idStream << "0000";
        else if (maxNum < 100) idStream << "000";
        else if (maxNum < 1000) idStream << "00";
        else if (maxNum < 10000) idStream << "0";
        idStream << maxNum;

        vector<string> row;
        row.push_back(idStream.str());
        row.push_back(roll);
        row.push_back(code);
        row.push_back(date);
        row.push_back(string(1, status));
        appendTXT(ATTENDANCE_FILE, row);
    }
    cout << "Attendance recorded for " << code << " on " << date << ".\n";
}

// (present + 0.5*late) / total * 100
double getAttendancePct(const string &roll, const string &code) {
    vector<vector<string> > rows = readTXT(ATTENDANCE_FILE);
    double presentCount = 0.0;
    int total = 0;

    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i][AT_ROLL] == roll && rows[i][AT_COURSE] == code) {
            total++;
            if (rows[i][AT_STATUS] == "P") presentCount += 1.0;
            else if (rows[i][AT_STATUS] == "L") presentCount += 0.5;
        }
    }
    if (total == 0) return 0.0;
    return (presentCount / (double)total) * 100.0;
}

vector<vector<string> > getShortageList() {
    vector<vector<string> > result;
    vector<vector<string> > students = listActiveStudents();
    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE);

    for (size_t i = 0; i < students.size(); i++) {
        string roll = students[i][ST_ROLL];
        for (size_t j = 0; j < enrollments.size(); j++) {
            if (enrollments[j][EN_ROLL] == roll && enrollments[j][EN_STATUS] == "active") {
                string code = enrollments[j][EN_COURSE];
                double pct = getAttendancePct(roll, code);
                if (pct < 75.0) {
                    vector<string> row;
                    row.push_back(roll);
                    row.push_back(students[i][ST_NAME]);
                    row.push_back(code);
                    ostringstream pctStream; pctStream << fixed << setprecision(2) << pct;
                    row.push_back(pctStream.str());
                    result.push_back(row);
                }
            }
        }
    }
    return result;
}

bool undoLastSession() {
    if (!hasBackup) {
        cout << "No previous session to undo.\n";
        return false;
    }
    writeTXT(ATTENDANCE_FILE, attendanceHeader(), backupSnapshot);
    hasBackup = false;
    cout << "Last attendance session undone.\n";
    return true;
}

void printDailySheet(const string &code, const string &date) {
    vector<vector<string> > rows = readTXT(ATTENDANCE_FILE);

    cout << "\n+------------+----------------------+------------+--------+\n";
    cout << "| Roll No    | Name                 | Course     | Status |\n";
    cout << "+------------+----------------------+------------+--------+\n";

    bool any = false;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i][AT_COURSE] == code && rows[i][AT_DATE] == date) {
            any = true;
            vector<string> studentRow = searchByRoll(rows[i][AT_ROLL]);
            string name = studentRow.empty() ? "" : studentRow[ST_NAME];
            cout << "| " << setw(10) << left << rows[i][AT_ROLL]
                 << " | " << setw(20) << left << name
                 << " | " << setw(10) << left << code
                 << " | " << setw(6) << left << rows[i][AT_STATUS] << " |\n";
        }
    }
    if (!any) cout << "| (no attendance records found for this date)               |\n";
    cout << "+------------+----------------------+------------+--------+\n";
}
