#include "engineActor.h"
#include "engineEvent.h"
#include "phive.h"
#include "havok.h"

// #include <prim/seadFunction.h>
#include <prim/seadSafeString.h>
#include <swkbd/swkbd.h>

#include "lib.hpp"
#include "utils.hpp"

#include <cstdlib>

u64 main_offset;

using CreateFunc = bool (engine::actor::ActorMgr*, const sead::SafeString&, const engine::actor::ActorMgr::CreateArg&,
                          engine::actor::CreateWatcherRef*, engine::actor::CreatePriority, engine::actor::PreActor*,
                          engine::actor::ActorFile*, sead::Function*, bool, engine::actor::ActorMgr::Result*, engine::actor::PreActor**);
using RequestFunc = bool (engine::event::EventMgr*, const engine::event::EventRequestArg&);
using SetFunc = bool (engine::actor::ActorBaseLink*, engine::actor::ActorBase*, u8);
using ConvertFunc = int (u32*, char*, u32, u16*, int);
using GetKeyboardFunc = bool (sead::WBufferedSafeString*, int, nn::swkbd::Preset, nn::swkbd::KeyboardMode,
                                u32, nn::swkbd::InputFormMode);

engine::actor::ActorMgr** g_ActorMgrPtr = nullptr;
engine::event::EventMgr** g_EventMgrPtr = nullptr;
CreateFunc* requestCreateActorAsync = nullptr;
RequestFunc* requestEvent = nullptr;
SetFunc* setActorLink = nullptr;
DtorFunc* ActorLinkDtor = nullptr;
ConvertFunc* convertUtf16ToUtf8 = nullptr;
GetKeyboardFunc* getKeyboardInput = nullptr;

void GetKeyboardInput(sead::BufferedSafeString& string) {
    char16_t input_buffer[0x80];
    sead::WBufferedSafeString input{input_buffer, 0x80};

    getKeyboardInput(&input, 0x80, nn::swkbd::Preset::UserName, nn::swkbd::KeyboardMode::ModeLanguageSet2,
        static_cast<u32>(nn::swkbd::InvalidChar::AtMark) | static_cast<u32>(nn::swkbd::InvalidChar::Percent)
        | static_cast<u32>(nn::swkbd::InvalidChar::BackSlash), nn::swkbd::InputFormMode::OneLine);
    
    u32 out_size;
    convertUtf16ToUtf8(&out_size, string.getBuffer(), 0x80, reinterpret_cast<u16*>(input_buffer), 0x80);
}

HOOK_DEFINE_INLINE(Whistle) {
    static void Callback(exl::hook::InlineCtx* ctx) {
        auto player = reinterpret_cast<engine::actor::ActorBase*>(ctx->X[0]);

        if (!player || !g_ActorMgrPtr || !(*g_ActorMgrPtr)) {
            char buf[0x20];
            PRINT("Necessary pointers are null")
            return;
        }

        bb::InitInfo<32> init_info;
        // init_info.setParam(sead::SafeString{"EquipmentUser_Bow"}, sead::SafeString{"Weapon_Bow_032"});
        // init_info.setParam(sead::SafeString{"EquipmentUser_Shield"}, sead::SafeString{"Weapon_Shield_018"});
        // init_info.setParam(sead::SafeString{"EquipmentUser_Weapon"}, sead::SafeString{"Weapon_Sword_124"});
        engine::actor::ActorMgr::CreateArg create_arg;
        create_arg.position = player->getPosition();
        create_arg.scale = { 1.f, 1.f, 1.f };
        create_arg.blackboard_info = &init_info;
        create_arg.transform_flags.set(engine::actor::ActorMgr::CreateArg::TransformFlags::UsePosition);
        create_arg.transform_flags.set(engine::actor::ActorMgr::CreateArg::TransformFlags::UseScale);

        char actor_name_buf[0x80];
        sead::BufferedSafeString actor_name{actor_name_buf, 0x80};

        GetKeyboardInput(actor_name);

        const char* buffer = actor_name.getBuffer();
        if (buffer[0] == 'A' && buffer[1] == 'i' && buffer[2] == 'r' && buffer[3] == 'W' && buffer[4] == 'a' && buffer[5] == 'l' && buffer[6] == 'l') {
            create_arg.scale = { 210.f, 0.5f, 210.f };
            create_arg.position.y += 2.f;

            // hknpBodyId id = *exl::util::pointer_path::FollowSafe<hknpBodyId, 0x04728538, 0x1e8, 0x58, 0x68, 0x28, 0x388, 0x40, 0x228, 0x50, 0x20, 0x150, 0x70, 0x8>();
            hknpMotion* motions = *exl::util::pointer_path::FollowSafe<hknpMotion*, 0x0471dcb0, 0xc8, 0xc0, 0xe0, 0x150>();
            hknpBody* bodies = *exl::util::pointer_path::FollowSafe<hknpBody*, 0x0471dcb0, 0xc8, 0xc0, 0xe0, 0x38>();

            auto cmp = reinterpret_cast<engine::component::PhysicsComponent*>(player->getComponent(10));
            if (cmp && cmp->controllerSet && cmp->controllerSet->mainRigidBody) {
                auto rb = cmp->controllerSet->mainRigidBody;
                if (rb->sdkBody == nullptr) {
                    return;
                }
                auto body = bodies + (rb->sdkBody->bodyId & 0xffffff);
                body->m_transform.m_translation.y += 2.0f;
                if (body->m_motionId != 0xffffffff) {
                    motions[body->m_motionId].m_centerOfMass.y += 2.0;
                }
            }
        }

        char input_buf[0x80];
        sead::BufferedSafeString input{input_buf, 0x80};
        GetKeyboardInput(input);
        char valueBuffers[8][0x80];
        int count = 0;
        while (input != sead::SafeString{""} && count < 8) {
            sead::BufferedSafeString value{valueBuffers[count++], 0x80};
            GetKeyboardInput(value);
            sead::SafeString key{input.getBuffer()};
            init_info.setParam(input, sead::SafeString(value));
            memset(input.getBuffer(), 0, input.getBufferSize());
            GetKeyboardInput(input);
        }

        engine::actor::ActorMgr::Result result;

        bool res = requestCreateActorAsync(*g_ActorMgrPtr, actor_name, create_arg, nullptr, engine::actor::CreatePriority::High, nullptr,
                                            nullptr, nullptr, false, &result, nullptr);

        char buf[0x80];
        if (res) {
            PRINT("Created %s", actor_name.cstr())
        } else {
            PRINT("Failed to create %s: [Result:%d]", actor_name.cstr(), result)
        }
    }
};

HOOK_DEFINE_TRAMPOLINE(Impulse) {
    static float Callback(float impulse, float* thresholds) {
        float result = Orig(impulse, thresholds);

        char buf[0x20];
        if (thresholds[3] == 500000.f)
            PRINT("Zonai?: %f -> %f", impulse, result)
        else
            PRINT("Normal: %f -> %f", impulse, result)

        return result;
    }
};

HOOK_DEFINE_INLINE(ChargeAttack) {
    static void Callback(exl::hook::InlineCtx* ctx) {
        auto player = reinterpret_cast<engine::actor::ActorBase*>(ctx->X[0]);

        if (!player || !g_EventMgrPtr || !(*g_EventMgrPtr)) {
            char buf[0x20];
            PRINT("Necessary pointers are null")
            return;
        }

        engine::event::EventRequestArg request_arg{"DmF_SY_Camp"};

        // third arg is some link reference type mask, usually is 0 but notably is 5 for new item get messages
        setActorLink(&request_arg.event_starter_link, player, 0);

        bool res = requestEvent(*g_EventMgrPtr, request_arg);

        char buf[0x80];
        if (res) {
            PRINT("Requested %s", request_arg.event_name.cstr())
        } else {
            PRINT("Failed to request %s", request_arg.event_name.cstr())
        }
    }
};

extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();

    g_ActorMgrPtr = reinterpret_cast<engine::actor::ActorMgr**>(exl::util::modules::GetTargetOffset(0x04722920));
    g_EventMgrPtr = reinterpret_cast<engine::event::EventMgr**>(exl::util::modules::GetTargetOffset(0x047258c8));
    requestCreateActorAsync = reinterpret_cast<CreateFunc*>(exl::util::modules::GetTargetOffset(0x00ab92cc));
    requestEvent = reinterpret_cast<RequestFunc*>(exl::util::modules::GetTargetOffset(0x011f0700));
    setActorLink = reinterpret_cast<SetFunc*>(exl::util::modules::GetTargetOffset(0x00b7cb84));
    ActorLinkDtor = reinterpret_cast<DtorFunc*>(exl::util::modules::GetTargetOffset(0x0076eb88));
    convertUtf16ToUtf8 = reinterpret_cast<ConvertFunc*>(exl::util::modules::GetTargetOffset(0x00ef6c30));
    getKeyboardInput = reinterpret_cast<GetKeyboardFunc*>(exl::util::modules::GetTargetOffset(0x01b2b3a0));

    Whistle::InstallAtOffset(0x01d8fecc);
    Impulse::InstallAtOffset(0x020bf5d4);

    main_offset = exl::util::modules::GetTargetStart();
    // SetPosition::InstallAtOffset(0x01d8fecc);
    // visitRigidBodyEntities = reinterpret_cast<VisitEntites*>(exl::util::modules::GetTargetOffset(0x008aa374));
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}