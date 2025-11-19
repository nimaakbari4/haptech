#ifndef PONG_BALL_OBJECT_H
#define PONG_BALL_OBJECT_H

#include "custom_object_interface.h"
#include "pong_game_state.h"

class PongBallObject : public CustomObjectInterface {
public:
    PongBallObject() = default;
    ~PongBallObject() override = default;

    chai3d::cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                         double x, double y, double z,
                                         double r, double g, double b) override;

    void updatePhysics(chai3d::cGenericObject* object, double deltaTime) override;

    std::string getTypeName() const override { return "pong_ball"; }

private:
    chai3d::cGenericObject* ballNode_ = nullptr;
    double radius_ = 0.03;
    double launchSpeed_ = 0.8;
    double resetDelay_ = 1.0;
    double fieldHalfWidth_ = 0.35;
    double fieldHalfHeight_ = 0.25;
    double speedIncrement_ = 0.05;
    double maxSpeed_ = 1.5;
    double maxBounceAngleRad_ = chai3d::cDegToRad(50.0);
    double serveAngleOffsetDeg_ = 15.0;

    int nextServeDirection_ = 1;
    bool stateInitialized_ = false;

    int scoreboardX_ = 20;
    int scoreboardY_ = 20;
    double scoreboardFontSize_ = 32.0;
    chai3d::cColorf scoreboardColor_ = chai3d::cColorf(1.0, 1.0, 1.0);

    void initializeState(PongGameState& state);
    void prepareScoreboard(PongGameState& state);
    void scheduleServe(PongGameState& state, int direction);
    void launchBall(PongGameState& state);
    void updateScoreboardText(PongGameState& state);
    void handleWallBounce(PongGameState& state, chai3d::cVector3d& pos);
    void handlePaddleCollisions(PongGameState& state, chai3d::cVector3d& pos);
    bool checkPaddleCollision(const PaddleInfo& paddleInfo,
                              bool isPlayer,
                              PongGameState& state,
                              chai3d::cVector3d& pos);
};

#endif // PONG_BALL_OBJECT_H

