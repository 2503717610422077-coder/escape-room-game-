#pragma once
/*
============================================================
  FINAL ESCAPE — GAME LOGIC MODULE
  gamelogic.h
  -------------------------------------------------------
  MEMBER    : Member 3
  PURPOSE   : Declares the command dispatcher that processes
              all game commands sent from the browser/main.

  COMMANDS HANDLED:
    LOAD_GAME    <name>
    SAVE_PROGRESS <name> <score> <room> <puzzle> <health>
    GET_QUESTIONS
    EXIT_GAME    <name> <score> <room> <puzzle> <health>
    LEADERBOARD

  DEPENDENCIES: structs.h, filemanager.h
============================================================
*/

#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "structs.h"
#include <string>
#include <vector>
#include <map>

// ---- Main command dispatcher ----
// Called by main.cpp for every line received from stdin.
void handleCommand(
    const std::string&                        line,
    std::map<std::string, PlayerRecord>&      players,
    std::map<std::string, UserEntry>&         users,
    const std::vector<std::string>&           puzzleQ,
    const std::vector<std::string>&           activityQ,
    const std::vector<std::string>&           puzzleAns,
    const std::vector<std::string>&           activityAns
);

#endif // GAMELOGIC_H
