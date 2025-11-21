#include "pong_game_state.h"

#include <algorithm>

namespace {
    PongGameState g_state;
}

PongGameState& GetPongGameState() {
    return g_state;
}

void ResetPongGameState() {
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.playerPaddle = PaddleInfo{};
    g_state.aiPaddle = PaddleInfo{};
    g_state.scoreboardLabel = nullptr;
    g_state.fieldHalfWidth = 0.35;
    g_state.fieldHalfHeight = 0.25;
    g_state.ballRadius = 0.02;
    g_state.baseBallSpeed = 0.8;
    g_state.ballSpeed = g_state.baseBallSpeed;
    g_state.maxBallSpeed = 1.5;
    g_state.speedIncrement = 0.05;
    g_state.maxBounceAngleRad = chai3d::cDegToRad(50.0);
    g_state.resetDelay = 1.0;
    g_state.nextServeDirection = 1;
    g_state.waitingForServe = true;
    g_state.serveTimer = 0.0;
    g_state.playerScore = 0;
    g_state.aiScore = 0;
    g_state.scoreLimit = 0;
    g_state.useScoreLimit = false;
    g_state.ballVelocity = chai3d::cVector3d(0, 0, 0);
    g_state.ballPosition = chai3d::cVector3d(0, 0, 0);
}

