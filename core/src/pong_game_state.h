#ifndef PONG_GAME_STATE_H
#define PONG_GAME_STATE_H

#include <mutex>
#include <chai3d.h>

struct PaddleInfo {
    chai3d::cGenericObject* node = nullptr;
    double halfHeight = 0.1;
    double halfWidth = 0.01;
    double halfDepth = 0.01;
    double clampMinY = -0.25;
    double clampMaxY = 0.25;
    double baseX = 0.0;
    double baseZ = 0.0;
    double aiSpeed = 0.6;
    double playerScaleY = 1.0;
    double playerOffsetY = 0.0;
    double smoothing = 12.0;
};

struct PongGameState {
    std::mutex mutex;
    PaddleInfo playerPaddle;
    PaddleInfo aiPaddle;
    chai3d::cLabel* scoreboardLabel = nullptr;

    double fieldHalfWidth = 0.35;
    double fieldHalfHeight = 0.25;
    double ballRadius = 0.02;
    double baseBallSpeed = 0.8;
    double ballSpeed = 0.8;
    double maxBallSpeed = 1.5;
    double speedIncrement = 0.05;
    double maxBounceAngleRad = chai3d::cDegToRad(50.0);
    double resetDelay = 1.0;

    int nextServeDirection = 1; // 1 -> towards AI (positive X), -1 -> towards player
    bool waitingForServe = true;
    double serveTimer = 0.0;
    int playerScore = 0;
    int aiScore = 0;

    chai3d::cVector3d ballVelocity = chai3d::cVector3d(0, 0, 0);
    chai3d::cVector3d ballPosition = chai3d::cVector3d(0, 0, 0);
};

PongGameState& GetPongGameState();
void ResetPongGameState();

#endif // PONG_GAME_STATE_H

