#ifndef PONG_PADDLE_OBJECT_H
#define PONG_PADDLE_OBJECT_H

#include "custom_object_interface.h"
#include "pong_game_state.h"

class PongPaddleObject : public CustomObjectInterface {
public:
    PongPaddleObject() = default;
    ~PongPaddleObject() override = default;

    chai3d::cGenericObject* createObject(const std::map<std::string, std::string>& params,
                                         double x, double y, double z,
                                         double r, double g, double b) override;

    void updatePhysics(chai3d::cGenericObject* object, double deltaTime) override;

    std::string getTypeName() const override { return "pong_paddle"; }

private:
    enum class ControlMode {
        PLAYER,
        AI
    };

    chai3d::cGenericObject* paddleNode_ = nullptr;
    ControlMode controlMode_ = ControlMode::PLAYER;
    double halfHeight_ = 0.1;
    double halfWidth_ = 0.01;
    double halfDepth_ = 0.02;
    double clampMinY_ = -0.25;
    double clampMaxY_ = 0.25;
    double baseX_ = 0.0;
    double baseZ_ = 0.0;
    double aiSpeed_ = 0.6;
    double playerScaleY_ = 1.2;
    double playerOffsetY_ = 0.0;
    double smoothing_ = 12.0;

    static double parseDouble(const std::map<std::string, std::string>& params,
                              const std::string& key,
                              double defaultValue);
    static std::string parseString(const std::map<std::string, std::string>& params,
                                   const std::string& key,
                                   const std::string& defaultValue);
    void registerWithState(PongGameState& state);
    void updatePlayerControlled(PongGameState& state, double deltaTime);
    void updateAIControlled(PongGameState& state, double deltaTime);
};

#endif // PONG_PADDLE_OBJECT_H

