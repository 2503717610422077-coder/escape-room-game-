/*
============================================================
  FINAL ESCAPE — GAME LOGIC MODULE
  gamelogic.cpp
  -------------------------------------------------------
  MEMBER    : Member 3
  PURPOSE   : Implements the command dispatcher.
              Each command mirrors a C++ function call that
              the browser (via Node bridge) sends over stdin.

  YOUR RESPONSIBILITIES:
    1. LOAD_GAME    — login check via user.txt, restore progress
    2. SAVE_PROGRESS— persist score/room/puzzle/health
    3. GET_QUESTIONS— trigger game.json rebuild
    4. EXIT_GAME    — final save before player quits
    5. LEADERBOARD  — print ranked scores from user.txt

  HOW TO COMPILE (together with other modules):
    g++ -std=c++17 -c gamelogic.cpp -o gamelogic.o
============================================================
*/

#include "gamelogic.h"
#include "filemanager.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

// ============================================================
//  HANDLE COMMAND
//  Reads the first token as the command name, then dispatches
//  to the correct block. Called once per stdin line in main.
// ============================================================

void handleCommand(
    const string&                   line,
    map<string, PlayerRecord>&      players,
    map<string, UserEntry>&         users,
    const vector<string>&           puzzleQ,
    const vector<string>&           activityQ,
    const vector<string>&           puzzleAns,
    const vector<string>&           activityAns
) {
    istringstream iss(line);
    string cmd;
    iss >> cmd;

    // ==========================================================
    //  LOAD_GAME <playerName>
    //
    //  Flow:
    //    1. Read remaining input as the player name.
    //    2. Check user.txt map — if found, it's a returning player.
    //    3. Load their full progress from players.txt map.
    //    4. If NOT found, register them in both user.txt and players.txt.
    //    5. Output a PLAYER_DATA JSON line for the browser to parse.
    // ==========================================================
    if (cmd == "LOAD_GAME") {

        string name;
        getline(iss, name);
        name = trim(name);

        if (name.empty()) {
            cout << "{\"error\":\"No name provided\"}\n";
            return;
        }

        // Lowercase key for case-insensitive lookup
        string key = name;
        transform(key.begin(), key.end(), key.begin(), ::tolower);

        PlayerRecord rec;
        bool isReturning = false;

        if (users.count(key)) {
            // ---- Returning player ----
            isReturning = true;

            if (players.count(key)) {
                rec = players[key];
                // user.txt score is authoritative (catches mid-session crashes)
                rec.score = users[key].score;
            } else {
                // user.txt entry exists but players.txt row is missing — rebuild defaults
                rec.name          = users[key].name;
                rec.score         = users[key].score;
                rec.currentRoom   = 0;
                rec.currentPuzzle = 0;
                rec.health        = 100;
                players[key]      = rec;
                savePlayers(players);
                cout << "[GAMELOGIC] players.txt row rebuilt for: " << name << "\n";
            }

            cout << "[GAMELOGIC] Login OK (user.txt): " << rec.name
                 << "  Score="   << rec.score
                 << "  Room="    << rec.currentRoom
                 << "  Puzzle="  << rec.currentPuzzle
                 << "  Health="  << rec.health << "\n";

        } else {
            // ---- New player: register in both files ----
            rec.name          = name;
            rec.score         = 0;
            rec.currentRoom   = 0;
            rec.currentPuzzle = 0;
            rec.health        = 100;
            isReturning       = false;

            users[key] = { name, 0 };
            saveUsers(users);

            players[key] = rec;
            savePlayers(players);

            cout << "[GAMELOGIC] New user registered: " << name << "\n";
        }

        // Output compact JSON for the browser
        cout << "PLAYER_DATA {"
             << "\"name\":\""        << jsonEscape(rec.name) << "\","
             << "\"score\":"         << rec.score             << ","
             << "\"currentRoom\":"   << rec.currentRoom       << ","
             << "\"currentPuzzle\":" << rec.currentPuzzle     << ","
             << "\"health\":"        << rec.health            << ","
             << "\"isReturning\":"   << (isReturning ? "true" : "false")
             << "}\n";
    }

    // ==========================================================
    //  SAVE_PROGRESS <name> <score> <room> <puzzle> <health>
    //
    //  Writes full progress to players.txt AND syncs the score
    //  into user.txt so the login store is always up to date.
    //  Also called after every wrong answer (score deduction).
    // ==========================================================
    else if (cmd == "SAVE_PROGRESS") {

        string name;
        int score, room, puzzle, health;
        iss >> name >> score >> room >> puzzle >> health;
        name = trim(name);

        if (name.empty()) {
            cout << "SAVE_ERROR No name provided\n";
            return;
        }

        string key = name;
        transform(key.begin(), key.end(), key.begin(), ::tolower);

        // Update full progress record
        players[key] = { name, score, room, puzzle, health };
        savePlayers(players);

        // Sync score to user.txt login store
        users[key] = { name, score };
        saveUsers(users);

        cout << "[GAMELOGIC] Progress saved: " << name
             << " | Score="  << score
             << " | Room="   << room
             << " | Puzzle=" << puzzle
             << " | Health=" << health << "\n";

        cout << "SAVE_OK\n";
    }

    // ==========================================================
    //  GET_QUESTIONS
    //  Regenerates game.json from the in-memory question arrays.
    //  Browser calls this if it needs a fresh copy of questions.
    // ==========================================================
    else if (cmd == "GET_QUESTIONS") {
        writeGameJson(puzzleQ, activityQ, puzzleAns, activityAns);
        cout << "QUESTIONS_READY " << FILE_GAME_JSON << "\n";
    }

    // ==========================================================
    //  EXIT_GAME <name> <score> <room> <puzzle> <health>
    //  Same as SAVE_PROGRESS but signals the browser that the
    //  player has intentionally left. Saves to both files.
    // ==========================================================
    else if (cmd == "EXIT_GAME") {

        string name;
        int score, room, puzzle, health;
        iss >> name >> score >> room >> puzzle >> health;
        name = trim(name);

        if (!name.empty()) {
            string key = name;
            transform(key.begin(), key.end(), key.begin(), ::tolower);

            players[key] = { name, score, room, puzzle, health };
            savePlayers(players);

            users[key] = { name, score };
            saveUsers(users);

            cout << "[GAMELOGIC] Exit save complete for: " << name << "\n";
        }

        cout << "EXIT_OK\n";
    }

    // ==========================================================
    //  LEADERBOARD
    //  Reads scores from user.txt (the authoritative score store)
    //  and prints them sorted highest-first.
    // ==========================================================
    else if (cmd == "LEADERBOARD") {

        cout << "\n======== LEADERBOARD (from user.txt) ========\n";

        // Collect all UserEntry values into a sortable vector
        vector<UserEntry> all;
        for (auto& p : users) all.push_back(p.second);

        sort(all.begin(), all.end(),
            [](const UserEntry& a, const UserEntry& b){
                return a.score > b.score;
            });

        if (all.empty()) {
            cout << "  (no players yet)\n";
        } else {
            int rank = 1;
            for (auto& u : all) {
                cout << "  " << rank++ << ". "
                     << u.name << "  —  Score: " << u.score << "\n";
            }
        }

        cout << "=============================================\n";
    }

    // ==========================================================
    //  Unknown command — warn but don't crash
    // ==========================================================
    else {
        cout << "[GAMELOGIC WARN] Unknown command: " << cmd << "\n";
    }
}
