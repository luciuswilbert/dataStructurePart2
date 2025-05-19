#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
using namespace std;

struct Player {
    int playerID;
    string username;
    int rank;
    string university;
    bool checkInStatus;
    string registrationTime;
    bool isWildcard;
    bool inTournament; // flag to indicate if the player is in the tournament
    Player* next; // pointer to next player in the queue

    Player(int id, string user, int r, string uni, string regTime, bool wildcard, bool inTour)
    : playerID(id), username(user), rank(r), university(uni),
      checkInStatus(false), registrationTime(regTime), isWildcard(wildcard),
      inTournament(inTour), next(nullptr) {}

};

// ------ Circular Queue using Linked List ------
class CircularQueue {
protected:
    Player* front;
    Player* rear;
    int nextID;
    int size = 0;
    const int maxSize = 12;

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
        int maxID = 999; // start before 1000

        while (getline(file, line)) {
            stringstream ss(line);
            string idStr;
            getline(ss, idStr, ','); // get the first token (ID)

            try {
                int id = stoi(idStr);
                if (id > maxID) {
                    maxID = id;
                }
            } catch (...) {
                continue; // skip invalid lines safely
            }
        }

        nextID = maxID + 1; // use next available ID
        file.close();
    }


    // Add a new player to the queue (enqueue)
    void enqueue(string username, int rank, string university, bool isWildcard = false) {
        initializeNextIDFromCSV("player.csv"); // ensure latest ID
        bool isInTournament = (size < maxSize);  // Only first 12 are active
        Player* newPlayer = new Player(nextID++, username, rank, university, getCurrentTime(), isWildcard, isInTournament);

        if (!front) {
            front = rear = newPlayer;
            newPlayer->next = newPlayer; // Circular!
        } else {
            rear->next = newPlayer;
            newPlayer->next = front;
            rear = newPlayer;
        }
        size++;
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
                << (rear->isWildcard ? "1" : "0") << ","
                << (rear->inTournament ? "1" : "0") << "\n";
        }
        file.close();
        cout << "Registration saved to " << filename << endl;
    }

    void withdrawPlayer(int id) {
        if (!front) {
            cout << "Queue is empty.\n";
            return;
        }

        Player* curr = front;
        Player* prev = rear;
        bool found = false;

        do {
            if (curr->playerID == id && curr->inTournament) {
                found = true;

                cout << "Player ID " << id << " has been withdrawn from the tournament.\n";

                if (curr == front && curr == rear) {
                    delete curr;
                    front = rear = nullptr;
                }
                else if (curr == front) {
                    front = front->next;
                    rear->next = front;
                    delete curr;
                }
                else if (curr == rear) {
                    prev->next = front;
                    rear = prev;
                    delete curr;
                }
                else {
                    prev->next = curr->next;
                    delete curr;
                }

                size--;
                break; // Exit the loop after deleting
            }

            prev = curr;
            curr = curr->next;
        } while (curr != front);

        if (!found) {
            cout << "No active tournament player with ID " << id << " found.\n";
            return;
        }

        // Promote the first waiting player
        curr = front;
        do {
            if (!curr->inTournament) {
                curr->inTournament = true;
                size++;

                cout << "Promoted waiting player ID " << curr->playerID << " into tournament.\n";
                return;
            }
            curr = curr->next;
        } while (curr != front);

        cout << "No waiting players to promote.\n";
    }


    void rewriteFullCSV(const string& filename) {
        ofstream file(filename, ios::trunc); // overwrite mode
        if (!file.is_open()) {
            cout << "Error: Could not overwrite file.\n";
            return;
        }

        if (!front) {
            file.close();
            return;
        }

        Player* curr = front;
        do {
            file << curr->playerID << "," << curr->username << "," << curr->rank << ","
                << curr->university << "," << (curr->checkInStatus ? "1" : "0") << ","
                << curr->registrationTime << "," << (curr->isWildcard ? "1" : "0") << ","
                << (curr->inTournament ? "1" : "0") << "\n";
            curr = curr->next;
        } while (curr != front);

        file.close();
    }

    int getNextIDForPreview() const {
        return nextID;
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

    cout << "\n" << string(142, '=') << "\n";
    cout << "| " << setw(6) << left << "ID"
         << " | " << setw(20) << left << "Username"
         << " | " << setw(6) << left << "Rank"
         << " | " << setw(20) << left << "University"
         << " | " << setw(19) << left << "Registered"
         << " | " << setw(17) << left << "Status"
         << " | " << setw(9) << left << "Wildcard"
         << " | " << setw(20) << left << "Queue"
         << " |\n";
    cout << string(142, '-') << "\n";

    int rowCount = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        string id, username, rank, university, checkInStatus, registrationTime, isWildcard, inTournament;

        getline(ss, id, ',');
        getline(ss, username, ',');
        getline(ss, rank, ',');
        getline(ss, university, ',');
        getline(ss, checkInStatus, ',');
        getline(ss, registrationTime, ',');
        getline(ss, isWildcard, ',');
        getline(ss, inTournament, ',');

        if (inTournament == "1") {
            cout << "| " << setw(6) << left << id
                << " | " << setw(20) << left << username
                << " | " << setw(6) << left << rank
                << " | " << setw(20) << left << university
                << " | " << setw(19) << left << registrationTime
                << " | " << setw(17) << left << (checkInStatus == "1" ? "Checked-In" : "Not Checked-In")
                << " | " << setw(9) << left << (isWildcard == "1" ? "Yes" : "No")
                << " | " << setw(20) << left << (inTournament == "1" ? "In Tournament" : "Not In Tournament")
                << " |\n";
            rowCount++;
        }
    }
    cout << string(142, '=') << "\n";
    if (rowCount == 0) {
        cout << "|                                          No player records in CSV file.                                            |\n";
        cout << "=====================================================================================================================\n";
    }
    file.close();
}

int main() {
    CircularQueue queue;
    bool running = true;
    string userRole = "";
    int currentPlayerID = -1;
    int loginChoice = 0;
    string adminID = "AdmiN"; // Placeholder for admin ID
    string adminID2;
    string CSV_FILE = "player.csv";

    while (loginChoice != 4) {
        cout << "\n====== Welcome to APUEC Tournament System ======\n";
        cout << "1. Admin Login\n";
        cout << "2. Player Registration\n";
        cout << "3. Player Login\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> loginChoice;
        cin.ignore();

        if (loginChoice == 1) {
            while (true) {
                cout << "Enter Admin ID (case-sensitive): ";
                cin >> adminID2;

                if (adminID2 == adminID) {
                    cout << "Admin login successful.\n";
                    userRole = "admin";
                    break; // break from inner loop
                } else {
                    cout << "Invalid Admin ID. Please try again.\n";
                    if (adminID2 == "exit") {
                        break; // go back to login menu
                    }
                }
            }

            if (userRole == "admin") break; // break from main login loop into system
        }
        else if (loginChoice == 2) {
            string username, university;
            int rank;

            cout << "\n--- Player Registration ---\n";
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
                CircularQueue tempQueue;
                tempQueue.initializeNextIDFromCSV(CSV_FILE);

                // Store the next ID before enqueue
                int assignedID = tempQueue.getNextIDForPreview();

                tempQueue.enqueue(username, rank, university, false);
                tempQueue.saveToCSV(CSV_FILE);

                cout << "\nRegistration complete.\n";
                cout << "Your assigned Player ID is: " << assignedID << "\n";
                cout << "Please remember this ID to log in later.\n";
            } else {
                cout << "Registration cancelled.\n";
            }
        }
        else if (loginChoice == 3) {
            cout << "\n--- Player Login ---\n";
            cout << "Enter your Player ID: ";
            cin >> currentPlayerID;
            cin.ignore();

            ifstream file(CSV_FILE);
            if (!file.is_open()) {
                cout << "Error: Could not open player file.\n";
                continue;
            }

            bool found = false;
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string idStr;
                getline(ss, idStr, ',');

                if (stoi(idStr) == currentPlayerID) {
                    found = true;
                    userRole = "player";
                    break;
                }
            }
            file.close();

            if (found) {
                cout << "Login successful. Welcome Player " << currentPlayerID << "!\n";
                break; // go to player menu
            } else {
                cout << "Player ID not found. Please register first.\n";
                currentPlayerID = -1;
            }
        }
        else if (loginChoice == 4) {
            cout << "Goodbye!\n";
            return 0; // Exit program
        }
        else {
            cout << "Invalid choice. Please try again.\n";
        }
    }


    queue.initializeNextIDFromCSV(CSV_FILE);

    while (running) {
        if (userRole == "admin"){
            cout << "\n------ Tournament Registration (Circular Queue) ------\n";
            cout << "1. Register Player\n";
            cout << "2. Display All Players\n";
            cout << "3. Player Check-In\n";
            cout << "4. Withdraw Player\n";
            cout << "5. Exit\n";
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
                    string idStr, username, rank, university, checkInStatus, registrationTime, isWildcard, inTournament;
                    getline(ss, idStr, ',');
                    getline(ss, username, ',');
                    getline(ss, rank, ',');
                    getline(ss, university, ',');
                    getline(ss, checkInStatus, ',');
                    getline(ss, registrationTime, ',');
                    getline(ss, isWildcard, ',');
                    getline(ss, inTournament, ',');

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
                            << isWildcard << ","
                            << inTournament << "\n";
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
                queue.withdrawPlayer(currentPlayerID);
                queue.rewriteFullCSV(CSV_FILE);
            }
            else if (choice == 5) {
                running = false;
                cout << "Goodbye!\n";
            }
            else {
                cout << "Invalid menu choice. Try again.\n";
            }
        }
        else if (userRole == "player") {
            cout << "\n------ Player Menu ------\n";
            cout << "1. View My Info\n";
            cout << "2. Check-In\n";
            cout << "3. Withdraw from Tournament\n";
            cout << "4. Exit\n";
            cout << "--------------------------\n";
            cout << "Enter your choice: ";
            int choice;
            cin >> choice;
            cin.ignore();

            if (choice == 1) {
                ifstream file(CSV_FILE);
                if (!file.is_open()) {
                    cout << "Error: Could not open player file.\n";
                } else {
                    string line;
                    bool found = false;

                    while (getline(file, line)) {
                        stringstream ss(line);
                        string idStr, username, rank, university, checkInStatus, registrationTime, isWildcard, inTournament;
                        getline(ss, idStr, ',');
                        getline(ss, username, ',');
                        getline(ss, rank, ',');
                        getline(ss, university, ',');
                        getline(ss, checkInStatus, ',');
                        getline(ss, registrationTime, ',');
                        getline(ss, isWildcard, ',');
                        getline(ss, inTournament, ',');

                        if (stoi(idStr) == currentPlayerID) {
                            found = true;
                            cout << "\n------ Your Player Info ------\n";
                            cout << "Player ID      : " << idStr << "\n";
                            cout << "Username       : " << username << "\n";
                            cout << "Rank           : " << rank << "\n";
                            cout << "University     : " << university << "\n";
                            cout << "Check-In       : " << (checkInStatus == "1" ? "Checked-In" : "Not Checked-In") << "\n";
                            cout << "Wildcard Entry : " << (isWildcard == "1" ? "Yes" : "No") << "\n";
                            cout << "Tournament     : " << (inTournament == "1" ? "In Tournament" : "Waiting") << "\n";
                            cout << "Registered On  : " << registrationTime << "\n";
                            break;
                        }
                    }

                    if (!found) {
                        cout << "Player info not found.\n";
                    }

                    file.close();
                }
            }
            else if (choice == 2) {
                ifstream inFile(CSV_FILE);
                ofstream tempFile("temp.csv");
                bool found = false;

                string line;
                while (getline(inFile, line)) {
                    stringstream ss(line);
                    string idStr, username, rank, university, checkInStatus, registrationTime, isWildcard, inTournament;
                    getline(ss, idStr, ',');
                    getline(ss, username, ',');
                    getline(ss, rank, ',');
                    getline(ss, university, ',');
                    getline(ss, checkInStatus, ',');
                    getline(ss, registrationTime, ',');
                    getline(ss, isWildcard, ',');
                    getline(ss, inTournament, ',');

                    if (stoi(idStr) == currentPlayerID) {
                        checkInStatus = "1";
                        found = true;
                        cout << "You have checked in successfully.\n";
                    }

                    tempFile << idStr << ","
                            << username << ","
                            << rank << ","
                            << university << ","
                            << checkInStatus << ","
                            << registrationTime << ","
                            << isWildcard << ","
                            << inTournament << "\n";
                }

                inFile.close();
                tempFile.close();
                remove(CSV_FILE.c_str());
                rename("temp.csv", CSV_FILE.c_str());

                if (!found) {
                    cout << "Player ID not found.\n";
                }
            }
            else if (choice == 3) {
                // Withdraw from tournament
                
            }
            else if (choice == 4) {
                running = false;
                cout << "Goodbye!\n";
            }
            else {
                cout << "Invalid menu choice. Try again.\n";
            }
        }
    }
        

    return 0;
}
