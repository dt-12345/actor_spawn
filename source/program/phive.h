#pragma once

#include "engineActor.h"

namespace phive {

struct CharacterParameterCore {
    
};

struct CharacterProperty {

};

struct CharacterController {

};

struct SdkRigidBody {
    char _00[0x8];
    u64 bodyId;
};

struct RigidBody {
    char _00[0x70];
    SdkRigidBody* sdkBody;
};

struct ControllerSet {
    char _00[0x150];
    RigidBody* mainRigidBody;
};

} // namespace phive

namespace engine::component {

struct PhysicsComponent {
    char _00[0x20];
    phive::ControllerSet* controllerSet;
};

} // namespace engine::component