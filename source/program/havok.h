#pragma once

#include <basis/seadTypes.h>

struct hkQuaternionf {
    float x, y, z, w;
};

struct hkDouble3 {
    f64 x, y, z;
};

struct hkFloat3Pair {
    float m_secX, m_secY, m_secZ, m_firstX, m_firstY, m_firstZ;
};

struct hkVector4 {
    float x, y, z, w;
};

struct hkRotationf {
    hkVector4 m_col0, m_col1, m_col2;
};

struct hknpPreciseTransform {
    hkRotationf m_rotation;
    hkVector4 m_translation;
    hkVector4 m_translationAdjustment;
};

struct hkAabbFloat3 {
    hkFloat3Pair m_minMax;
};

struct hknpMotion {
    hkQuaternionf m_orientation;
    hkDouble3 m_centerOfMass;
    hkFloat3Pair m_previousStepLinearAndAngularVelocity;
    u16 m_inverseInertia[4];
    u32 m_firstAttachedBodyIndex;
    u32 m_solverId;
    u8 m_padding[6];
    u16 m_integrationFactor;
    u16 m_motionPropertiesId;
    u16 m_lookAheadDistance;
    u16 m_maxRotationPerStep;
    u8 m_cellIndex;
    u8 m_spaceSplitterWeight;
    hkVector4 m_linearVelocityAndSpeedLimit;
    hkVector4 m_angularVelocityLocalAndSpeedLimit;
};
static_assert(sizeof(hknpMotion) == 0x80);

struct hknpBodyIndex {
    u32 m_serialAndIndex;
};

struct hknpBodyId {
    u64 m_value;
};

struct hknpBody {
    hknpPreciseTransform m_transform;
    hknpBodyIndex m_index;
    int m_flags;
    u16 m_collisionControl;
    u16 m_timAngle;
    u16 m_maxTimDistance;
    u16 m_maxTimDistanceFromRotation;
    hkAabbFloat3 m_aabb;
    void* m_shape;
    u32 m_motionId;
    hknpBodyIndex m_nextAttachedBodyIndex;
    u8 m_addedToWorldFlags;
    unsigned char m_qualityId;
    u16 m_materialId;
    u32 m_collisionFilterInfo;
    u16 m_maxContactDistance;
    u16 m_maxContactDistanceFromRotation;
    u16 m_radiusOfComCenteredBoundingSphere;
    u16 m_maxDepenetrationSpeed;
    u8 m_activationPriority;
    u8 m_padding0[7];
    hkQuaternionf m_bodyFromMotionRotation;
    hknpBodyId m_id;
    u64 m_userData;
};
static_assert(sizeof(hknpBody) == 0xc0);