---
layout: post
title: "Checked Numbers (with invariants)"
---

<a href="/downloads/checkedNumber.hpp" class="link_btn">checkedNumber.hpp</a>

### Basic Usage

{% highlight C++ %}
#include "checkedNumber.hpp"
int main() {
    PositiveNumber<double> x = 9;
    x--; // OK
    x -= 88; // x < 0 now, throws exception
    return 0;
}
{% endhighlight %}
