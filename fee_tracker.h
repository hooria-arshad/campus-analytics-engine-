#ifndef FEE_TRACKER_H
#define FEE_TRACKER_H

#include <string>
#include <vector>
using namespace std;

// M6: fees.txt -> id(0), roll(1), semester(2), total(3), paid(4), due_date(5),
// pay_date(6), method(7), status(8). dates are DD-MM-YYYY

const string FEES_FILE = "data/fees.txt";

const int FE_ID = 0;
const int FE_ROLL = 1;
const int FE_SEM = 2;
const int FE_TOTAL = 3;
const int FE_PAID = 4;
const int FE_DUE = 5;
const int FE_PAYDATE = 6;
const int FE_METHOD = 7;
const int FE_STATUS = 8;

bool isValidDateFormat(const string &date);
int daysBetween(const string &date1, const string &date2);   // no ctime, manual calc
double computeLateFine(const string &roll, int semester);     // 2% per complete week late

bool recordPayment(const string &roll, int semester, double amountPaid,
                    const string &paymentDate, const string &method);
void generateReceipt(const string &roll, int semester);
vector<vector<string> > getDefaulters();   // balance > 0, sorted by amount (bubble sort)

#endif
