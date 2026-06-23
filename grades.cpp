#include "grades.h"
#include "filehandler.h"
#include "course_ops.h"
#include "attendance.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <iomanip>
using namespace std;

static vector<string> gradesHeader() {
    vector<string> header;
    header.push_back("roll_no"); header.push_back("course_code"); header.push_back("semester");
    header.push_back("quiz_avg"); header.push_back("assignment_avg"); header.push_back("mid");
    header.push_back("final"); header.push_back("total"); header.push_back("letter");
    header.push_back("gpa_points");
    return header;
}

// average of quiz marks after dropping the 2 lowest (no sort used)
double bestThreeOfFive(double quizzes[], int n) {
    if (n <= 0) return 0.0;
    if (n < 3) {
        double sum = 0.0;
        for (int i = 0; i < n; i++) sum += quizzes[i];
        return sum / n;
    }

    int lowest1 = 0;
    for (int i = 1; i < n; i++) if (quizzes[i] < quizzes[lowest1]) lowest1 = i;

    int lowest2 = -1;
    for (int i = 0; i < n; i++) {
        if (i == lowest1) continue;
        if (lowest2 == -1 || quizzes[i] < quizzes[lowest2]) lowest2 = i;
    }

    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (i == lowest1 || i == lowest2) continue;
        sum += quizzes[i];
        count++;
    }
    return (count == 0) ? 0.0 : sum / count;
}

double computeWeightedTotal(double quizAvg, double asgnAvg, double mid, double final_) {
    return quizAvg * 0.10 + asgnAvg * 0.10 + mid * 0.30 + final_ * 0.50;
}

string getLetterGrade(double total) {
    if (total >= 85) return "A";
    if (total >= 80) return "B+";
    if (total >= 70) return "B";
    if (total >= 65) return "C+";
    if (total >= 60) return "C";
    if (total >= 50) return "D";
    return "F";
}

double letterToGpaPoints(const string &letter) {
    if (letter == "A") return 4.0;
    if (letter == "B+") return 3.5;
    if (letter == "B") return 3.0;
    if (letter == "C+") return 2.5;
    if (letter == "C") return 2.0;
    if (letter == "D") return 1.0;
    return 0.0;
}

bool enterMarks(const string &roll, const string &course, int semester,
                 double quizzes[], int quizCount,
                 double assignments[], int asgnCount,
                 double mid, double final_) {

    for (int i = 0; i < quizCount; i++)
        if (quizzes[i] < 0 || quizzes[i] > 100) {
            cout << "Error: quiz marks must be between 0 and 100.\n"; return false;
        }
    for (int i = 0; i < asgnCount; i++)
        if (assignments[i] < 0 || assignments[i] > 100) {
            cout << "Error: assignment marks must be between 0 and 100.\n"; return false;
        }
    if (mid < 0 || mid > 40) { cout << "Error: mid marks must be between 0 and 40.\n"; return false; }
    if (final_ < 0 || final_ > 60) { cout << "Error: final marks must be between 0 and 60.\n"; return false; }

    double quizAvgRaw = bestThreeOfFive(quizzes, quizCount);
    double asgnSum = 0.0;
    for (int i = 0; i < asgnCount; i++) asgnSum += assignments[i];
    double asgnAvg = (asgnCount > 0) ? (asgnSum / asgnCount) : 0.0;

    double midPct = (mid / 40.0) * 100.0;
    double finalPct = (final_ / 60.0) * 100.0;

    double total = computeWeightedTotal(quizAvgRaw, asgnAvg, midPct, finalPct);
    string letter = getLetterGrade(total);
    double gpaPoints = letterToGpaPoints(letter);

    ostringstream semStream; semStream << semester;
    ostringstream quizStream; quizStream << fixed << setprecision(2) << quizAvgRaw;
    ostringstream asgnStream; asgnStream << fixed << setprecision(2) << asgnAvg;
    ostringstream midStream; midStream << fixed << setprecision(2) << mid;
    ostringstream finalStream; finalStream << fixed << setprecision(2) << final_;
    ostringstream totalStream; totalStream << fixed << setprecision(2) << total;
    ostringstream gpaStream; gpaStream << fixed << setprecision(2) << gpaPoints;

    vector<string> newRow;
    newRow.push_back(roll);
    newRow.push_back(course);
    newRow.push_back(semStream.str());
    newRow.push_back(quizStream.str());
    newRow.push_back(asgnStream.str());
    newRow.push_back(midStream.str());
    newRow.push_back(finalStream.str());
    newRow.push_back(totalStream.str());
    newRow.push_back(letter);
    newRow.push_back(gpaStream.str());

    // overwrite existing row for this roll+course+semester, else append new
    vector<vector<string> > allRows = readTXT(GRADES_FILE);
    bool found = false;
    for (size_t i = 0; i < allRows.size(); i++) {
        if (allRows[i][GR_ROLL] == roll && allRows[i][GR_COURSE] == course &&
            allRows[i][GR_SEM] == semStream.str()) {
            allRows[i] = newRow;
            found = true;
            break;
        }
    }
    if (found) writeTXT(GRADES_FILE, gradesHeader(), allRows);
    else appendTXT(GRADES_FILE, newRow);

    cout << "Marks recorded. Total: " << total << "  Letter: " << letter << "\n";

    applyAttendancePenalty(roll, course); // overrides grade to F if attendance is low
    return true;
}

// credit-weighted average of GPA points for one student/semester
double computeGPA(const string &roll, int semester) {
    ostringstream semStream; semStream << semester;
    vector<vector<string> > grades = readTXT(GRADES_FILE);
    vector<vector<string> > courses = readTXT(COURSES_FILE);

    double weightedSum = 0.0;
    int totalCredits = 0;

    for (size_t i = 0; i < grades.size(); i++) {
        if (grades[i][GR_ROLL] == roll && grades[i][GR_SEM] == semStream.str()) {
            double gpaPoints = atof(grades[i][GR_GPA].c_str());
            int credits = 0;
            for (size_t j = 0; j < courses.size(); j++) {
                if (courses[j][CR_CODE] == grades[i][GR_COURSE]) {
                    credits = atoi(courses[j][CR_CREDITS].c_str());
                    break;
                }
            }
            weightedSum += gpaPoints * credits;
            totalCredits += credits;
        }
    }
    return (totalCredits == 0) ? 0.0 : weightedSum / totalCredits;
}

Stats computeClassStats(const string &course, int semester) {
    ostringstream semStream; semStream << semester;
    vector<vector<string> > grades = readTXT(GRADES_FILE);

    vector<double> totals;
    for (size_t i = 0; i < grades.size(); i++)
        if (grades[i][GR_COURSE] == course && grades[i][GR_SEM] == semStream.str())
            totals.push_back(atof(grades[i][GR_TOTAL].c_str()));

    Stats stats;
    stats.highest = 0; stats.lowest = 0; stats.mean = 0; stats.median = 0;
    int n = (int)totals.size();
    if (n == 0) return stats;

    stats.highest = totals[0];
    stats.lowest = totals[0];
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        if (totals[i] > stats.highest) stats.highest = totals[i];
        if (totals[i] < stats.lowest) stats.lowest = totals[i];
        sum += totals[i];
    }
    stats.mean = sum / n;

    // selection sort to find the median
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) if (totals[j] < totals[minIdx]) minIdx = j;
        double tmp = totals[i]; totals[i] = totals[minIdx]; totals[minIdx] = tmp;
    }
    if (n % 2 == 1) stats.median = totals[n / 2];
    else stats.median = (totals[n / 2 - 1] + totals[n / 2]) / 2.0;

    return stats;
}

void applyAttendancePenalty(const string &roll, const string &course) {
    double pct = getAttendancePct(roll, course);
    if (pct >= 75.0) return;

    vector<vector<string> > grades = readTXT(GRADES_FILE);
    bool changed = false;
    for (size_t i = 0; i < grades.size(); i++) {
        if (grades[i][GR_ROLL] == roll && grades[i][GR_COURSE] == course) {
            if (grades[i][GR_LETTER] != "F") {
                grades[i][GR_LETTER] = "F";
                grades[i][GR_GPA] = "0.00";
                changed = true;
            }
        }
    }
    if (changed) {
        writeTXT(GRADES_FILE, gradesHeader(), grades);
        cout << "Attendance shortage (<75%) detected for " << roll << " in " << course
             << " -> grade overridden to F.\n";
    }
}
