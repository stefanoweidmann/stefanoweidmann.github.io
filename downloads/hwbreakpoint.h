#pragma once

#ifdef __cplusplus
namespace HWBreakpoint
{
    enum class Condition
    {
        Write = 1,
        ReadWrite = 3
    };

    bool Set(void* address, Condition when);
    void Clear(void* address);
    void ClearAll();
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HWBRK_WRITE = 1,
    HWBRK_READ_WRITE = 3
} HWBRK_Condition;

int HWBRK_Set(
    void* address,
    HWBRK_Condition when
);

void HWBRK_Clear(
    void* address
);

void HWBRK_ClearAll(
);

#ifdef __cplusplus
}
#endif
