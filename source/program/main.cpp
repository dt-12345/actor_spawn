#include "engineActor.h"

#include "lib.hpp"
#include "utils.hpp"

using CreateFunc = bool (engine::actor::ActorMgr*, const sead::SafeString&, const engine::actor::ActorMgr::CreateArg&,
                          engine::actor::CreateWatcherRef*, engine::actor::Priority, engine::actor::PreActor*,
                          engine::actor::ActorFile*, sead::Function*, bool, engine::actor::ActorMgr::Result*, engine::actor::PreActor**);

engine::actor::ActorMgr** g_ActorMgrPtr = nullptr;
CreateFunc* requestCreateActorAsync = nullptr;

HOOK_DEFINE_INLINE(Whistle) {
    static void Callback(exl::hook::InlineCtx* ctx) {
        auto player = reinterpret_cast<engine::actor::ActorBase*>(ctx->X[0]);

        if (!player || !g_ActorMgrPtr || !(*g_ActorMgrPtr)) {
            char buf[0x20];
            PRINT("Necessary pointers are null")
        }

        bb::InitInfo<32> init_info;
        engine::actor::ActorMgr::CreateArg create_arg;
        create_arg.position = player->getPosition();
        create_arg._90 = 1;

        sead::SafeString actor_name{"Enemy_Lynel_Dark"};

        engine::actor::ActorMgr::Result result;

        requestCreateActorAsync(*g_ActorMgrPtr, actor_name, create_arg, nullptr, engine::actor::Priority::High, nullptr,
                                nullptr, nullptr, false, &result, nullptr);

        char buf[0x10];
        PRINT("Result: %d", result)
    }
};

extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();

    g_ActorMgrPtr = reinterpret_cast<engine::actor::ActorMgr**>(exl::util::modules::GetTargetOffset(0x04722920));
    requestCreateActorAsync = reinterpret_cast<CreateFunc*>(exl::util::modules::GetTargetOffset(0x00ab92cc));

    Whistle::InstallAtOffset(0x01d8fecc);
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}