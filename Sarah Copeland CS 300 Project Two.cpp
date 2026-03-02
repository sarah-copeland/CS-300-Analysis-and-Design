/*
    CS 300 Project Two: Advising Assistance Program
    Author: Sarah Copeland
    Southern New Hampshire University

    Notes:
    - Single-file C++ program (no external CSV parser headers)
    - Data structure: ordered tree via std::map (courseNumber -> Course)
    - Supports loading a CSV, printing sorted course list, and printing a single course with prerequisites
*/

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};

static string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static string toUpper(string s) {
    for (char& ch : s) {
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}

static vector<string> splitCSVLine(const string& line) {
    // For this assignment's file format, values are comma-separated and not quoted.
    vector<string> tokens;
    string token;
    stringstream ss(line);
    while (getline(ss, token, ',')) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

static bool tryParseMenuChoice(const string& s, int& out) {
    string t = trim(s);
    if (t.empty()) return false;

    for (char ch : t) {
        if (!isdigit(static_cast<unsigned char>(ch))) return false;
    }

    try {
        out = stoi(t);
        return true;
    } catch (...) {
        return false;
    }
}

static void printMenu() {
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
}

static bool loadCoursesFromFile(
    const string& filePath,
    map<string, Course>& courseTree,
    string& errorMessage
) {
    courseTree.clear();
    errorMessage.clear();

    ifstream inFile(filePath);
    if (!inFile.is_open()) {
        errorMessage = "Error: file not found or cannot be opened.";
        return false;
    }

    vector<Course> tempCourses;
    set<string> courseNumbers;
    vector<string> errors;

    string line;
    while (getline(inFile, line)) {
        line = trim(line);
        if (line.empty()) continue;

        vector<string> tokens = splitCSVLine(line);

        // Must have at least courseNumber and title
        if (tokens.size() < 2) {
            errors.push_back("Invalid format: less than two fields on line: " + line);
            continue;
        }

        string courseNum = toUpper(tokens[0]);
        string courseTitle = tokens[1];

        if (courseNum.empty() || courseTitle.empty()) {
            errors.push_back("Invalid format: missing course number or title on line: " + line);
            continue;
        }

        Course c;
        c.courseNumber = courseNum;
        c.title = courseTitle;

        for (size_t i = 2; i < tokens.size(); i++) {
            string prereq = toUpper(tokens[i]);
            if (!prereq.empty()) {
                c.prerequisites.push_back(prereq);
            }
        }

        tempCourses.push_back(c);
        courseNumbers.insert(courseNum);
    }

    inFile.close();

    // Validate prerequisites exist
    for (const Course& c : tempCourses) {
        for (const string& prereq : c.prerequisites) {
            if (courseNumbers.find(prereq) == courseNumbers.end()) {
                errors.push_back("Invalid prerequisite: " + prereq + " not found for course " + c.courseNumber);
            }
        }
    }

    if (!errors.empty()) {
        // Keep message short and readable for a command-line program
        errorMessage = "File validation failed. First issue: " + errors[0];
        return false;
    }

    // Load into ordered map (tree)
    for (const Course& c : tempCourses) {
        courseTree[c.courseNumber] = c;
    }

    return true;
}

static void printCourseList(const map<string, Course>& courseTree) {
    cout << "Here is a sample schedule:" << endl;
    for (const auto& kv : courseTree) {
        const Course& c = kv.second;
        cout << c.courseNumber << ", " << c.title << endl;
    }
}

static void printCourseInfo(const map<string, Course>& courseTree, const string& userCourseNumber) {
    string key = toUpper(trim(userCourseNumber));

    auto it = courseTree.find(key);
    if (it == courseTree.end()) {
        cout << "Course not found." << endl;
        return;
    }

    const Course& c = it->second;
    cout << c.courseNumber << ", " << c.title << endl;

    if (c.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    // Print prerequisites with titles when available (keeps the sample label, but adds clarity)
    cout << "Prerequisites: ";
    for (size_t i = 0; i < c.prerequisites.size(); i++) {
        const string& prereqNum = c.prerequisites[i];
        cout << prereqNum;

        auto pit = courseTree.find(prereqNum);
        if (pit != courseTree.end()) {
            cout << " (" << pit->second.title << ")";
        }

        if (i + 1 < c.prerequisites.size()) cout << ", ";
    }
    cout << endl;
}

int main() {
    cout << "Welcome to the course planner." << endl;

    map<string, Course> courseTree;
    bool loaded = false;

    while (true) {
        printMenu();
        cout << "What would you like to do? ";

        string input;
        getline(cin, input);

        int choice = 0;
        if (!tryParseMenuChoice(input, choice)) {
            cout << input << " is not a valid option." << endl;
            continue;
        }

        if (choice == 1) {
            cout << "Enter the file name: ";
            string filePath;
            getline(cin, filePath);
            filePath = trim(filePath);

            string errorMessage;
            if (loadCoursesFromFile(filePath, courseTree, errorMessage)) {
                loaded = true;
            } else {
                loaded = false;
                courseTree.clear();
                cout << errorMessage << endl;
            }
        } else if (choice == 2) {
            if (!loaded) {
                cout << "Please load data first." << endl;
                continue;
            }
            printCourseList(courseTree);
        } else if (choice == 3) {
            if (!loaded) {
                cout << "Please load data first." << endl;
                continue;
            }
            cout << "What course do you want to know about? ";
            string courseNumber;
            getline(cin, courseNumber);
            printCourseInfo(courseTree, courseNumber);
        } else if (choice == 9) {
            cout << "Thank you for using the course planner!" << endl;
            break;
        } else {
            cout << choice << " is not a valid option." << endl;
        }
    }

    return 0;
}
