#include "fee_tracker.h"
#include "filehandler.h"
#include "student_ops.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <iomanip>
using namespace std;

bool isValidDateFormat(const string &date) {
    if (date.length() != 10) return false;
    if (date[2] != '-' || date[5] != '-') return false;

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }

    int day = atoi(date.substr(0, 2).c_str());
    int month = atoi(date.substr(3, 2).c_str());
    int year = atoi(date.substr(6, 4).c_str());

    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if (year < 1900 || year > 2100) return false;
    return true;
}

static bool isLeapYear(int year) {
    if (year % 4 != 0) return false;
    if (year % 100 == 0 && year % 400 != 0) return false;
    return true;
}

// turns DD-MM-YYYY into a running day count, no ctime used
static long dateToDayCount(const string &date) {
    int day = atoi(date.substr(0, 2).c_str());
    int month = atoi(date.substr(3, 2).c_str());
    int year = atoi(date.substr(6, 4).c_str());

    int monthLengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (isLeapYear(year)) monthLengths[1] = 29;

    long totalDays = 0;
    for (int y = 1; y < year; y++) totalDays += isLeapYear(y) ? 366 : 365;
    for (int m = 0; m < month - 1; m++) totalDays += monthLengths[m];
    totalDays += day;
    return totalDays;
}

int daysBetween(const string &date1, const string &date2) {
    if (!isValidDateFormat(date1) || !isValidDateFormat(date2)) return 0;
    long d1 = dateToDayCount(date1);
    long d2 = dateToDayCount(date2);
    return (int)(d2 - d1);
}

double computeLateFine(const string &roll, int semester) {
    ostringstream semStream; semStream << semester;
    vector<vector<string> > rows = readTXT(FEES_FILE);

    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i][FE_ROLL] == roll && rows[i][FE_SEM] == semStream.str()) {
            string dueDate = rows[i][FE_DUE];
            string paidDate = rows[i][FE_PAYDATE];

            if (!isValidDateFormat(paidDate)) return 0.0;
            int diff = daysBetween(dueDate, paidDate);
            if (diff <= 0) return 0.0;

            int weeksLate = diff / 7;
            if (weeksLate <= 0) return 0.0;

            double totalFee = atof(rows[i][FE_TOTAL].c_str());
            return totalFee * 0.02 * weeksLate;
        }
    }
    return 0.0;
}

static vector<string> feesHeader() {
    vector<string> header;
    header.push_back("fee_id"); header.push_back("roll_no"); header.push_back("semester");
    header.push_back("total_fee"); header.push_back("amount_paid"); header.push_back("due_date");
    header.push_back("payment_date"); header.push_back("payment_method"); header.push_back("status");
    return header;
}

bool recordPayment(const string &roll, int semester, double amountPaid,
                    const string &paymentDate, const string &method) {
    if (!isValidDateFormat(paymentDate)) {
        cout << "Error: payment date must be in DD-MM-YYYY format.\n";
        return false;
    }
    if (amountPaid < 0) {
        cout << "Error: amount paid cannot be negative.\n";
        return false;
    }

    ostringstream semStream; semStream << semester;
    vector<vector<string> > rows = readTXT(FEES_FILE);
    bool found = false;

    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i][FE_ROLL] == roll && rows[i][FE_SEM] == semStream.str()) {
            found = true;
            double totalFee = atof(rows[i][FE_TOTAL].c_str());
            double previouslyPaid = atof(rows[i][FE_PAID].c_str());
            double newPaidTotal = previouslyPaid + amountPaid;

            ostringstream paidStream; paidStream << fixed << setprecision(0) << newPaidTotal;
            rows[i][FE_PAID] = paidStream.str();
            rows[i][FE_PAYDATE] = paymentDate;
            rows[i][FE_METHOD] = method;

            string dueDate = rows[i][FE_DUE];
            bool isLate = isValidDateFormat(dueDate) && daysBetween(dueDate, paymentDate) > 0;

            if (newPaidTotal >= totalFee) rows[i][FE_STATUS] = isLate ? "paid_late" : "paid";
            else if (newPaidTotal > 0) rows[i][FE_STATUS] = "partial";
            else rows[i][FE_STATUS] = "unpaid";
            break;
        }
    }
    if (!found) {
        cout << "Error: no fee record found for roll " << roll << " semester " << semester << ".\n";
        return false;
    }

    writeTXT(FEES_FILE, feesHeader(), rows);
    cout << "Payment recorded for " << roll << ".\n";
    return true;
}

void generateReceipt(const string &roll, int semester) {
    ostringstream semStream; semStream << semester;
    vector<vector<string> > rows = readTXT(FEES_FILE);
    vector<string> studentRow = searchByRoll(roll);
    string name = studentRow.empty() ? "" : studentRow[ST_NAME];

    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i][FE_ROLL] == roll && rows[i][FE_SEM] == semStream.str()) {
            double totalFee = atof(rows[i][FE_TOTAL].c_str());
            double paid = atof(rows[i][FE_PAID].c_str());
            double lateFine = computeLateFine(roll, semester);
            double balance = (totalFee + lateFine) - paid;

            cout << "\n========== FEE RECEIPT ==========\n";
            cout << setw(20) << left << "Roll No:" << roll << "\n";
            cout << setw(20) << left << "Name:" << name << "\n";
            cout << setw(20) << left << "Semester:" << semester << "\n";
            cout << setfill('-') << setw(34) << "" << setfill(' ') << "\n";
            cout << setw(20) << left << "Tuition Fee:" << fixed << setprecision(2) << totalFee << "\n";
            cout << setw(20) << left << "Late Fine:" << fixed << setprecision(2) << lateFine << "\n";
            cout << setw(20) << left << "Total Due:" << fixed << setprecision(2) << (totalFee + lateFine) << "\n";
            cout << setw(20) << left << "Amount Paid:" << fixed << setprecision(2) << paid << "\n";
            cout << setw(20) << left << "Balance:" << fixed << setprecision(2) << balance << "\n";
            cout << "==================================\n";
            return;
        }
    }
    cout << "Error: no fee record found for roll " << roll << " semester " << semester << ".\n";
}

vector<vector<string> > getDefaulters() {
    vector<vector<string> > rows = readTXT(FEES_FILE);
    vector<vector<string> > defaulters;
    vector<double> outstanding;

    for (size_t i = 0; i < rows.size(); i++) {
        double totalFee = atof(rows[i][FE_TOTAL].c_str());
        double paid = atof(rows[i][FE_PAID].c_str());
        double balance = totalFee - paid;

        if (balance > 0 && (rows[i][FE_STATUS] == "unpaid" || rows[i][FE_STATUS] == "partial"
                             || rows[i][FE_STATUS] == "paid_late")) {
            defaulters.push_back(rows[i]);
            outstanding.push_back(balance);
        }
    }

    // bubble sort descending by outstanding amount
    int n = (int)defaulters.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (outstanding[j] < outstanding[j + 1]) {
                double tmpVal = outstanding[j]; outstanding[j] = outstanding[j + 1]; outstanding[j + 1] = tmpVal;
                vector<string> tmpRow = defaulters[j]; defaulters[j] = defaulters[j + 1]; defaulters[j + 1] = tmpRow;
            }
        }
    }
    return defaulters;
}
