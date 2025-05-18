#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace std;

// ------ Player Node (Linked List Node) ------
struct Player {
    int playerID;
    string username;
    int rank;
    string university;
    bool checkInStatus;
    string registrationTime;
    bool isWildcard;
    Player* next; // pointer to next player in the queue

    Player(int id, string user, int r, string uni, string regTime, bool wildcard)
        : playerID(id), username(user), rank(r), university(uni),
          checkInStatus(false), registrationTime(regTime), isWildcard(wildcard), next(nullptr) {}
};

// ------ Circular Queue using Linked List ------
class CircularQueue {
protected:
    Player* front;
    Player* rear;
    int nextID;

    string getCurrentTime() {
        time_t now = time(0);
        char buf[80];
        tm* ltm = localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
        return buf;
    }

public:
    CircularQueue() : front(nullptr), rear(nullptr), nextID(1000) {}

    void initializeNextIDFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        string line;
        int maxID = 999;

        while (getline(file, line)) {
            stringstream ss(line);
            string idStr;
            getline(ss, idStr, ',');

            // Skip the header line or any non-integer ID
            if (idStr == "playerID") continue;

            try {
                int id = stoi(idStr); // may throw if not a number
                if (id > maxID) maxID = id;
            } catch (invalid_argument&) {
                continue; // safely skip malformed or duplicate header lines
            } catch (out_of_range&) {
                continue; // skip numbers too large to convert
            }
        }

        nextID = maxID + 1;
        file.close();
    }


    // Add a new player to the queue (enqueue)
    void enqueue(string username, int rank, string university, bool isWildcard = false) {
        Player* newPlayer = new Player(nextID++, username, rank, university, getCurrentTime(), isWildcard);

        if (!front) {
            front = rear = newPlayer;
            newPlayer->next = newPlayer; // Circular!
        } else {
            rear->next = newPlayer;
            newPlayer->next = front;
            rear = newPlayer;
        }
        cout << "Player \"" << username << "\" has been registered in the queue.\n";
    }

    // Save all players in queue to CSV file (overwrite file with all in-memory data)
    void saveToCSV(const string& filename) {
        ofstream file(filename, ios::app);
        if (!file.is_open()) {
            cout << "Error: Could not open file for writing.\n";
            return;
        }
        // Save only the latest registered player (for immediate append)
        if (rear) {
            file << rear->playerID << ","
                << rear->username << ","
                << rear->rank << ","
                << rear->university << ","
                << (rear->checkInStatus ? "1" : "0") << ","
                << rear->registrationTime << ","
                << (rear->isWildcard ? "1" : "0") << "\n";
        }
        file.close();
        cout << "Registration saved to " << filename << endl;
    }

    // (Optional) Destructor to free all memory
    ~CircularQueue() {
        if (!front) return;
        Player* curr = front;
        do {
            Player* temp = curr;
            curr = curr->next;
            delete temp;
        } while (curr != front);
        front = rear = nullptr;
    }
};

// Display the entire CSV in a table format (ignore in-memory queue)
void displayCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "No player records found yet.\n";
        return;
    }
    string line;

    cout << "\n=====================================================================================================================\n";
    cout << "| " << setw(6) << left << "ID"
         << " | " << setw(20) << left << "Username"
         << " | " << setw(6) << left << "Rank"
         << " | " << setw(20) << left << "University"
         << " | " << setw(19) << left << "Registered"
         << " | " << setw(12) << left << "Status"
         << " | " << setw(9) << left << "Wildcard"
         << " |\n";
    cout << "---------------------------------------------------------------------------------------------------------------------\n";

    int rowCount = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        string id, username, rank, university, checkInStatus, registrationTime, isWildcard;

        getline(ss, id, ',');
        getline(ss, username, ',');
        getline(ss, rank, ',');
        getline(ss, university, ',');
        getline(ss, checkInStatus, ',');
        getline(ss, registrationTime, ',');
        getline(ss, isWildcard, ',');

        cout << "| " << setw(6) << left << id
             << " | " << setw(20) << left << username
             << " | " << setw(6) << left << rank
             << " | " << setw(20) << left << university
             << " | " << setw(19) << left << registrationTime
             << " | " << setw(12) << left << (checkInStatus == "1" ? "Checked-In" : "Not Checked-In")
             << " | " << setw(9) << left << (isWildcard == "1" ? "Yes" : "No")
             << " |\n";
        rowCount++;
    }
    cout << "=====================================================================================================================\n";
    if (rowCount == 0) {
        cout << "|                                          No player records in CSV file.                                            |\n";
        cout << "=====================================================================================================================\n";
    }
    file.close();
}

int main() {
    CircularQueue queue;
    bool running = true;
    string CSV_FILE = "player.csv";

    queue.initializeNextIDFromCSV(CSV_FILE);

    while (running) {
        cout << "\n------ Tournament Registration (Circular Queue) ------\n";
        cout << "1. Register Player\n";
        cout << "2. Display All Players\n";
        cout << "3. Player Check-In\n";
        cout << "4. Exit\n";
        cout << "-----------------------------------------------------\n";
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string username, university;
            int rank;

            cout << "Enter username: ";
            getline(cin, username);
            cout << "Enter rank: ";
            cin >> rank;
            cin.ignore();
            cout << "Enter university: ";
            getline(cin, university);

            char confirm;
            cout << "Confirm registration? (Y/N): ";
            cin >> confirm;
            cin.ignore();
            if (confirm == 'Y' || confirm == 'y') {
                queue.enqueue(username, rank, university, false);
                queue.saveToCSV(CSV_FILE); // Save immediately after registration
            } else {
                cout << "Registration cancelled.\n";
            }
        }
        else if (choice == 2) {
            displayCSV(CSV_FILE); // Always read and display fresh from CSV
        }
        else if (choice == 3) {
            int idToCheckIn;
            cout << "Enter Player ID to check-in: ";
            cin >> idToCheckIn;
            cin.ignore();

            ifstream inFile(CSV_FILE);
            ofstream tempFile("temp.csv");
            bool found = false;

            string line;
            while (getline(inFile, line)) {
                stringstream ss(line);
                string idStr, username, rank, university, checkInStatus, registrationTime, isWildcard;
                getline(ss, idStr, ',');
                getline(ss, username, ',');
                getline(ss, rank, ',');
                getline(ss, university, ',');
                getline(ss, checkInStatus, ',');
                getline(ss, registrationTime, ',');
                getline(ss, isWildcard, ',');

                if (stoi(idStr) == idToCheckIn) {
                    checkInStatus = "1";
                    found = true;
                    cout << "Player ID " << idToCheckIn << " has been checked in.\n";
                }

                tempFile << idStr << ","
                        << username << ","
                        << rank << ","
                        << university << ","
                        << checkInStatus << ","
                        << registrationTime << ","
                        << isWildcard << "\n";
            }

            inFile.close();
            tempFile.close();
            remove(CSV_FILE.c_str());
            rename("temp.csv", CSV_FILE.c_str());

            if (!found) {
                cout << "Player ID not found.\n";
            }
        }
        else if (choice == 4) {
            running = false;
            cout << "Goodbye!\n";
        }
        else {
            cout << "Invalid menu choice. Try again.\n";
        }
    }

    return 0;
}
