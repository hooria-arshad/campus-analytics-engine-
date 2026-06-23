#include "course_ops.h"
#include "filehandler.h"
#include "student_ops.h"
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

// grades.txt is owned by the grades module, but we read it here to check prerequisites
const string GRADES_FILE_FOR_PREREQ = "data/grades.txt";

string enrollResultMessage(EnrollResult result) {
    switch (result) {
        case ENROLL_SUCCESS: return "Enrollment successful.";
        case ENROLL_STUDENT_INACTIVE: return "Error: student is not active.";
        case ENROLL_COURSE_NOT_FOUND: return "Error: course does not exist.";
        case ENROLL_NO_SEATS: return "Error: no seats available in this course.";
        case ENROLL_ALREADY_ENROLLED: return "Error: student is already enrolled in this course.";
        case ENROLL_CREDIT_OVERLOAD: return "Error: enrolling would exceed the 21 credit hour limit.";
        case ENROLL_PREREQ_NOT_MET: return "Error: prerequisite course not passed.";
        default: return "Unknown error.";
    }
}

bool courseExists(const string &code) {
    return rowExists(COURSES_FILE, CR_CODE, code);
}

// sums credit hours of active enrollments for one student in one semester
int getCreditLoad(const string &roll, int semester) {
    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE);
    vector<vector<string> > courses = readTXT(COURSES_FILE);

    ostringstream semStream; semStream << semester;
    string semStr = semStream.str();
    int totalCredits = 0;

    for (size_t i = 0; i < enrollments.size(); i++) {
        if (enrollments[i][EN_ROLL] == roll && enrollments[i][EN_SEM] == semStr &&
            enrollments[i][EN_STATUS] == "active") {
            for (size_t j = 0; j < courses.size(); j++) {
                if (courses[j][CR_CODE] == enrollments[i][EN_COURSE]) {
                    totalCredits += atoi(courses[j][CR_CREDITS].c_str());
                    break;
                }
            }
        }
    }
    return totalCredits;
}

// looks at course's prereq field, checks grades.txt for a non-F grade in it
bool checkPrerequisite(const string &roll, const string &code) {
    vector<string> courseRow = findRow(COURSES_FILE, CR_CODE, code);
    if (courseRow.empty()) return false;

    string prereq = courseRow[CR_PREREQ];
    if (prereq == "NONE") return true;

    vector<vector<string> > grades = readTXT(GRADES_FILE_FOR_PREREQ);
    for (size_t i = 0; i < grades.size(); i++) {
        if (grades[i].size() >= 9 && grades[i][0] == roll && grades[i][1] == prereq) {
            if (grades[i][8] != "F") return true;
        }
    }
    return false;
}

EnrollResult enrollStudent(const string &roll, const string &code, int semester) {
    if (!isStudentActive(roll)) return ENROLL_STUDENT_INACTIVE;

    vector<string> courseRow = findRow(COURSES_FILE, CR_CODE, code);
    if (courseRow.empty()) return ENROLL_COURSE_NOT_FOUND;

    int capacity = atoi(courseRow[CR_CAPACITY].c_str());
    int enrolledCount = atoi(courseRow[CR_ENROLLED].c_str());
    if (enrolledCount >= capacity) return ENROLL_NO_SEATS;

    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE);
    for (size_t i = 0; i < enrollments.size(); i++) {
        if (enrollments[i][EN_ROLL] == roll && enrollments[i][EN_COURSE] == code &&
            enrollments[i][EN_STATUS] == "active") return ENROLL_ALREADY_ENROLLED;
    }

    int courseCredits = atoi(courseRow[CR_CREDITS].c_str());
    int currentLoad = getCreditLoad(roll, semester);
    if (currentLoad + courseCredits > MAX_CREDIT_LOAD) return ENROLL_CREDIT_OVERLOAD;

    if (!checkPrerequisite(roll, code)) return ENROLL_PREREQ_NOT_MET;

    // build a new enrollment id like E0125
    int maxNum = 0;
    for (size_t i = 0; i < enrollments.size(); i++) {
        string idStr = enrollments[i][EN_ID];
        if (idStr.length() > 1) {
            int num = atoi(idStr.substr(1).c_str());
            if (num > maxNum) maxNum = num;
        }
    }
    int newNum = maxNum + 1;
    ostringstream idStream;
    idStream << "E";
    if (newNum < 10) idStream << "000";
    else if (newNum < 100) idStream << "00";
    else if (newNum < 1000) idStream << "0";
    idStream << newNum;

    ostringstream semStream; semStream << semester;

    vector<string> newRow;
    newRow.push_back(idStream.str());
    newRow.push_back(roll);
    newRow.push_back(code);
    newRow.push_back(semStream.str());
    newRow.push_back("01-01-2024");
    newRow.push_back("active");
    appendTXT(ENROLLMENTS_FILE, newRow);

    // bump up the course's enrolled count by 1
    vector<vector<string> > courses = readTXT(COURSES_FILE);
    for (size_t i = 0; i < courses.size(); i++) {
        if (courses[i][CR_CODE] == code) {
            int cnt = atoi(courses[i][CR_ENROLLED].c_str());
            ostringstream cntStream; cntStream << (cnt + 1);
            courses[i][CR_ENROLLED] = cntStream.str();
            break;
        }
    }
    vector<string> header;
    header.push_back("course_code"); header.push_back("course_name"); header.push_back("credit_hours");
    header.push_back("instructor"); header.push_back("capacity"); header.push_back("enrolled");
    header.push_back("prerequisite");
    writeTXT(COURSES_FILE, header, courses);

    return ENROLL_SUCCESS;
}

bool dropCourse(const string &roll, const string &code, int semester) {
    ostringstream semStream; semStream << semester;
    string semStr = semStream.str();

    // can't drop a course once attendance has been recorded for it
    vector<vector<string> > attendance = readTXT("data/attendance_log.txt");
    for (size_t i = 0; i < attendance.size(); i++) {
        if (attendance[i][1] == roll && attendance[i][2] == code) {
            cout << "Error: cannot drop, attendance already recorded for this course.\n";
            return false;
        }
    }

    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE);
    bool found = false;
    for (size_t i = 0; i < enrollments.size(); i++) {
        if (enrollments[i][EN_ROLL] == roll && enrollments[i][EN_COURSE] == code &&
            enrollments[i][EN_SEM] == semStr && enrollments[i][EN_STATUS] == "active") {
            enrollments[i][EN_STATUS] = "dropped";
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Error: active enrollment not found for this roll/course/semester.\n";
        return false;
    }

    vector<string> header;
    header.push_back("enrollment_id"); header.push_back("roll_no"); header.push_back("course_code");
    header.push_back("semester"); header.push_back("enrollment_date"); header.push_back("status");
    writeTXT(ENROLLMENTS_FILE, header, enrollments);

    // decrement the course's enrolled count
    vector<vector<string> > courses = readTXT(COURSES_FILE);
    for (size_t i = 0; i < courses.size(); i++) {
        if (courses[i][CR_CODE] == code) {
            int cnt = atoi(courses[i][CR_ENROLLED].c_str());
            if (cnt > 0) cnt--;
            ostringstream cntStream; cntStream << cnt;
            courses[i][CR_ENROLLED] = cntStream.str();
            break;
        }
    }
    vector<string> cHeader;
    cHeader.push_back("course_code"); cHeader.push_back("course_name"); cHeader.push_back("credit_hours");
    cHeader.push_back("instructor"); cHeader.push_back("capacity"); cHeader.push_back("enrolled");
    cHeader.push_back("prerequisite");
    writeTXT(COURSES_FILE, cHeader, courses);

    cout << "Course " << code << " dropped for " << roll << ".\n";
    return true;
}

vector<vector<string> > listEnrolledStudents(const string &code) {
    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE);
    vector<vector<string> > result;
    for (size_t i = 0; i < enrollments.size(); i++)
        if (enrollments[i][EN_COURSE] == code && enrollments[i][EN_STATUS] == "active")
            result.push_back(enrollments[i]);
    return result;
}

// ---------------- live-typing for course codes (bonus feature) ----------------

static char toUpperCharCourse(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}

static bool courseStartsWithPrefix(const string &text, const string &prefix) {
    if (prefix.length() > text.length()) return false;
    for (size_t i = 0; i < prefix.length(); i++) {
        if (toUpperCharCourse(text[i]) != toUpperCharCourse(prefix[i])) return false;
    }
    return true;
}

static vector<vector<string> > filterCoursesByPrefix(const string &prefix) {
    vector<vector<string> > allCourses = readTXT(COURSES_FILE);
    vector<vector<string> > matches;
    for (size_t i = 0; i < allCourses.size(); i++) {
        bool codeMatch = courseStartsWithPrefix(allCourses[i][CR_CODE], prefix);
        bool nameMatch = courseStartsWithPrefix(allCourses[i][CR_NAME], prefix);
        if (codeMatch || nameMatch) matches.push_back(allCourses[i]);
    }
    return matches;
}

static void printCourseMatches(const string &label, const string &prefix,
                                const vector<vector<string> > &matches) {
    cout << "\n" << label << " (type live): \"" << prefix << "\"\n";
    cout << "(Backspace = delete, Enter = finish, Esc = cancel)\n";
    cout << "------------------------------------------------------------\n";
    if (matches.empty()) {
        cout << "(no matches)\n";
    } else {
        for (size_t i = 0; i < matches.size(); i++) {
            cout << matches[i][CR_CODE] << " | " << matches[i][CR_NAME]
                 << " | Seats " << matches[i][CR_ENROLLED] << "/" << matches[i][CR_CAPACITY] << "\n";
        }
    }
    cout << "------------------------------------------------------------\n";
}

#ifndef _WIN32
static int getCharRawCourse() {
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

static int readOneCharCourse() {
#ifdef _WIN32
    return _getch();
#else
    return getCharRawCourse();
#endif
}

string liveTypeCourseCode(const string &label) {
    string prefix = "";
    printCourseMatches(label, prefix, filterCoursesByPrefix(prefix));

    while (true) {
        int ch = readOneCharCourse();

        if (ch == 13 || ch == 10) return prefix; // Enter
        if (ch == 27) return "";                  // Esc cancels

        if (ch == 8 || ch == 127) {
            if (!prefix.empty()) prefix = prefix.substr(0, prefix.length() - 1);
        } else if (ch >= 32 && ch <= 126) {
            prefix += (char)ch;
        } else {
            continue;
        }

        printCourseMatches(label, prefix, filterCoursesByPrefix(prefix));
    }
}
