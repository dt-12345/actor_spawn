#pragma once

#include <container/seadBuffer.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace bb {

enum class ParamType : u32 {
    Invalid,
    String,
    Int,
    F32,
    Bool,
    Vec3f,
    VoidPtr,
    S8,
    S16,
    S64,
    U8,
    U16,
    U32,
    U64,
    F64,
    Vec3i,
    Clr4f,
    Mtx33f,
    Mtx34f,
    Quatf,
    CustomType,
};

struct Param {
    // they might've not used a raw union but whatever
    union Value {
        sead::SafeString str;
        s32 s_int;
        f32 float_32;
        bool boolean;
        sead::Vector3f vec3f;
        void* void_ptr;
        s8 s_byte;
        s16 s_short;
        s64 s_long;
        u8 u_byte;
        u16 u_short;
        u32 u_int;
        u64 u_long;
        f64 float_64;
        sead::Color4f clr;
        sead::Matrix33f mtx33f;
        sead::Matrix34f mtx34f;
    };

    Param() : type{ParamType::Invalid}, value{0} {}

    ParamType type;
    sead::FixedSafeString<128> key;
    Value value;
};
static_assert(sizeof(Param) == 0xc8);

template <int count>
struct InitInfo {
    sead::Buffer<Param> params;
    s32 param_count;
    Param buffer[count];

    InitInfo() : param_count{count} {}
};
static_assert(sizeof(InitInfo<32>) == 0x1918);

} // namespace bb