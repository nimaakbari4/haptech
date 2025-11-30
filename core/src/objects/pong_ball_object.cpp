#include "pong_ball_object.h"

#include <simulation_manager.h>

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

namespace {
    std::mt19937& RandomEngine() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }

    double parseDouble(const std::map<std::string, std::string>& params,
                       const std::string& key,
                       double defaultValue) {
        auto it = params.find(key);
        if (it == params.end()) {
            return defaultValue;
        }
        try {
            return std::stod(it->second);
        } catch (...) {
            return defaultValue;
        }
    }

    int parseInt(const std::map<std::string, std::string>& params,
                 const std::string& key,
                 int defaultValue) {
        auto it = params.find(key);
        if (it == params.end()) {
            return defaultValue;
        }
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }

    chai3d::cColorf parseColor(const std::map<std::string, std::string>& params,
                               const std::string& prefix,
                               const chai3d::cColorf& fallback) {
        auto rIt = params.find(prefix + "_r");
        auto gIt = params.find(prefix + "_g");
        auto bIt = params.find(prefix + "_b");
        if (rIt == params.end() || gIt == params.end() || bIt == params.end()) {
            return fallback;
        }
        try {
            return chai3d::cColorf(
                std::clamp(std::stod(rIt->second), 0.0, 1.0),
                std::clamp(std::stod(gIt->second), 0.0, 1.0),
                std::clamp(std::stod(bIt->second), 0.0, 1.0)
            );
        } catch (...) {
            return fallback;
        }
    }

    class PongScoreLimitCondition final : public TrialConditionInterface {
    public:
        PongScoreLimitCondition(bool success, const std::string& message)
            : TrialConditionInterface(dummyParams(), success, 100),
              message_(message) {}

        bool evaluate() override { return true; }
        std::string getMessage() override { return message_; }
        std::string getName() override { return "pong_score_limit"; }

    private:
        std::string message_;

        static std::map<std::string, std::string>& dummyParams() {
            static std::map<std::string, std::string> params;
            return params;
        }
    };
}

chai3d::cGenericObject* PongBallObject::createObject(const std::map<std::string, std::string>& params,
                                                     double x, double y, double z,
                                                     double r, double g, double b) {
    try {
        ResetPongGameState();

        radius_ = parseDouble(params, "radius", radius_);
        launchSpeed_ = parseDouble(params, "speed", launchSpeed_);
        resetDelay_ = parseDouble(params, "reset_delay", resetDelay_);
        fieldHalfWidth_ = parseDouble(params, "field_half_width", fieldHalfWidth_);
        fieldHalfHeight_ = parseDouble(params, "field_half_height", fieldHalfHeight_);
        speedIncrement_ = parseDouble(params, "speed_increment", speedIncrement_);
        maxSpeed_ = parseDouble(params, "max_speed", maxSpeed_);
        maxBounceAngleRad_ = chai3d::cDegToRad(parseDouble(params, "max_bounce_angle_deg", chai3d::cRadToDeg(maxBounceAngleRad_)));
        serveAngleOffsetDeg_ = parseDouble(params, "serve_angle_deg", serveAngleOffsetDeg_);
        nextServeDirection_ = parseInt(params, "initial_direction", 1);
        scoreLimit_ = parseInt(params, "score_limit", scoreLimit_);

        scoreboardX_ = parseInt(params, "scoreboard_x", scoreboardX_);
        scoreboardY_ = parseInt(params, "scoreboard_y", scoreboardY_);
        scoreboardFontSize_ = parseDouble(params, "scoreboard_font_size", scoreboardFontSize_);
        scoreboardColor_ = parseColor(params, "scoreboard_color", scoreboardColor_);

        chai3d::cMesh* ball = new chai3d::cMesh();
        chai3d::cCreateSphere(ball, radius_);
        ball->createAABBCollisionDetector(radius_ / 4.0);
        ball->m_material->m_diffuse.set(r, g, b);
        ball->m_material->m_ambient.set(r, g, b);
        ball->m_material->m_specular.set(r, g, b);
        ball->setLocalPos(x, y, z);

        ballNode_ = ball;

        auto& state = GetPongGameState();
        initializeState(state);
        prepareScoreboard(state);
        scheduleServe(state, nextServeDirection_);

        stateInitialized_ = true;

        std::cout << "[PongBall] Created Pong ball at (" << x << ", " << y << ", " << z
                  << ") radius=" << radius_ << " speed=" << launchSpeed_ << std::endl;

        return ball;
    } catch (const std::exception& e) {
        std::cerr << "[PongBall] Exception during createObject: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[PongBall] Unknown exception during createObject" << std::endl;
    }
    return nullptr;
}

void PongBallObject::initializeState(PongGameState& state) {
    std::lock_guard<std::mutex> lock(state.mutex);
    state.ballRadius = radius_;
    state.baseBallSpeed = launchSpeed_;
    state.ballSpeed = launchSpeed_;
    state.maxBallSpeed = maxSpeed_;
    state.speedIncrement = speedIncrement_;
    state.maxBounceAngleRad = maxBounceAngleRad_;
    state.resetDelay = resetDelay_;
    state.fieldHalfWidth = fieldHalfWidth_;
    state.fieldHalfHeight = fieldHalfHeight_;
    state.nextServeDirection = nextServeDirection_;
    state.waitingForServe = true;
    state.serveTimer = resetDelay_;
    state.ballVelocity = chai3d::cVector3d(0, 0, 0);
    state.ballPosition = chai3d::cVector3d(0, 0, 10.0);
    state.playerScore = 0;
    state.aiScore = 0;
    state.scoreLimit = scoreLimit_;
    state.useScoreLimit = scoreLimit_ > 0;
}

void PongBallObject::prepareScoreboard(PongGameState& state) {
    std::lock_guard<std::mutex> lock(state.mutex);

    if (state.scoreboardLabel == nullptr) {
        SimulationManager::addTextOverlay("Player 0 : 0 CPU",
                                          scoreboardX_,
                                          scoreboardY_,
                                          scoreboardFontSize_,
                                          scoreboardColor_.getR(),
                                          scoreboardColor_.getG(),
                                          scoreboardColor_.getB());
        if (!SimulationManager::simContext.customTextOverlays.empty()) {
            state.scoreboardLabel = SimulationManager::simContext.customTextOverlays.back();
        }
    }
}

void PongBallObject::scheduleServe(PongGameState& state, int direction) {
    std::lock_guard<std::mutex> lock(state.mutex);
    state.nextServeDirection = (direction >= 0) ? 1 : -1;
    state.waitingForServe = true;
    state.serveTimer = resetDelay_;
    state.ballVelocity = chai3d::cVector3d(0, 0, 0);
    state.ballPosition = chai3d::cVector3d(0, 0, 10.0);
    state.ballSpeed = state.baseBallSpeed;
    if (ballNode_) {
        ballNode_->setLocalPos(state.ballPosition);
    }
    updateScoreboardText(state);
}

void PongBallObject::launchBall(PongGameState& state) {
    std::uniform_real_distribution<double> dist(-serveAngleOffsetDeg_, serveAngleOffsetDeg_);
    double angleDeg = dist(RandomEngine());
    double angleRad = chai3d::cDegToRad(angleDeg);

    double speed = state.ballSpeed;
    double dirX = std::cos(angleRad);
    double dirY = std::sin(angleRad);

    chai3d::cVector3d velocity(dirX * speed * state.nextServeDirection,
                               dirY * speed,
                               0.0);

    state.ballVelocity = velocity;
    state.waitingForServe = false;
    state.ballPosition = chai3d::cVector3d(0, 0, 10.0);
    state.ballSpeed = speed;
}

void PongBallObject::updateScoreboardText(PongGameState& state) {
    if (!state.scoreboardLabel) {
        return;
    }

    std::ostringstream oss;
    oss << "Player " << state.playerScore << " : " << state.aiScore << " CPU";
    state.scoreboardLabel->setText(oss.str());
}

void PongBallObject::handleWallBounce(PongGameState& state, chai3d::cVector3d& pos) {
    double limitY = state.fieldHalfHeight - state.ballRadius;
    if (pos.y() > limitY) {
        pos.y(limitY);
        state.ballVelocity.y(-std::abs(state.ballVelocity.y()));
    } else if (pos.y() < -limitY) {
        pos.y(-limitY);
        state.ballVelocity.y(std::abs(state.ballVelocity.y()));
    }
}

bool PongBallObject::checkPaddleCollision(const PaddleInfo& paddleInfo,
                                          bool isPlayer,
                                          PongGameState& state,
                                          chai3d::cVector3d& pos) {
    if (!paddleInfo.node) {
        return false;
    }

    const auto paddlePos = paddleInfo.node->getLocalPos();
    double paddleMinY = paddlePos.y() - paddleInfo.halfHeight - state.ballRadius;
    double paddleMaxY = paddlePos.y() + paddleInfo.halfHeight + state.ballRadius;

    double paddlePlaneX = paddlePos.x();
    double paddleHalfWidth = paddleInfo.halfWidth + state.ballRadius;
    
    // Check Z coordinate - ball and paddle must be at same Z level (within tolerance)
    double zTolerance = 0.1; // Allow 10cm tolerance in Z
    if (std::abs(pos.z() - paddlePos.z()) > zTolerance) {
        return false;
    }

    if (pos.y() < paddleMinY || pos.y() > paddleMaxY) {
        return false;
    }

    if (isPlayer) {
        if (pos.x() < paddlePlaneX) {
            return false;
        }
        if (pos.x() <= paddlePlaneX + paddleHalfWidth) {
            pos.x(paddlePlaneX + paddleHalfWidth);
        } else {
            return false;
        }
    } else {
        if (pos.x() > paddlePlaneX) {
            return false;
        }
        if (pos.x() >= paddlePlaneX - paddleHalfWidth) {
            pos.x(paddlePlaneX - paddleHalfWidth);
        } else {
            return false;
        }
    }

    double offset = (pos.y() - paddlePos.y()) / std::max(0.0001, paddleInfo.halfHeight);
    offset = std::clamp(offset, -1.0, 1.0);
    double bounceAngle = offset * state.maxBounceAngleRad;

    // Use current speed for this bounce
    double speed = state.ballSpeed;

    double dirX = std::cos(bounceAngle);
    double dirY = std::sin(bounceAngle);

    state.ballVelocity.x((isPlayer ? 1.0 : -1.0) * std::abs(dirX) * speed);
    state.ballVelocity.y(dirY * speed);
    state.ballVelocity.z(0.0);

    // Increment speed for next bounce (capped at max)
    state.ballSpeed = std::min(state.ballSpeed + state.speedIncrement, state.maxBallSpeed);

    // Apply haptic impulse for player paddle contact
    if (isPlayer) {
        // Calculate force direction based on bounce angle and ball speed
        // Force pushes outward from the paddle (negative X for player paddle)
        double forceMagnitude = std::min(3.5, speed * 15.0);  // Scale factor to convert speed to force (N)
        double fx = forceMagnitude * std::abs(dirX);  // Outward push (positive X)
        double fy = forceMagnitude * dirY;             // Y component based on angle
        double fz = 0.0;

        cVector3d force(fx, fy, fz);
        force.normalize();
        force *= forceMagnitude;
        
        int impulseDuration = 50;  // milliseconds
        
        std::cout << "[PongBall] Player paddle contact at Y offset: " << offset 
                  << " | Bounce angle: " << chai3d::cRadToDeg(bounceAngle) << " deg"
                  << " | Speed: " << speed
                  << " | Applying impulse: (" << force.x() << ", " << force.y() << ", " << force.z() << ") (" << forceMagnitude << "N) for " << impulseDuration << "ms" << std::endl;
        
        SimulationManager::applyConstantImpulse(force.x(), force.y(), force.z(), impulseDuration);
    }

    return true;
}

void PongBallObject::handlePaddleCollisions(PongGameState& state, chai3d::cVector3d& pos) {
    bool collided = false;
    collided |= checkPaddleCollision(state.playerPaddle, true, state, pos);
    collided |= checkPaddleCollision(state.aiPaddle, false, state, pos);

    if (collided) {
        // Slightly increase speed on paddle hit already handled within collision
    }
}

bool PongBallObject::checkScoreLimit(PongGameState& state, bool playerScored) {
    if (!state.useScoreLimit) {
        return false;
    }

    int currentScore = playerScored ? state.playerScore : state.aiScore;
    if (currentScore < state.scoreLimit) {
        return false;
    }

    bool success = playerScored;
    std::ostringstream oss;
    oss << (playerScored ? "Player" : "CPU")
        << " reached score " << currentScore
        << " (limit " << state.scoreLimit << ")";

    if (!SimulationManager::trialContext.endConditionMet) {
        SimulationManager::trialContext.endConditionMet =
            std::make_unique<PongScoreLimitCondition>(success, oss.str());
    }

    state.ballVelocity = chai3d::cVector3d(0, 0, 0);
    state.waitingForServe = false;
    return true;
}

void PongBallObject::updatePhysics(chai3d::cGenericObject* object, double deltaTime) {
    if (!stateInitialized_ || !object) {
        return;
    }

    auto& state = GetPongGameState();
    std::lock_guard<std::mutex> lock(state.mutex);

    if (state.waitingForServe) {
        state.serveTimer -= deltaTime;
        if (state.serveTimer <= 0.0) {
            launchBall(state);
        } else {
            state.ballPosition = chai3d::cVector3d(0, 0, 10.0);
            object->setLocalPos(state.ballPosition);
            return;
        }
    }

    chai3d::cVector3d pos = state.ballPosition;
    pos += state.ballVelocity * deltaTime;

    handleWallBounce(state, pos);
    handlePaddleCollisions(state, pos);
    
    // Ensure ball stays at z=10 (2D pong game)
    pos.z(10.0);

    double goalLimit = state.fieldHalfWidth + radius_;
    if (pos.x() > goalLimit) {
        state.playerScore += 1;
        updateScoreboardText(state);
        state.ballPosition = chai3d::cVector3d(0, 0, 10.0);
        state.ballSpeed = state.baseBallSpeed;  // Reset speed on goal
        if (checkScoreLimit(state, true)) {
            object->setLocalPos(state.ballPosition);
            return;
        }
        {
            std::uniform_real_distribution<double> offsetDist(-state.fieldHalfHeight * 0.8, state.fieldHalfHeight * 0.8);
            double targetY = offsetDist(RandomEngine());
            double dirX = state.baseBallSpeed;
            double dirY = (targetY - state.ballPosition.y()) * (state.baseBallSpeed / std::max(0.001, state.fieldHalfWidth));
            state.ballVelocity = chai3d::cVector3d(dirX, dirY, 0.0);
        }
        object->setLocalPos(state.ballPosition);
        state.serveTimer = state.resetDelay;
        state.waitingForServe = true;
        return;
    } else if (pos.x() < -goalLimit) {
        state.aiScore += 1;
        updateScoreboardText(state);
        state.ballPosition = chai3d::cVector3d(0, 0, 10.0);
        state.ballSpeed = state.baseBallSpeed;  // Reset speed on goal
        if (checkScoreLimit(state, false)) {
            object->setLocalPos(state.ballPosition);
            return;
        }
        {
            std::uniform_real_distribution<double> offsetDist(-state.fieldHalfHeight * 0.8, state.fieldHalfHeight * 0.8);
            double targetY = offsetDist(RandomEngine());
            double dirX = -state.baseBallSpeed;
            double dirY = (targetY - state.ballPosition.y()) * (state.baseBallSpeed / std::max(0.001, state.fieldHalfWidth));
            state.ballVelocity = chai3d::cVector3d(dirX, dirY, 0.0);
        }
        object->setLocalPos(state.ballPosition);
        state.serveTimer = state.resetDelay;
        state.waitingForServe = true;
        return;
    }

    state.ballPosition = pos;
    object->setLocalPos(pos);
}

