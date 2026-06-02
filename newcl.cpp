#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>           
#include <iomanip>

using namespace std;

#define MAX_TEAMS 36
#define MATCHES_PER_TEAM 8

// Core Data Structures
struct Teams {
    string name;
    int points;
    int GF;
    int GA;
    int GD;
    int wins;
    int draws;
    int losses;
    int opponents[MATCHES_PER_TEAM];
};

struct TeamNode {
    Teams team;
    TeamNode* next;
};

// Stack Implementation for Match History
class MatchStack {
private:
    struct MatchNode {
        string team1;
        string team2;
        int score1;
        int score2;
        MatchNode* next;

        MatchNode(string t1, string t2, int s1, int s2) {
            team1 = t1;
            team2 = t2;
            score1 = s1;
            score2 = s2;
            next = nullptr;
        }
    };

    MatchNode* top;

    bool compareIgnoreCase(const string& a, const string& b) const {
        if (a.length() != b.length()) return false;
        for (size_t i = 0; i < a.length(); i++) {      
            if (tolower(a[i]) != tolower(b[i])) return false;
        }
        return true;
    }

public:
    MatchStack() {
        top = nullptr;
    }

    ~MatchStack() {
        while (top != nullptr) {
            MatchNode* temp = top;
            top = top->next;
            delete temp;
        }
    }

    void push(string t1, string t2, int s1, int s2, bool quiet = false) {
        MatchNode* newNode = new MatchNode(t1, t2, s1, s2);
        newNode->next = top;                    
        top = newNode;

        if (!quiet) {          
            cout << "Match added: "
                << t1 << " " << s1 << "-" << s2 << " " << t2 << endl;
        }
    }

    void showFavoriteTeamHistory(const string& teamName) const {       
        if (top == nullptr) {
            cout << "No match history available.\n";
            return;
        }

        MatchNode* current = top;
        bool found = false;
        cout << "\n=== Match History for " << teamName << " ===\n";
        cout << "(Showing most recent matches first)\n";

        while (current != nullptr) {
            if (compareIgnoreCase(current->team1, teamName) || compareIgnoreCase(current->team2, teamName)) {    //////
                if (compareIgnoreCase(current->team1, teamName)) {
                    cout << current->team1 << " " << current->score1 << " - "
                        << current->score2 << " " << current->team2 << endl;
                }
                else {
                    cout << current->team2 << " " << current->score2 << " - "
                        << current->score1 << " " << current->team1 << endl;
                }
                found = true;
            }
            current = current->next;
        }

        if (!found) {
            cout << "No matches found for this team.\n";
        }
        cout << "=====================================\n";
    }
};

// Initialization and Sorting
void loadTeams(const string& filename, Teams teams[], int& team_count) {
    ifstream file(filename);
    team_count = 0;
    if (!file.is_open()) {
        cout << "Error: File '" << filename << "' not found.\n";
        return;
    }

    string name;
    while (file >> name) {
        teams[team_count].name = name;
        teams[team_count].points = 0;
        teams[team_count].GF = 0;
        teams[team_count].GA = 0;
        teams[team_count].GD = 0;
        teams[team_count].wins = 0;
        teams[team_count].draws = 0;
        teams[team_count].losses = 0;

        for (int k = 0; k < MATCHES_PER_TEAM; k++)
            teams[team_count].opponents[k] = -1;

        team_count++;
        if (team_count >= MAX_TEAMS) break;
    }
    file.close();
}

void SortTeams(Teams teams[], int team_count) {
    Teams temp;
    for (int i = 0; i < team_count - 1; i++) {
        for (int j = 0; j < team_count - i - 1; j++) {
            bool swap = false;

            if (teams[j].points < teams[j + 1].points) {
                swap = true;
            }
            else if (teams[j].points == teams[j + 1].points && teams[j].GD < teams[j + 1].GD) {
                swap = true;
            }
            else if (teams[j].points == teams[j + 1].points && teams[j].GD == teams[j + 1].GD && teams[j].GF < teams[j + 1].GF) {
                swap = true;
            }

            if (swap) {
                temp = teams[j];
                teams[j] = teams[j + 1];
                teams[j + 1] = temp;
            }
        }
    }
}

void UpdateStats(Teams* t1, Teams* t2, int s1, int s2) {
    t1->GF += s1;
    t1->GA += s2;
    t1->GD = t1->GF - t1->GA;

    t2->GF += s2;
    t2->GA += s1;
    t2->GD = t2->GF - t2->GA;

    if (s1 > s2) {
        t1->points += 3;
        t1->wins++;
        t2->losses++;
    }
    else if (s1 == s2) {
        t1->points += 1;
        t1->draws++;
        t2->points += 1;
        t2->draws++;
    }
    else {
        t2->points += 3;
        t2->wins++;
        t1->losses++;
    }
}

// Linked List Management & Synchronization
TeamNode* BuildLinkedList(Teams teams[], int n) {  
    TeamNode* head = nullptr;
    TeamNode* tail = nullptr;
    for (int i = 0; i < n; i++) {
        TeamNode* node = new TeamNode;
        node->team = teams[i];
        node->next = nullptr;

        if (!head) {
            head = tail = node;
        }
        else {
            tail->next = node;
            tail = node;
        }
    }
    return head;
}

void CopyListToArray(TeamNode* head, Teams teams[], int& team_count) {
    team_count = 0;
    while (head != nullptr && team_count < MAX_TEAMS) {
        teams[team_count] = head->team;
        head = head->next;
        team_count++;
    }
}

void DisplayStandings(const TeamNode* head) {
    cout << left << setw(5) << "Pos"
        << setw(20) << "Team"
        << setw(6) << "Pts"
        << setw(4) << "W"
        << setw(4) << "D"
        << setw(4) << "L"
        << setw(6) << "GF"
        << setw(6) << "GA"
        << setw(6) << "GD" << endl;

    cout << string(70, '-') << endl;
    const TeamNode* curr = head;
    int pos = 1;
    while (curr != nullptr) {
        const Teams* t = &curr->team;
        cout << left << setw(5) << pos
            << setw(20) << t->name
            << setw(6) << t->points
            << setw(4) << t->wins
            << setw(4) << t->draws
            << setw(4) << t->losses
            << setw(6) << t->GF
            << setw(6) << t->GA
            << setw(6) << t->GD << endl;
        curr = curr->next;
        pos++;
    }
}

// Group Stage Mechanics           // Round Robin Algorithm
void ManualMatchEntry(Teams teams[], int team_count, MatchStack& history) {
    if (team_count < MAX_TEAMS) {
        cout << "Error: Need " << MAX_TEAMS << " teams loaded.\n";
        return;
    }

    int indices[MAX_TEAMS];
    for (int i = 0; i < MAX_TEAMS; i++) indices[i] = i;

    cout << "\n=== MANUAL MATCH ENTRY MODE ===\n";

    for (int round = 0; round < MATCHES_PER_TEAM; round++) {
        cout << "\n--- ROUND " << round + 1 << " ---\n";

        for (int i = 0; i < MAX_TEAMS / 2; i++) {
            int t1 = indices[i];
            int t2 = indices[MAX_TEAMS - 1 - i];

            teams[t1].opponents[round] = t2;
            teams[t2].opponents[round] = t1;

            int s1, s2;
            cout << teams[t1].name << " vs " << teams[t2].name << ": ";

            while (!(cin >> s1 >> s2) || s1 < 0 || s2 < 0) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input. Enter two non-negative integers: ";
            }

            history.push(teams[t1].name, teams[t2].name, s1, s2);
            UpdateStats(&teams[t1], &teams[t2], s1, s2);
        }

        int last = indices[MAX_TEAMS - 1];
        for (int i = MAX_TEAMS - 1; i > 1; i--) {
            indices[i] = indices[i - 1];
        }
        indices[1] = last;
    }

    cout << "\nAll matches entered! Updating table...\n";
    SortTeams(teams, team_count);

    TeamNode* head = BuildLinkedList(teams, team_count);
    DisplayStandings(head);
    while (head) {
        TeamNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void SimulateGroupStage(Teams teams[], int team_count, MatchStack& history) {
    if (team_count < MAX_TEAMS) {
        cout << "Error: Need " << MAX_TEAMS << " teams loaded.\n";
        return;
    }

    int indices[MAX_TEAMS];
    for (int i = 0; i < MAX_TEAMS; i++) indices[i] = i;

    cout << "\n--- Simulating Group Stage... ---\n";

    for (int round = 0; round < MATCHES_PER_TEAM; round++) {
        for (int i = 0; i < MAX_TEAMS / 2; i++) {
            int t1 = indices[i];
            int t2 = indices[MAX_TEAMS - 1 - i];

            teams[t1].opponents[round] = t2;
            teams[t2].opponents[round] = t1;

            int s1 = rand() % 5;
            int s2 = rand() % 5;

            history.push(teams[t1].name, teams[t2].name, s1, s2, true);
            UpdateStats(&teams[t1], &teams[t2], s1, s2);
        }

        int last = indices[MAX_TEAMS - 1];
        for (int i = MAX_TEAMS - 1; i > 1; i--) {
            indices[i] = indices[i - 1];
        }
        indices[1] = last;
    }

    cout << "Simulation Complete! Updating table...\n";
    SortTeams(teams, team_count);

    TeamNode* head = BuildLinkedList(teams, team_count);
    DisplayStandings(head);
    while (head) {
        TeamNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// Knockout Stage Mechanics
int PlayTwoLegs(Teams* home, Teams* away, const string& stage_name, MatchStack& history) {
    int h1, a1, h2, a2;
    cout << "\n" << stage_name << " (Leg 1): " << home->name << " vs " << away->name << "\nEnter Score: ";

    while (!(cin >> h1 >> a1) || h1 < 0 || a1 < 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input. Enter two non-negative integers: ";
    }
    history.push(home->name, away->name, h1, a1);

    cout << stage_name << " (Leg 2): " << away->name << " vs " << home->name << "\nEnter Score: ";
    while (!(cin >> h2 >> a2) || h2 < 0 || a2 < 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input. Enter two non-negative integers: ";
    }
    history.push(away->name, home->name, h2, a2);

    int agg_home = h1 + a2;
    int agg_away = a1 + h2;

    cout << "Aggregate: " << home->name << " " << agg_home << " - " << agg_away << " " << away->name << endl;

    if (agg_home > agg_away) return 0;
    else if (agg_away > agg_home) return 1;
    else {
        int pen;
        cout << "Tie! Penalties? 1-" << home->name << " 2-" << away->name << ": ";
        while (!(cin >> pen) || (pen != 1 && pen != 2)) {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "Invalid. Enter 1 or 2: ";
        }
        return (pen == 1) ? 0 : 1;
    }
}

int PlayFinal(Teams* t1, Teams* t2, MatchStack& history) {
    int s1, s2;
    cout << "\n=== FINAL ===\n" << t1->name << " vs " << t2->name << "\nEnter Score: ";

    while (!(cin >> s1 >> s2) || s1 < 0 || s2 < 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input. Enter two non-negative integers: ";
    }
    history.push(t1->name, t2->name, s1, s2);

    if (s1 > s2) return 0;
    else if (s2 > s1) return 1;

    int pen;
    cout << "Tie! Penalties? 1-" << t1->name << " 2-" << t2->name << ": ";
    while (!(cin >> pen) || (pen != 1 && pen != 2)) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid. Enter 1 or 2: ";
    }
    return (pen == 1) ? 0 : 1;
}

void PlayKnockoutStage(Teams teams[], int team_count, MatchStack& history) {
    if (team_count < 24) {
        cout << "Not enough teams to run knockout stage.\n";
        return;
    }

    SortTeams(teams, team_count);
    Teams* top8[8];
    Teams* playoff_winners[8];
    Teams* r16_winners[8];
    Teams* qf_winners[4];
    Teams* sf_winners[2];

    for (int i = 0; i < 8; i++) top8[i] = &teams[i];

    cout << "\n--- Play-offs ---\n";
    for (int i = 0; i < 8; i++) {
        int winner = PlayTwoLegs(&teams[8 + i], &teams[23 - i], "Play-off", history);
        if (winner == 0) playoff_winners[i] = &teams[8 + i];
        else playoff_winners[i] = &teams[23 - i];
    }

    cout << "\n--- Round of 16 ---\n";
    for (int i = 0; i < 8; i++) {
        int winner = PlayTwoLegs(top8[i], playoff_winners[i], "Round of 16", history);
        if (winner == 0) r16_winners[i] = top8[i];
        else r16_winners[i] = playoff_winners[i];
    }

    cout << "\n--- Quarter Finals ---\n";
    for (int i = 0; i < 4; i++) {
        int winner = PlayTwoLegs(r16_winners[i * 2], r16_winners[i * 2 + 1], "Quarter Final", history);
        if (winner == 0) qf_winners[i] = r16_winners[i * 2];
        else qf_winners[i] = r16_winners[i * 2 + 1];
    }

    cout << "\n--- Semi Finals ---\n";
    for (int i = 0; i < 2; i++) {
        int winner = PlayTwoLegs(qf_winners[i * 2], qf_winners[i * 2 + 1], "Semi Final", history);
        if (winner == 0) sf_winners[i] = qf_winners[i * 2];
        else sf_winners[i] = qf_winners[i * 2 + 1];
    }

    int champ_code = PlayFinal(sf_winners[0], sf_winners[1], history);
    Teams* champion = (champ_code == 0) ? sf_winners[0] : sf_winners[1];

    cout << "\n********** WINNER: " << champion->name << " **********\n";
}

void StartNewSeason(Teams teams[], int team_count, MatchStack& history) {
    int choice;
    cout << "\n1. Manual Group Matches\n"
        << "2. Simulate Group Matches\n"
        << "3. Back\n"
        << "Choice: ";

    while (!(cin >> choice)) { 
        cin.clear(); cin.ignore(1000, '\n');
    }

    if (choice == 3) return;

    if (choice == 1) {
        ManualMatchEntry(teams, team_count, history);
    }
    else if (choice == 2) {
        SimulateGroupStage(teams, team_count, history);
    }
    else {
        cout << "Invalid choice.\n";
        return;
    }

    cin.ignore();
    cout << "Press Enter for Knockout...";
    cin.get();

    PlayKnockoutStage(teams, team_count, history);
}

// Team Data Modification (Linked List Implementation)
void InsertTeam(TeamNode*& head, Teams teams[], int& team_count) {
    if (team_count >= MAX_TEAMS) {
        cout << "League already has 36 teams.\n";
        return;
    }

    string name;
    cout << "Enter team name to insert: ";
    cin >> name;

    TeamNode* node = new TeamNode;
    node->team.name = name;
    node->team.points = 0;
    node->team.GF = 0;
    node->team.GA = 0;
    node->team.GD = 0;
    node->team.wins = 0;
    node->team.draws = 0;
    node->team.losses = 0;

    for (int i = 0; i < MATCHES_PER_TEAM; i++) {
        node->team.opponents[i] = -1;
    }
    node->next = nullptr;

    if (!head) {
        head = node;
    }
    else {
        TeamNode* temp = head;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = node;
    }

    CopyListToArray(head, teams, team_count);
    cout << "Team inserted successfully.\n";
}

void DeleteTeam(TeamNode*& head, Teams teams[], int& team_count) {
    string name;
    cout << "Enter team to delete: ";
    cin >> name;

    TeamNode* curr = head;
    TeamNode* prev = nullptr;

    while (curr) {
        if (curr->team.name == name) {
            if (!prev) {
                head = curr->next;
            }
            else {
                prev->next = curr->next;
            }
            delete curr;
            CopyListToArray(head, teams, team_count);
            cout << "Team deleted.\n";
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    cout << "Team not found.\n";
}

// Main Program Loop
int main() {
    srand(time(NULL));
    Teams league[MAX_TEAMS];
    int team_count = 0;
    MatchStack history;
    TeamNode* head = nullptr;

    loadTeams("names.txt", league, team_count);
    head = BuildLinkedList(league, team_count);

    int choice;
    do {
        cout << "\n--- Champions League Manager ---\n";
        cout << "1. Start New Season\n";
        cout << "2. Display Standings\n";
        cout << "3. Favorite Team To Show Match History\n";
        cout << "4. Insert Team\n";
        cout << "5. Delete Team\n";
        cout << "6. Exit\n";
        cout << "Choice: ";

        while (!(cin >> choice)) {
            cin.clear(); cin.ignore(1000, '\n');
        }

        switch (choice) {
        case 1:
            StartNewSeason(league, team_count, history);
            // Rebuild linked list after season to reflect sorting
            if (head) {
                while (head) {
                    TeamNode* t = head;
                    head = head->next;
                    delete t;
                }
            }
            head = BuildLinkedList(league, team_count);
            break;

        case 2: {
            TeamNode* temp = BuildLinkedList(league, team_count);
            DisplayStandings(temp);
            while (temp) {
                TeamNode* t = temp;
                temp = temp->next;
                delete t;
            }
            break;
        }

        case 3: {
            string favTeam;
            cout << "Enter your favorite team: ";
            cin >> favTeam;
            history.showFavoriteTeamHistory(favTeam);
            break;
        }

        case 4:
            InsertTeam(head, league, team_count);
            break;

        case 5:
            DeleteTeam(head, league, team_count);
            break;

        case 6:
            cout << "Exiting.\n";
            break;

        default:
            cout << "Invalid choice.\n";
        }

    } while (choice != 6);

    // Final Memory Cleanup
    while (head != nullptr) {
        TeamNode* temp = head;
        head = head->next;
        delete temp;
    }

    return 0;
}