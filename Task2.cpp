#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cctype> 
#include <algorithm>
#include "Task2.hpp"
using namespace std;

int main() {
    CircularQueue queue;
    WildcardQueue wildcardQueue;
    string filename = "player.csv";
    wildcardQueue.loadFromCSV("wildcard.csv");

    queue.loadFromCSV(filename);
    queue.updateTournamentStatus();

    bool running = true;
    string userRole = "";
    int currentPlayerID = -1;

    while (running) {
        cout << "\n====== Welcome to APUEC Tournament System ======\n";
        cout << "1. Admin Login\n";
        cout << "2. Player Registration\n";
        cout << "3. Player Login\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string adminID;
            cout << "Enter Admin ID (case-sensitive): ";
            cin >> adminID;
            if (adminID == "AdmiN") {
                userRole = "admin";
                break;
            } else {
                cout << "Invalid Admin ID.\n";
            }
        }
        else if (choice == 2) {
            cout << "\nPlayer Registration:\n";
            cout << "1. Normal Registration\n";
            cout << "2. Wildcard Registration\n";
            cout << "Enter choice: ";
            int regType;
            cin >> regType;
            cin.ignore();

            if (regType == 1) {
                string name, uni;
                int rank;
                cout << "Enter username: ";
                getline(cin, name);
                cout << "Enter rank: ";
                cin >> rank;
                cin.ignore();
                cout << "Enter university: ";
                getline(cin, uni);
                queue.enqueue(name, rank, uni);
            }
            else if (regType == 2) {
                string code, name, uni;
                int rank;
                cout << "Enter your wildcard coupon code: ";
                getline(cin, code);

                if (wildcardQueue.redeemWildcard(code, name, rank, uni)) {
                    queue.enqueue(name, rank, uni, true); // isWildcard = true
                    cout << "Registered via wildcard.\n";
                } else {
                    cout << "Invalid or already used coupon code.\n";
                }
            }
            else {
                cout << "Invalid choice.\n";
            }
        }
        else if (choice == 3) {
            cout << "Enter your Player ID: ";
            cin >> currentPlayerID;
            cin.ignore();

            if (queue.exists(currentPlayerID)) {
                cout << "Successfully logged in as Player ID: " << currentPlayerID << endl;
                userRole = "player";
                break;
            } else {
                cout << "Player ID not found. Please try again.\n";
            }
        }
        else if (choice == 4) {
            queue.saveAllToCSV(filename);
            wildcardQueue.saveToCSV("wildcard.csv");
            cout << "Exiting program.\n";
            return 0;
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

    while (running) {
        if (userRole == "admin") {
            cout << "\n--- Admin Menu ---\n";
            cout << "1. Register Player\n2. Display All Players\n3. Check-In Player\n4. Withdraw Player\n5. Edit Player Info\n6. Make Wildcard\n7. View All Wildcard Entries\n8. Exit\nEnter choice: ";
            int ch, id, rank;
            string name, uni;
            cin >> ch;
            cin.ignore();
            if (ch == 1) {
                while (true) {
                    cout << "Enter username: ";
                    getline(cin, name);
                    if (name.empty()) {
                        cout << "Username must not be empty.\n";
                        continue;
                    }
                    break;
                }

                string rankStr;
                while (true) {
                    cout << "Enter rank: ";
                    getline(cin, rankStr);
                    if (rankStr.empty()) {
                        cout << "Rank must not be empty.\n";
                        continue;
                    }
                    bool valid = all_of(rankStr.begin(), rankStr.end(), ::isdigit);
                    if (!valid) {
                        cout << "Rank must be a positive integer.\n";
                        continue;
                    }
                    rank = stoi(rankStr); // safely convert
                    break;
                }

                while (true) {
                    cout << "Enter university: ";
                    getline(cin, uni);
                    if (uni.empty()) {
                        cout << "University must not be empty.\n";
                        continue;
                    }
                    break;
                }

                queue.enqueue(name, rank, uni);
            } else if (ch == 2) {
                queue.display();
            } else if (ch == 3) {
                cout << "Enter Player ID to check-in: "; cin >> id;
                queue.checkIn(id);
            } else if (ch == 4) {
                cout << "Enter Player ID to withdraw: "; cin >> id;
                queue.withdraw(id);
            } else if (ch == 8) {
                queue.saveAllToCSV(filename);
                wildcardQueue.saveToCSV("wildcard.csv");
                cout << "Exiting admin mode.\n";
                break;
            } else if (ch == 6) {
                string code, name, uni;
                int rank;

                cout << "Enter coupon code (no spaces): ";
                getline(cin, code);
                cout << "Enter username: ";
                getline(cin, name);
                cout << "Enter rank: ";
                cin >> rank;
                cin.ignore();
                cout << "Enter university: ";
                getline(cin, uni);

                wildcardQueue.addWildcard(code, name, rank, uni);
            } else if (ch == 5) {
                cout << "Enter Player ID to edit: ";
                cin >> id;
                cin.ignore();
                if (queue.exists(id)) {
                    queue.editPlayerInfo(id);
                    // queue.saveAllToCSV(filename);
                } else {
                    cout << "Player ID not found.\n";
                }
            } else if (ch == 7) {
                wildcardQueue.displayWildcards();
            } else {
                cout << "Invalid choice!\n";
            }
        }
        else if (userRole == "player") {
            cout << "\n--- Player Menu ---\n";
            cout << "1. View My Info\n2. Check-In\n3. Withdraw\n4. Edit Info\n5. Exit\nEnter choice: ";
            int ch;
            cin >> ch;
            cin.ignore();
            if (ch == 1) {
                queue.displayPlayer(currentPlayerID); 
            }
            else if (ch == 2) {
                queue.checkIn(currentPlayerID);
            } else if (ch == 3) {
                queue.withdraw(currentPlayerID);
                queue.saveAllToCSV(filename);
                cout << "You have been withdrawn. Exiting player mode.\n";
                break;
            } else if (ch == 5) {
                queue.saveAllToCSV(filename);
                cout << "Exiting player mode.\n";
                wildcardQueue.saveToCSV("wildcard.csv");
                break;
            } else if (ch == 4) {
                queue.editPlayerInfo(currentPlayerID);
            }else {
                cout << "Invalid choice!\n";
            }
            
        }
    }
    return 0;
}