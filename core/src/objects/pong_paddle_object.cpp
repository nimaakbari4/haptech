#include "pong_paddle_object.h"

#include <simulation_manager.h>

#include <algorithm>
#include <iostream>

namespace {
    double clampValue(double v, double minV, double maxV) {
        return std::max(minV, std::min(maxV, v));
    }
}

double PongPaddleObject::parseDouble(const std::map<std::string, std::string>& params,
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

std::string PongPaddleObject::parseString(const std::map<std::string, std::string>& params,
                                          const std::string& key,
                                          const std::string& defaultValue) {
    auto it = params.find(key);
    if (it == params.end()) {
        return defaultValue;
    }
    return it->second;
}

chai3d::cGenericObject* PongPaddleObject::createObject(const std::map<std::string, std::string>& params,
                                                       double x, double y, double z,
                                                       double r, double g, double b) {
    try {
        double width = parseDouble(params, "width", halfWidth_ * 2.0);
        double height = parseDouble(params, "height", halfHeight_ * 2.0);
        double depth = parseDouble(params, "depth", halfDepth_ * 2.0);

        halfWidth_ = std::max(0.001, width * 0.5);
        halfHeight_ = std::max(0.01, height * 0.5);
        halfDepth_ = std::max(0.001, depth * 0.5);

        clampMinY_ = parseDouble(params, "clamp_min_y", -0.25);
        clampMaxY_ = parseDouble(params, "clamp_max_y", 0.25);
        baseX_ = x;
        baseZ_ = z;

        aiSpeed_ = parseDouble(params, "ai_speed", aiSpeed_);
        playerScaleY_ = parseDouble(params, "player_axis_scale", playerScaleY_);
        playerOffsetY_ = parseDouble(params, "player_axis_offset", playerOffsetY_);
        smoothing_ = parseDouble(params, "smoothing", smoothing_);

        std::string role = parseString(params, "role", "player");
        if (role == "ai" || role == "cpu") {
            controlMode_ = ControlMode::AI;
        } else {
            controlMode_ = ControlMode::PLAYER;
        }

        chai3d::cMesh* paddle = new chai3d::cMesh();
        chai3d::cCreateBox(paddle, width, height, depth);
        paddle->createAABBCollisionDetector(std::max({halfWidth_, halfHeight_, halfDepth_}) / 2.0);
        paddle->m_material->m_diffuse.set(r, g, b);
        paddle->m_material->m_ambient.set(r, g, b);
        paddle->m_material->m_specular.set(r, g, b);
        paddle->setLocalPos(x, y, z);

        paddleNode_ = paddle;

        auto& state = GetPongGameState();
        registerWithState(state);

        std::cout << "[PongPaddle] Created paddle (" << role << ") at (" << x << ", "
                  << y << ", " << z << ") size=(" << width << ", "
                  << height << ", " << depth << ")" << std::endl;

        return paddle;
    } catch (const std::exception& e) {
        std::cerr << "[PongPaddle] Exception during createObject: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[PongPaddle] Unknown exception during createObject" << std::endl;
    }
    return nullptr;
}

void PongPaddleObject::registerWithState(PongGameState& state) {
    std::lock_guard<std::mutex> lock(state.mutex);
    PaddleInfo info;
    info.node = paddleNode_;
    info.halfHeight = halfHeight_;
    info.halfWidth = halfWidth_;
    info.halfDepth = halfDepth_;
    info.clampMinY = clampMinY_;
    info.clampMaxY = clampMaxY_;
    info.baseX = baseX_;
    info.baseZ = baseZ_;
    info.aiSpeed = aiSpeed_;
    info.playerScaleY = playerScaleY_;
    info.playerOffsetY = playerOffsetY_;
    info.smoothing = smoothing_;

    if (controlMode_ == ControlMode::PLAYER) {
        state.playerPaddle = info;
    } else {
        state.aiPaddle = info;
    }
}

void PongPaddleObject::updatePlayerControlled(PongGameState& state, double deltaTime) {
    if (!paddleNode_) {
        return;
    }

    if (!SimulationManager::simContext.tool) {
        return;
    }

    auto devicePos = SimulationManager::simContext.tool->getDeviceGlobalPos();
    double targetY = -devicePos.y() * playerScaleY_ + playerOffsetY_;
    targetY = clampValue(targetY, clampMinY_, clampMaxY_);

    auto currentPos = paddleNode_->getLocalPos();
    double lerpFactor = std::clamp(deltaTime * smoothing_, 0.0, 1.0);
    double newY = currentPos.y() + (targetY - currentPos.y()) * lerpFactor;
    newY = clampValue(newY, clampMinY_, clampMaxY_);

    paddleNode_->setLocalPos(baseX_, newY, baseZ_);

    state.playerPaddle.node = paddleNode_;
    state.playerPaddle.halfHeight = halfHeight_;
    state.playerPaddle.halfWidth = halfWidth_;
    state.playerPaddle.clampMinY = clampMinY_;
    state.playerPaddle.clampMaxY = clampMaxY_;
}

void PongPaddleObject::updateAIControlled(PongGameState& state, double deltaTime) {
    if (!paddleNode_) {
        return;
    }

    cVector3d ballPos = state.ballPosition;
    cVector3d paddlePos = paddleNode_->getLocalPos();
    double distance = cDistance(ballPos, paddlePos);
    if (distance < 0.01) {
        return;
    }

    double targetY = ballPos.y();
    double speed = state.ballSpeed;
    targetY = paddlePos.y() + (ballPos.y() - paddlePos.y()) * deltaTime * speed;
    targetY = clampValue(targetY, clampMinY_, clampMaxY_);
    paddleNode_->setLocalPos(baseX_, targetY, baseZ_);
    
    // Update state with current AI paddle info for collision detection
    state.aiPaddle.node = paddleNode_;
    state.aiPaddle.halfHeight = halfHeight_;
    state.aiPaddle.halfWidth = halfWidth_;
    state.aiPaddle.clampMinY = clampMinY_;
    state.aiPaddle.clampMaxY = clampMaxY_;
}

void PongPaddleObject::updatePhysics(chai3d::cGenericObject* object, double deltaTime) {
    if (!object) {
        return;
    }

    auto& state = GetPongGameState();
    std::lock_guard<std::mutex> lock(state.mutex);

    if (controlMode_ == ControlMode::PLAYER) {
        updatePlayerControlled(state, deltaTime);
    } else {
        updateAIControlled(state, deltaTime);
    }
}

