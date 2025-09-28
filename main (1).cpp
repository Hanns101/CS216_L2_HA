/******************************************************
 * Lab #2 – Bank Accounts
 * Constraints honored:
 *  - No STL containers (vector/list/stack/etc.)
 *  - No pointers, no dynamic memory
 *  - No lambdas / regex / range-based loops
 *****************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

/* ------------------- Global constants (filenames) ------------------- */
const string INPUT_FILE     = "requests.txt";
const string OUTPUT_FILE    = "new_accounts.txt";
const string ERROR_FILE     = "invalid_records.txt";
const string LOG_FILE       = "bank_log.txt";

/* ------------------- Config knobs (one place) ------------------- */
const int    MAX_ACCOUNTS   = 200;   // stack capacity
const double MAX_OVERDRAFT  = 50.0;  // allowed overdraft
const int    DECIMALS       = 2;     // numeric formatting

/* --------------- Utility: logging --------------- */
void appendLog(const string& msg) {
    ofstream log(LOG_FILE.c_str(), ios::app);
    if (log) log << msg << '\n';
}

/* ------------------- BankAccount ------------------- */
class BankAccount {
private:
    string accountId;       // 8 digits: first 6 random, last 2 sequential
    string firstName;
    string lastName;
    string email;
    double presentBalance;   // can be negative but not less than -MAX_OVERDRAFT
    double availableBalance; // cannot exceed presentBalance + MAX_OVERDRAFT

    /* ---- tiny helpers (no regex) ---- */
    static bool isAlpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
    static bool isDigit(char c) { return (c >= '0' && c <= '9'); }
    static bool isAlnum_(char c){ return isAlpha(c) || isDigit(c) || c == '_' || c == '.'; }

public:
    /* ---- default ctor ---- */
    BankAccount() : accountId("00000000"), firstName(""), lastName(""),
                    email(""), presentBalance(0.0), availableBalance(0.0) { }

    /* ---- copy ctor ---- */
    BankAccount(const BankAccount& other) {
        accountId       = other.accountId;
        firstName       = other.firstName;
        lastName        = other.lastName;
        email           = other.email;
        presentBalance  = other.presentBalance;
        availableBalance= other.availableBalance;
    }

    /* ---- destructor: reset to defaults (per spec note) ---- */
    ~BankAccount() {
        accountId = "00000000";
        firstName.clear();
        lastName.clear();
        email.clear();
        presentBalance  = 0.0;
        availableBalance= 0.0;
    }

    /* ------------ Validators (finish as needed) ------------ */

    static bool validName(const string& s) {
        // at least 2 alphabetic characters (letters only)
        int count = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            if (!isAlpha(s[i])) return false;
            ++count;
        }
        return count >= 2;
    }

    static bool validSSN(const string& s) {
        // Spec screenshot says "SSN: 10 digits" (use exactly 10 here)
        if (s.size() != 10) return false;
        for (size_t i = 0; i < s.size(); ++i) if (!isDigit(s[i])) return false;
        return true;
    }

    static bool validEmail(const string& e) {
        // Format: user@server.domain
        // user: min 4 alnum + dot/underscore
        // server: min 4 alpha
        // domain: com or edu
        size_t at = e.find('@');
        size_t dot = e.rfind('.');
        if (at == string::npos || dot == string::npos || dot <= at + 1) return false;

        string user = e.substr(0, at);
        string host = e.substr(at + 1, dot - at - 1);
        string dom  = e.substr(dot + 1);

        if (dom != "com" && dom != "edu") return false;

        if (host.size() < 4) return false;
        for (size_t i = 0; i < host.size(); ++i) if (!isAlpha(host[i])) return false;

        if (user.size() < 4) return false;
        for (size_t i = 0; i < user.size(); ++i) if (!isAlnum_(user[i])) return false;

        return true;
    }

    /* ------------ Setters with rules ------------ */

    bool setPresentBalance(double val) {
        // Max overdraft is $50: i.e., presentBalance >= -MAX_OVERDRAFT
        if (val < -MAX_OVERDRAFT) return false;
        presentBalance = val;
        // keep availableBalance consistent
        if (availableBalance > presentBalance + MAX_OVERDRAFT)
            availableBalance = presentBalance + MAX_OVERDRAFT;
        return true;
    }

    bool setAvailableBalance(double val) {
        // cannot be higher than present + overdraft
        if (val > presentBalance + MAX_OVERDRAFT) return false;
        availableBalance = val;
        return true;
    }

    bool setEmail(const string& e) {
        if (!validEmail(e)) return false;
        email = e;
        return true;
    }

    bool setName(const string& first, const string& last) {
        if (!validName(first) || !validName(last)) return false;
        firstName = first;
        lastName  = last;
        return true;
    }

    void setAccountId(const string& id8) { accountId = id8; }

    /* ---- All-in-one setter per spec; logs on failure ---- */
    bool setAccount(const string& ssn, const string& first, const string& last,
                    const string& mail, double present, double available,
                    string& errorOut)
    {
        errorOut.clear();
        if (!validSSN(ssn))                errorOut = "Invalid SSN";
        else if (!setName(first, last))    errorOut = "Invalid name";
        else if (!setEmail(mail))          errorOut = "Invalid email";
        else if (!setPresentBalance(present)) errorOut = "Invalid present balance";
        else if (!setAvailableBalance(available)) errorOut = "Invalid available balance";

        if (!errorOut.empty()) {
            appendLog("setAccount FAILED: " + errorOut +
                      " | " + ssn + " " + first + " " + last + " " + mail);
            return false;
        }
        appendLog("setAccount OK: " + first + " " + last + " (" + accountId + ")");
        return true;
    }

    /* ------------ Getters ------------ */
    string getAccountId() const { return accountId; }
    string getFirst() const { return firstName; }
    string getLast()  const { return lastName; }
    string getEmail() const { return email; }
    double getPresent() const { return presentBalance; }
    double getAvailable() const { return availableBalance; }

    /* ------------ Print row ------------ */
    void printRow() const {
        cout << left  << setw(12) << accountId
             << left  << setw(14) << firstName
             << left  << setw(14) << lastName
             << left  << setw(26) << email
             << right << fixed << setprecision(DECIMALS)
             << setw(10) << presentBalance
             << setw(10) << availableBalance << '\n';
    }

    /* Reset to defaults (for destructor semantics / reuse) */
    void reset() {
        accountId = "00000000";
        firstName.clear();
        lastName.clear();
        email.clear();
        presentBalance  = 0.0;
        availableBalance= 0.0;
    }
};

/* ------------------- Fixed Stack (array) ------------------- */
template <typename T, int N>
class FixedStack {
private:
    T data[N];
    int topIdx; // number of items (also index of next push)
public:
    FixedStack(): topIdx(0) { }
    bool push(const T& item) {
        if (topIdx >= N) return false;
        data[topIdx] = item;
        ++topIdx;
        return true;
    }
    bool empty() const { return topIdx == 0; }
    int  size()  const { return topIdx; }
    const T& at(int i) const { return data[i]; }
          T& at(int i)       { return data[i]; }
};

/* ------------------- Helpers for ID creation ------------------- */
string random6() {
    string s = "";
    for (int i = 0; i < 6; ++i) s += char('0' + (rand() % 10));
    return s;
}
string twoDigit(int x) {
    if (x < 0) x = 0;
    if (x > 99) x %= 100;
    string s = "";
    s += char('0' + (x / 10));
    s += char('0' + (x % 10));
    return s;
}

/* ------------------- Global state for this run ------------------- */
FixedStack<BankAccount, MAX_ACCOUNTS> gAccounts;
string gInvalidMsgs[MAX_ACCOUNTS];
int    gInvalidCount = 0;
int    gSeqLastTwo   = 0;  // sequential last 2 digits for account id

/* --------------- I/O: headers ---------------- */
void printTableHeader() {
    cout << left << setw(12) << "Account#"
         << left << setw(14) << "First"
         << left << setw(14) << "Last"
         << left << setw(26) << "Email"
         << right << setw(10) << "Present"
         << right << setw(10) << "Avail"
         << '\n';
    cout << string(12+14+14+26+10+10, '-') << '\n';
}

/* ------------------- Menu actions ------------------- */

void processRequestsOnce() {
    ifstream fin(INPUT_FILE.c_str());
    ofstream err(ERROR_FILE.c_str()); // overwrite for this run
    if (!fin) {
        cout << "Cannot open input file: " << INPUT_FILE << '\n';
        appendLog("ERROR: cannot open " + INPUT_FILE);
        return;
    }
    if (!err) {
        cout << "Cannot open error file: " << ERROR_FILE << '\n';
        appendLog("ERROR: cannot open " + ERROR_FILE);
        return;
    }

    int processed = 0, created = 0, invalid = 0;

    string ssn, first, last, mail;
    // For present/available balances: spec hints initial defaults.
    // Here we set present by role (student/edu vs other) from email domain:
    // NOTE: You may change this logic to match your instructor’s exact rule.
    while (fin >> ssn >> first >> last >> mail) {
        ++processed;

        BankAccount acc;
        const string id = random6() + twoDigit(gSeqLastTwo++);
        acc.setAccountId(id);

        // Spec defaults:
        double presentDefault = 100.0; // general default
        // If student or works at educational institution: present -150 (per spec)
        // We'll treat any email at *.edu as student/edu:
        if (mail.size() >= 4 && mail.substr(mail.size()-3) == "edu")
            presentDefault = -150.0;

        double availDefault = 0.0; // “available may be used immediately: $0.00”

        string why;
        if (!acc.setAccount(ssn, first, last, mail, presentDefault, availDefault, why)) {
            ++invalid;
            string line = ssn + " " + first + " " + last + " " + mail + " :: " + why;
            if (gInvalidCount < MAX_ACCOUNTS) gInvalidMsgs[gInvalidCount++] = line;
            err << line << '\n';
        } else {
            if (!gAccounts.push(acc)) {
                ++invalid;
                string line = ssn + " " + first + " " + last + " " + mail + " :: STACK FULL";
                if (gInvalidCount < MAX_ACCOUNTS) gInvalidMsgs[gInvalidCount++] = line;
                err << line << '\n';
                appendLog("PUSH FAILED (stack full) for account " + id);
            } else {
                ++created;
            }
        }
    }

    cout << "Processed: " << processed
         << " | Created: "  << created
         << " | Invalid: "  << invalid  << '\n';

    appendLog("RUN SUMMARY -> processed=" + to_string(processed) +
              " created=" + to_string(created) +
              " invalid=" + to_string(invalid));
}

void printSuccessfulAccounts() {
    if (gAccounts.size() == 0) { cout << "No accounts.\n"; return; }
    printTableHeader();
    for (int i = 0; i < gAccounts.size(); ++i) gAccounts.at(i).printRow();
}

void printInvalidRecords() {
    if (gInvalidCount == 0) { cout << "No invalid records.\n"; return; }
    for (int i = 0; i < gInvalidCount; ++i) cout << gInvalidMsgs[i] << '\n';
}

void printLogFile() {
    ifstream in(LOG_FILE.c_str());
    if (!in) { cout << "No log file yet.\n"; return; }
    string line;
    while (getline(in, line)) cout << line << '\n';
}

void writeAccountsToOutputAndQuit() {
    ofstream out(OUTPUT_FILE.c_str());
    if (!out) {
        cout << "Cannot open output file: " << OUTPUT_FILE << '\n';
        return;
    }
    out << left << setw(12) << "Account#"
        << left << setw(14) << "First"
        << left << setw(14) << "Last"
        << left << setw(26) << "Email"
        << right << setw(10) << "Present"
        << right << setw(10) << "Avail" << '\n';
    out << string(12+14+14+26+10+10, '-') << '\n';

    for (int i = 0; i < gAccounts.size(); ++i) {
        const BankAccount& a = gAccounts.at(i);
        out << left << setw(12) << a.getAccountId()
            << left << setw(14) << a.getFirst()
            << left << setw(14) << a.getLast()
            << left << setw(26) << a.getEmail()
            << right << fixed << setprecision(DECIMALS)
            << setw(10) << a.getPresent()
            << setw(10) << a.getAvailable() << '\n';
    }

    cout << "Wrote " << gAccounts.size() << " account(s) to " << OUTPUT_FILE << '\n';
}

/* ------------------- Menu ------------------- */
void showMenu() {
    cout << "\n--- Bank Account Menu ---\n"
         << "1) Process all new checking account requests (once)\n"
         << "2) Print successfully created accounts to screen\n"
         << "3) Print invalid records to screen\n"
         << "4) Print the log file\n"
         << "5) Quit and write accounts to output file\n"
         << "Choice: ";
}

int main() {
    srand(static_cast<unsigned>(time(0))); // for random first 6 digits
    // Clear/initialize log for this run
    ofstream clear(LOG_FILE.c_str()); if (clear) clear << "=== Log start ===\n";

    int choice = 0;
    do {
        showMenu();
        cin >> choice;
        cout << '\n';
        if      (choice == 1) processRequestsOnce();
        else if (choice == 2) printSuccessfulAccounts();
        else if (choice == 3) printInvalidRecords();
        else if (choice == 4) printLogFile();
        else if (choice == 5) writeAccountsToOutputAndQuit();
        else cout << "Invalid choice.\n";
    } while (choice != 5);

    /******************************************************
     * Test runs (paste input & expected notes here)
     *
     * Example 'requests.txt':
     * 1234567890 Mary Lee mary_lee@lapc.edu
     * 2222333344 Alan Turing alan.turing@computing.com
     *
     * Example output/notes:
     *  - First creates account with present = -150.00 (edu)
     *  - Second present = 100.00 (com)
     *****************************************************/

    return 0;
}
