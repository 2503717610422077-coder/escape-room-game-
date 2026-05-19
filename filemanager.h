#pragma once
/*
============================================================
  FINAL ESCAPE — FILE MANAGER MODULE
  filemanager.h
  -------------------------------------------------------
  MEMBER    : Member 2
  PURPOSE   : Declares all file read/write operations.

  FILES MANAGED:
    user.txt      →  NAME|SCORE           (login store)
    players.txt   →  NAME|SCORE|ROOM|PUZZLE|HEALTH
    game.json     →  generated for browser
    puzzle.txt    →  one puzzle question per line
    activity.txt  →  one activity question per line
    anspuzzle.txt →  one puzzle answer per line
    ansactivity.txt → one activity answer per line

  DEPENDENCIES: structs.h
============================================================
*/

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "structs.h"
#include <string>
#include <vector>
#include <map>

// ---- File paths (used across all modules) ----
extern const std::string FILE_PUZZLE_Q;
extern const std::string FILE_ACTIVITY_Q;
extern const std::string FILE_PUZZLE_ANS;
extern const std::string FILE_ACTIVITY_ANS;
extern const std::string FILE_USERS;
extern const std::string FILE_PLAYERS;
extern const std::string FILE_GAME_JSON;

// ---- Utility ----
std::string trim(const std::string& s);
std::string jsonEscape(const std::string& s);

// ---- Question file reader ----
std::vector<std::string> readLines(const std::string& filename);

// ---- user.txt (login store) ----
std::map<std::string, UserEntry>    loadUsers();
void                                saveUsers(const std::map<std::string, UserEntry>& users);

// ---- players.txt (full progress) ----
std::map<std::string, PlayerRecord> loadPlayers();
void                                savePlayers(const std::map<std::string, PlayerRecord>& players);

// ---- game.json (browser data bundle) ----
void writeGameJson(
    const std::vector<std::string>& puzzleQ,
    const std::vector<std::string>& activityQ,
    const std::vector<std::string>& puzzleAns,
    const std::vector<std::string>& activityAns
);

#endif // FILEMANAGER_H
