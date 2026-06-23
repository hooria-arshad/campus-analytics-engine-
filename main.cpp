#include <iostream>
#include <limits>
#include "filehandler.h"
#include "student_ops.h"
#include "course_ops.h"
#include "attendance.h"
#include "grades.h"
#include "fee_tracker.h"
#include "reports.h"
using namespace std;
static void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int readInt(const string &prompt) {
    int value;
    cout << prompt;
    while (!(cin >> value)) {
        cout << "Invalid input, please enter a number: ";
        clearInput();
    }
    clearInput();
    return value;
}

static double readDouble(const string &prompt) {
    double value;
    cout << prompt;
    while (!(cin >> value)) {
        cout << "Invalid input, please enter a number: ";
        clearInput();
    }
    clearInput();
    return value;
}

static string readLine(const string &prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

void studentMenu() {
    int choice;
    do {
        cout << "\n----- STUDENT MANAGEMENT -----\n";
        cout << "1. Add Student\n2. Search by Roll\n3. Search by Name\n";
        cout << "4. Update Student\n5. Soft Delete Student\n6. List Active Students\n0. Back\n";
        choice = readInt("Choice: ");

        if (choice == 1) {
            // live-typing here also prevents adding the same roll twice
            string roll = getNewStudentRoll();
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            string name = readLine("Name: ");
            string dept = readLine("Department: ");
            int sem = readInt("Semester: ");
            double cgpa = readDouble("CGPA: ");
            addStudent(roll, name, dept, sem, cgpa);
        } else if (choice == 2) {
            string roll = liveTypeRoll("Search by Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            vector<string> row = searchByRoll(roll);
            if (row.empty()) cout << "Not found.\n";
            else cout << row[0] << " | " << row[1] << " | " << row[2] << " | " << row[4] << " | " << row[5] << "\n";
        } else if (choice == 3) {
            searchByName(); // live filtering happens inside this call
        } else if (choice == 4) {
            string roll = liveTypeRoll("Update Student - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            cout << "Fields: 1=name 2=dept 3=semester 4=cgpa 5=status\n";
            int field = readInt("Field to update: ");
            string newVal = readLine("New value: ");
            int colMap[5] = {ST_NAME, ST_DEPT, ST_SEM, ST_CGPA, ST_STATUS};
            if (field >= 1 && field <= 5) updateStudent(roll, colMap[field - 1], newVal);
            else cout << "Invalid field.\n";
        } else if (choice == 5) {
            string roll = liveTypeRoll("Soft Delete - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            softDelete(roll);
        } else if (choice == 6) {
            vector<vector<string> > rows = listActiveStudents();
            for (size_t i = 0; i < rows.size(); i++)
                cout << rows[i][0] << " | " << rows[i][1] << " | CGPA " << rows[i][4] << "\n";
        }
    } while (choice != 0);
}

void courseMenu() {
    int choice;
    do {
        cout << "\n----- COURSE MANAGEMENT -----\n";
        cout << "1. Enroll Student\n2. Drop Course\n3. Get Credit Load\n";
        cout << "4. List Enrolled Students\n0. Back\n";
        choice = readInt("Choice: ");

        if (choice == 1) {
            string roll = liveTypeRoll("Enroll - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            string code = liveTypeCourseCode("Enroll - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            EnrollResult result = enrollStudent(roll, code, sem);
            cout << enrollResultMessage(result) << "\n";
        } else if (choice == 2) {
            string roll = liveTypeRoll("Drop - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            string code = liveTypeCourseCode("Drop - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            dropCourse(roll, code, sem);
        } else if (choice == 3) {
            string roll = liveTypeRoll("Credit Load - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            cout << "Credit load: " << getCreditLoad(roll, sem) << " hrs\n";
        } else if (choice == 4) {
            string code = liveTypeCourseCode("List Enrolled - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            vector<vector<string> > rows = listEnrolledStudents(code);
            for (size_t i = 0; i < rows.size(); i++) cout << rows[i][EN_ROLL] << "\n";
        }
    } while (choice != 0);
}

void attendanceMenu() {
    int choice;
    do {
        cout << "\n----- ATTENDANCE -----\n";
        cout << "1. Mark Attendance\n2. Get Attendance %\n3. Shortage List\n";
        cout << "4. Undo Last Session\n5. Print Daily Sheet\n0. Back\n";
        choice = readInt("Choice: ");

        if (choice == 1) {
            string code = liveTypeCourseCode("Mark Attendance - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            string date = readLine("Date (DD-MM-YYYY): ");
            markAttendance(code, sem, date);
        } else if (choice == 2) {
            string roll = liveTypeRoll("Attendance % - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            string code = liveTypeCourseCode("Attendance % - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            cout << "Attendance %: " << getAttendancePct(roll, code) << "\n";
        } else if (choice == 3) {
            vector<vector<string> > rows = getShortageList();
            for (size_t i = 0; i < rows.size(); i++)
                cout << rows[i][0] << " " << rows[i][1] << " " << rows[i][2] << " " << rows[i][3] << "%\n";
        } else if (choice == 4) {
            undoLastSession();
        } else if (choice == 5) {
            string code = liveTypeCourseCode("Daily Sheet - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            string date = readLine("Date (DD-MM-YYYY): ");
            printDailySheet(code, date);
        }
    } while (choice != 0);
}

void gradesMenu() {
    int choice;
    do {
        cout << "\n----- GRADES -----\n";
        cout << "1. Enter Marks\n2. Compute GPA\n0. Back\n";
        choice = readInt("Choice: ");

        if (choice == 1) {
            string roll = liveTypeRoll("Enter Marks - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            string code = liveTypeCourseCode("Enter Marks - Course Code");
            if (code.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            int qCount = readInt("How many quiz marks (max 5): ");
            if (qCount > 5) qCount = 5;
            double quizzes[5];
            for (int i = 0; i < qCount; i++) quizzes[i] = readDouble("  Quiz mark: ");

            int aCount = readInt("How many assignment marks: ");
            double assignments[20];
            if (aCount > 20) aCount = 20;
            for (int i = 0; i < aCount; i++) assignments[i] = readDouble("  Assignment mark: ");

            double mid = readDouble("Mid marks (out of 40): ");
            double final_ = readDouble("Final marks (out of 60): ");

            enterMarks(roll, code, sem, quizzes, qCount, assignments, aCount, mid, final_);
        } else if (choice == 2) {
            string roll = liveTypeRoll("Compute GPA - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            cout << "GPA: " << computeGPA(roll, sem) << "\n";
        }
    } while (choice != 0);
}

void feeMenu() {
    int choice;
    do {
        cout << "\n----- FEE TRACKER -----\n";
        cout << "1. Record Payment\n2. Generate Receipt\n3. Late Fine\n0. Back\n";
        choice = readInt("Choice: ");

        if (choice == 1) {
            string roll = liveTypeRoll("Record Payment - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            double amount = readDouble("Amount Paid: ");
            string date = readLine("Payment Date (DD-MM-YYYY): ");
            string method = readLine("Payment Method: ");
            recordPayment(roll, sem, amount, date, method);
        } else if (choice == 2) {
            string roll = liveTypeRoll("Receipt - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            generateReceipt(roll, sem);
        } else if (choice == 3) {
            string roll = liveTypeRoll("Late Fine - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            cout << "Late fine: " << computeLateFine(roll, sem) << "\n";
        }
    } while (choice != 0);
}

void reportsMenu() {
    int choice;
    do {
        cout << "\n----- REPORTS -----\n";
        cout << "1. Merit List\n2. Attendance Defaulters\n3. Fee Defaulters\n";
        cout << "4. Department Summary\n5. Semester Result (one student)\n";
        cout << "6. Export a Report to File\n0. Back\n";
        choice = readInt("Choice: ");

        if (choice == 1) printMeritList();
        else if (choice == 2) printAttendanceDefaulters();
        else if (choice == 3) printFeeDefaulters();
        else if (choice == 4) printDepartmentSummary();
        else if (choice == 5) {
            string roll = liveTypeRoll("Semester Result - Roll");
            if (roll.empty()) { cout << "Cancelled.\n"; continue; }
            int sem = readInt("Semester: ");
            printSemesterResult(roll, sem);
        } else if (choice == 6) {
            int rc = readInt("Report (1=Merit 2=AttendDef 3=FeeDef 4=DeptSummary): ");
            string filename = readLine("Output filename: ");
            exportReportToFile(rc, filename);
        }
    } while (choice != 0);
}

int main() {
    int choice;
    cout << "=========================================\n";
    cout << "      CAMPUS ANALYTICS ENGINE\n";
    cout << "=========================================\n";

    do {
        cout << "\n========== MAIN MENU ==========\n";
        cout << "1. Student Management\n";
        cout << "2. Course Management\n";
        cout << "3. Attendance\n";
        cout << "4. Grades\n";
        cout << "5. Fee Tracker\n";
        cout << "6. Reports\n";
        cout << "0. Exit\n";
        choice = readInt("Choice: ");

        switch (choice) {
            case 1: studentMenu(); break;
            case 2: courseMenu(); break;
            case 3: attendanceMenu(); break;
            case 4: gradesMenu(); break;
            case 5: feeMenu(); break;
            case 6: reportsMenu(); break;
            case 0: cout << "Goodbye.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    return 0;
}
