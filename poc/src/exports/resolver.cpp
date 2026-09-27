#include <appdome.hpp>
#include <logcat.hpp>

std::pair<void*, size_t> get_or_map_object(const uintptr_t address, uintptr_t return_to);
size_t get_object_original_size(const uintptr_t address);

__attribute__((used, noinline, no_stack_protector))
uintptr_t EXPORT_RESOLVER(uintptr_t RELATIVE_EXPORT_ADDR, uintptr_t SELF_ADDR, uintptr_t LR, uintptr_t X86_RET_ADDR)
{
    log_D("0x%lx 0x%lx 0x%lx", RELATIVE_EXPORT_ADDR, SELF_ADDR, LR);

    const size_t obj_size = get_object_original_size(RELATIVE_EXPORT_ADDR);
    if (obj_size == 0)
    {
        // will never happen butttttttttttttttttt just in case, i love code with 1000 ifs with error catching
        log_E("failed to resolve object size for: 0x%lx", RELATIVE_EXPORT_ADDR);
        return 0;
    }

    // return back
    uintptr_t resume_at = RELATIVE_EXPORT_ADDR + obj_size - THUMB_BIT;
    log_D("resume at: 0x%lx", resume_at);

    uintptr_t return_to = resume_at + exports::g_app_base;
    auto [obj, _mapped_size] = get_or_map_object(RELATIVE_EXPORT_ADDR, return_to);
    log_D("mapped entry: 0x%lx, return_to: 0x%lx", reinterpret_cast<uintptr_t>(obj), return_to);

    // log_D("ok jumping to: 0x%lx", return_to);

    // asm_jump_back(return_to);
#if defined(__aarch64__)
    __asm__ volatile(
        "mov x16, %0\n" // intra procedure call regs !!!111!!!!!!s
        "mov x17, %1\n"
        :
        : "r"(obj), "r"(return_to)
        : "x16", "x17"
    );
#elif defined(__arm__)
    // theres only ip0 in 32
    // https://learn.microsoft.com/en-us/cpp/build/overview-of-arm-abi-conventions?view=msvc-170
    // r10	Non-volatile
    // r12	Volatile	Intra-procedure-call scratch register

    // r0 gets moved into r12 post call, then trampoline does bx r12.
    // return reinterpret_cast<uintptr_t>(obj) | THUMB_BIT;
#elif defined(__x86_64__)
    log_D("x86 ret adr 0x%lx", X86_RET_ADDR);
    log_D("lr+0 0x%lx lr+8: 0x%lx", *reinterpret_cast<uintptr_t*>(LR), *reinterpret_cast<uintptr_t*>(LR + 8));
    
    *reinterpret_cast<uintptr_t*>(LR) = 0;
    *reinterpret_cast<uintptr_t*>(LR + 8) = reinterpret_cast<uintptr_t>(EXPORT_RESOLVER);
    
    log_D("lr set");
#endif

    return return_to;
}
