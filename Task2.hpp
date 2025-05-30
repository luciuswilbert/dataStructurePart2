#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cctype> 
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
    bool inTournament;
    Player* next;

    Player(int id, string user, int r, string uni, string regTime, bool wildcard, bool inTour)
        : playerID(id), username(user), rank(r), university(uni),
          checkInStatus(false), registrationTime(regTime),
          isWildcard(wildcard), inTournament(inTour), next(nullptr) {}
};

class CircularQueue {
private:
    Player* front;
    Player* rear;
    int nextID;
    const int maxSize = 12;
    int size;

    string getCurrentTime() {
        time_t now = time(0);
        char buf[80];
        tm* ltm = localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
        return buf;
    }

public:
    CircularQueue() : front(nullptr), rear(nullptr), nextID(1000), size(0) {}

    void loadFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string idStr, username, rankStr, university, checkInStr, timeStr, wildcardStr, inTournamentStr;
            getline(ss, idStr, ',');
            getline(ss, username, ',');
            getline(ss, rankStr, ',');
            getline(ss, university, ',');
            getline(ss, checkInStr, ',');
            getline(ss, timeStr, ',');
            getline(ss, wildcardStr, ',');
            getline(ss, inTournamentStr, ',');

            Player* newPlayer = new Player(
                stoi(idStr), username, stoi(rankStr), university, timeStr,
                wildcardStr == "1", inTournamentStr == "1"
            );
            newPlayer->checkInStatus = (checkInStr == "1");

            if (!front) {
                front = rear = newPlayer;
                newPlayer->next = newPlayer;
            } else {
                rear->next = newPlayer;
                newPlayer->next = front;
                rear = newPlayer;
            }
            size++;
            nextID = max(nextID, stoi(idStr) + 1);
        }
        file.close();
    }

    void saveAllToCSV(const string& filename) {
        ofstream file(filename, ios::trunc);
        if (!file.is_open() || !front) return;
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

    void enqueue(string username, int rank, string university, bool isWildcard = false) {
        bool isInTournament = (size < maxSize);
        Player* newPlayer = new Player(nextID++, username, rank, university, getCurrentTime(), isWildcard, isInTournament);

        if (!front) {
            front = rear = newPlayer;
            newPlayer->next = newPlayer;
        } else {
            rear->next = newPlayer;
            newPlayer->next = front;
            rear = newPlayer;
        }
        size++;
        cout << "Player \"" << username << "\" registered with ID: " << newPlayer->playerID << endl;
        updateTournamentStatus();
    }

    void display() {
        if (!front) {
            cout << "No players in queue.\n";
            return;
        }
        Player* curr = front;
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

        do {
            cout << "| " << setw(6) << left << curr->playerID
                 << " | " << setw(20) << left << curr->username
                 << " | " << setw(6) << left << curr->rank
                 << " | " << setw(20) << left << curr->university
                 << " | " << setw(19) << left << curr->registrationTime
                 << " | " << setw(17) << left << (curr->checkInStatus ? "Checked-In" : "Not Checked-In")
                 << " | " << setw(9) << left << (curr->isWildcard ? "Yes" : "No")
                 << " | " << setw(20) << left << (curr->inTournament ? "In Tournament" : "Waiting")
                 << " |\n";
            curr = curr->next;
        } while (curr != front);
        cout << string(142, '=') << "\n";
    }

    void checkIn(int id) {
        if (!front) return;
        Player* curr = front;
        do {
            if (curr->playerID == id) {
                curr->checkInStatus = true;
                cout << "Player ID " << id << " checked in.\n";
                return;
            }
            curr = curr->next;
        } while (curr != front);
        cout << "Player ID not found.\n";
    }

    void withdraw(int id) {
        if (!front) return;
        Player* curr = front;
        Player* prev = rear;
        do {
            if (curr->playerID == id) {
                if (curr == front && curr == rear) {
                    delete curr;
                    front = rear = nullptr;
                } else if (curr == front) {
                    front = front->next;
                    rear->next = front;
                    delete curr;
                } else if (curr == rear) {
                    prev->next = front;
                    rear = prev;
                    delete curr;
                } else {
                    prev->next = curr->next;
                    delete curr;
                }
                cout << "Player ID " << id << " withdrawn.\n";
                size--;
                updateTournamentStatus();
                return;
            }
            prev = curr;
            curr = curr->next;
        } while (curr != front);
        cout << "Player ID not found.\n";
    }

    int getNextID() const { return nextID; }

    bool exists(int id) {
        if (!front) return false;
        Player* curr = front;
        do {
            if (curr->playerID == id) return true;
            curr = curr->next;
        } while (curr != front);
        return false;
    }

    void displayPlayer(int id) {
        if (!front) {
            cout << "No players in queue.\n";
            return;
        }
        Player* curr = front;
        do {
            if (curr->playerID == id) {
                cout << "\n" << string(50, '=') << "\n";
                cout << "Your Player Info\n";
                cout << string(50, '=') << "\n";
                cout << "ID           : " << curr->playerID << "\n";
                cout << "Username     : " << curr->username << "\n";
                cout << "Rank         : " << curr->rank << "\n";
                cout << "University   : " << curr->university << "\n";
                cout << "Registered   : " << curr->registrationTime << "\n";
                cout << "Check-In     : " << (curr->checkInStatus ? "Checked-In" : "Not Checked-In") << "\n";
                cout << "Wildcard     : " << (curr->isWildcard ? "Yes" : "No") << "\n";
                cout << "Queue Status : " << (curr->inTournament ? "In Tournament" : "Waiting") << "\n";
                cout << string(50, '=') << "\n";
                return;
            }
            curr = curr->next;
        } while (curr != front);
        cout << "Player ID not found.\n";
    }

    void editPlayerInfo(int id) {
        if (!front) return;
        Player* curr = front;
        do {
            if (curr->playerID == id) {
                string newUsername, newUniversity, rankStr;
                int newRank;

                cout << "\nEditing Your Info:\n";

                // Edit username
                while (true) {
                    cout << "Enter new username (leave blank to keep \"" << curr->username << "\"): ";
                    getline(cin, newUsername);
                    if (newUsername.empty()) break;
                    curr->username = newUsername;
                    break;
                }

                // Edit rank
                while (true) {
                    cout << "Enter new rank (current: " << curr->rank << "): ";
                    getline(cin, rankStr);
                    if (rankStr.empty()) break;
                    bool valid = all_of(rankStr.begin(), rankStr.end(), ::isdigit);
                    if (!valid) {
                        cout << "Rank must be a number.\n";
                        continue;
                    }
                    newRank = stoi(rankStr);
                    curr->rank = newRank;
                    break;
                }

                // Edit university
                while (true) {
                    cout << "Enter new university (leave blank to keep \"" << curr->university << "\"): ";
                    getline(cin, newUniversity);
                    if (newUniversity.empty()) break;
                    curr->university = newUniversity;
                    break;
                }

                cout << "Info updated successfully.\n";
                return;
            }
            curr = curr->next;
        } while (curr != front);
        cout << "Player ID not found.\n";
    }

    void updateTournamentStatus() {
        if (!front) return;

        // Collect all players into an array
        Player* playerList[200]; // assume max 200 players
        int count = 0;
        Player* curr = front;

        do {
            playerList[count++] = curr;
            curr = curr->next;
        } while (curr != front);

        // Time parser that cleans up \r or other trailing characters
        auto parseTime = [](const string& timeStr) -> time_t {
            struct tm t = {};
            string cleanedTime = timeStr;
            cleanedTime.erase(remove(cleanedTime.begin(), cleanedTime.end(), '\r'), cleanedTime.end());
            istringstream ss(cleanedTime);
            ss >> get_time(&t, "%Y-%m-%d %H:%M:%S");
            if (ss.fail()) {
                cout << "Failed to parse time: [" << timeStr << "]\n";
                return time(0);
            }
            return mktime(&t);
        };

        // Sort players by wildcard priority, then registration time
        for (int i = 0; i < count - 1; ++i) {
            int minIndex = i;
            for (int j = i + 1; j < count; ++j) {
                bool aIsWildcard = playerList[j]->isWildcard;
                bool bIsWildcard = playerList[minIndex]->isWildcard;

                if (aIsWildcard && !bIsWildcard) {
                    minIndex = j;
                } else if (aIsWildcard == bIsWildcard) {
                    time_t timeA = parseTime(playerList[j]->registrationTime);
                    time_t timeB = parseTime(playerList[minIndex]->registrationTime);
                    if (timeA < timeB) {
                        minIndex = j;
                    }
                }
            }
            if (minIndex != i) {
                Player* temp = playerList[i];
                playerList[i] = playerList[minIndex];
                playerList[minIndex] = temp;
            }
        }

        // Update inTournament flags
        for (int i = 0; i < count; ++i) {
            playerList[i]->inTournament = (i < maxSize); // top 12 get in
        }

        // Rebuild circular linked list in new order
        for (int i = 0; i < count - 1; ++i) {
            playerList[i]->next = playerList[i + 1];
        }
        playerList[count - 1]->next = playerList[0];

        front = playerList[0];
        rear = playerList[count - 1];
    }
};
struct Wildcard {
    string code;
    string username;
    int rank;
    string university;
    bool used;
    Wildcard* next;

    Wildcard(string c, string u, int r, string uni, bool isUsed)
        : code(c), username(u), rank(r), university(uni), used(isUsed), next(nullptr) {}
};

class WildcardQueue {
private:
    Wildcard* head = nullptr;

public:
    void loadFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) return;

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string code, username, rankStr, university, usedStr;
            getline(ss, code, ',');
            getline(ss, username, ',');
            getline(ss, rankStr, ',');
            getline(ss, university, ',');
            getline(ss, usedStr, ',');

            int rank = stoi(rankStr);
            bool used = (usedStr == "1");

            addWildcard(code, username, rank, university, used, false); // don't print
        }
        file.close();
    }

    void saveToCSV(const string& filename) {
        ofstream file(filename, ios::trunc);
        Wildcard* curr = head;
        while (curr) {
            file << curr->code << "," << curr->username << "," << curr->rank << ","
                << curr->university << "," << (curr->used ? "1" : "0") << "\n";
            curr = curr->next;
        }
        file.close();
    }

    void addWildcard(string code, string username, int rank, string university, bool used = false, bool showConfirm = true) {
        Wildcard* newNode = new Wildcard(code, username, rank, university, used);

        if (!head || rank < head->rank) {
            newNode->next = head;
            head = newNode;
        } else {
            Wildcard* curr = head;
            while (curr->next && curr->next->rank <= rank) {
                curr = curr->next;
            }
            newNode->next = curr->next;
            curr->next = newNode;
        }

        if (showConfirm) {
            cout << "Wildcard for " << username << " (Code: " << code << ") added.\n";
        }
    }

    bool redeemWildcard(const string& code, string& username, int& rank, string& university) {
        Wildcard* curr = head;
        Wildcard* prev = nullptr;

        while (curr) {
            if (curr->code == code && !curr->used) {
                // Extract info
                username = curr->username;
                rank = curr->rank;
                university = curr->university;

                // Remove the used node from the list
                if (prev) {
                    prev->next = curr->next;
                } else {
                    head = curr->next;
                }
                delete curr;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }
    void displayWildcards() {
        if (!head) {
            cout << "No wildcard entries.\n";
            return;
        }

        Wildcard* curr = head;

        cout << "\n" << string(90, '=') << "\n";
        cout << "| " << setw(12) << left << "Coupon Code"
            << "| " << setw(20) << left << "Username"
            << "| " << setw(6)  << left << "Rank"
            << "| " << setw(20) << left << "University"
            << "| " << setw(10) << left << "Used"
            << "|\n";
        cout << string(90, '-') << "\n";

        while (curr) {
            cout << "| " << setw(12) << left << curr->code
                << "| " << setw(20) << left << curr->username
                << "| " << setw(6)  << left << curr->rank
                << "| " << setw(20) << left << curr->university
                << "| " << setw(10) << left << (curr->used ? "Yes" : "No")
                << "|\n";
            curr = curr->next;
        }

        cout << string(90, '=') << "\n";
    }
};