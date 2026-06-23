#ifndef GRADES_H
#define GRADES_H

#include <string>
#include <vector>
using namespace std;

// M5: grades.txt -> roll(0), course(1), semester(2), quiz_avg(3), asgn_avg(4),
// mid(5), final(6), total(7), letter(8), gpa_points(9)

const string GRADES_FILE = "data/grades.txt";

const int GR_ROLL = 0;
const int GR_COURSE = 1;
const int GR_SEM = 2;
const int GR_QUIZ = 3;
const int GR_ASGN = 4;
const int GR_MID = 5;
const int GR_FINAL = 6;
const int GR_TOTAL = 7;
const int GR_LETTER = 8;
const int GR_GPA = 9;

struct Stats {
    double highest;
    double lowest;
    double mean;
    double median;
};

double bestThreeOfFive(double quizzes[], int n);    // drops 2 lowest quiz marks
double computeWeightedTotal(double quizAvg, double asgnAvg, double mid, double final_);
string getLetterGrade(double total);
double letterToGpaPoints(const string &letter);

bool enterMarks(const string &roll, const string &course, int semester,
                 double quizzes[], int quizCount,
                 double assignments[], int asgnCount,
                 double mid, double final_);

double computeGPA(const string &roll, int semester);
Stats computeClassStats(const string &course, int semester);
void applyAttendancePenalty(const string &roll, const string &course); // <75% -> grade F

#endif
