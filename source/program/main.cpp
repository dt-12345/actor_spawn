#include "engineActor.h"
#include "engineEvent.h"

#include <prim/seadSafeString.h>
#include <swkbd/swkbd.h>

#include "lib.hpp"
#include "utils.hpp"

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

HOOK_DEFINE_INLINE(Whistle) {
    static void Callback(exl::hook::InlineCtx* ctx) {
        auto player = reinterpret_cast<engine::actor::ActorBase*>(ctx->X[0]);

        if (!player || !g_ActorMgrPtr || !(*g_ActorMgrPtr)) {
            char buf[0x20];
            PRINT("Necessary pointers are null")
            return;
        }

        bb::InitInfo<32> init_info;
        init_info.setParam(sead::SafeString{"EquipmentUser_Bow"}, sead::SafeString{"Weapon_Bow_032"});
        init_info.setParam(sead::SafeString{"EquipmentUser_Shield"}, sead::SafeString{"Weapon_Shield_018"});
        init_info.setParam(sead::SafeString{"EquipmentUser_Weapon"}, sead::SafeString{"Weapon_Sword_124"});
        engine::actor::ActorMgr::CreateArg create_arg;
        create_arg.position = player->getPosition();
        create_arg.scale = { 1.f, 1.f, 1.f };
        create_arg.blackboard_info = &init_info;
        create_arg.transform_flags.set(engine::actor::ActorMgr::CreateArg::TransformFlags::UsePosition);
        create_arg.transform_flags.set(engine::actor::ActorMgr::CreateArg::TransformFlags::UseScale);

        char16_t input_buffer[0x80];
        char name_buffer[0x80];

        sead::WBufferedSafeString input{input_buffer, 0x80};
        sead::BufferedSafeString actor_name{name_buffer, 0x80};

        getKeyboardInput(&input, 0x80, nn::swkbd::Preset::UserName, nn::swkbd::KeyboardMode::ModeLanguageSet2,
                            static_cast<u32>(nn::swkbd::InvalidChar::AtMark) | static_cast<u32>(nn::swkbd::InvalidChar::Percent)
                            | static_cast<u32>(nn::swkbd::InvalidChar::BackSlash), nn::swkbd::InputFormMode::OneLine);

        u32 out_size;
        convertUtf16ToUtf8(&out_size, name_buffer, 0x80, reinterpret_cast<u16*>(input_buffer), 0x80);

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
    ChargeAttack::InstallAtOffset(0x01d53a94);
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}