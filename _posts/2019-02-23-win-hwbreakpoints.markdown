---
layout: post
title: "Hardware breakpoints for Win64"
---

<a href="/downloads/hwbreakpoint.h" class="link_btn">hwbreakpoint.h</a>
<a href="/downloads/hwbreakpoint.cpp" class="link_btn">hwbreakpoint.cpp</a>

### Basic Usage

{% highlight C %}
#include "breakpoint.h"

// let's assume you have variable "val" that you want to watch on it
SomeNativeType val;

// to set write breakpoint
HWBreakpoint::Set(&val, HWBreakpoint::Condition::Write);

// to set read and write breakpoint
HWBreakpoint::Set(&val, HWBreakpoint::Condition::ReadWrite);

// to clear the breakpoint
HWBreakpoint::Clear(&val);

// to cleanup all breakpoint
HWBreakpoint::ClearAll();

// or for C
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


{% endhighlight %}
