---
layout: post
title: "Random Number Generation with xoroshiro.h"
---

For generating random numbers, I like to use the [Xoroshiro128+](http://xoshiro.di.unimi.it) generator, which is fast and generates high quality output.
It's especially nice to use it in multithreaded applications, because you can initialize a distinct generator for each thread with the same seed for all.
Altough it has done a good job in all my uses, consider using another generator for very long simulations or simulations with many threads.
Here some words of caution by the creators
> They [xoroshiro128+ and another variant called xoroshiro128**] are suitable only for low-scale parallel applications;
 moreover, xoroshiro128+ exhibits a mild dependency in Hamming weights that generates a failure after 5 TB of output in our test.
 We believe this slight bias cannot affect any application.

For my usage and yours, I have wrapped the basic algorithm into a header only library [`xoroshiro.h`](/downloads/xoroshiro.h)
 for C99 and C++.

<a href="/downloads/xoroshiro.h" class="link_btn">xoroshiro.h</a>

### Basic Usage

{% highlight C %}
#include <xoroshiro.h>
#define NUM_THREADS 2
const uint64_t seed = 42;
Xoroshiro_State_t rng[NUM_THREADS];
// initialize one generator per thread

for (int threadNo = 0; threadNo < NUM_THREADS; ++threadNo)
{
    rng[threadNo] = Xoroshiro_GetInitialState(seed, threadNo);
}

const bool ok = Xoroshiro_GetBool(&rng[0]);
const uint64_t uintNum = Xoroshiro_GetNextNumber(&rng[1]);
// 0 <= dbl < 1
const double dbl = Xoroshiro_GenerateRealInHalfOpenInterval(0, 1, &rng[0]);
{% endhighlight %}

### Advanced Usage

Included is also a Random number engine compatible with C++11 for use with [`<random>`](https://en.cppreference.com/w/cpp/header/random).
It is designed to be a drop in replacement for std::mt19937_64. Technically, it fulfills the concept
[`UniformRandomBitGenerator`](https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator).

{% highlight C++ %}
#include <random>
#include <xoroshiro.h>
// in thread number 9 (the 10th thread)
XoroshiroRandomNumberEngine rng(42 /* seed */, 8 /* number of this thread, >= 0, optional */);
std::uniform_int_distribution<int> diceDistribution(1,6);
const int diceRoll = diceDistribution(rng);
{% endhighlight %}

If you like to use the [GNU scientific library (GSL)](https://www.gnu.org/software/gsl/), as I do,
define `XOROSHIRO_GSL` before including `xoroshiro.h` and get a new RNG to use with it
{% highlight C %}

//////////////////////////////////////
// C99
//////////////////////////////////////
#define XOROSHIRO_GSL
#include <xoroshiro.h> // also includes gsl_rng.h now
#include <stdlib.h>
gsl_rng* rng = gsl_rng_alloc(Xoroshiro_GSL_rng);

// different thread numbers not supported
// gsl_rng_set(rng, 42 /* seed */);

// If different thread numbers are required, use this uglier variant,
const Xoroshiro_State_t xoroshiroState = Xoroshiro_GetInitialState(42, 3 /* thread no 3 */);
memcpy(rng->state, xoroshiroState, sizeof(Xoroshiro_State_t));

// generates real in [0, 1[
const double real = gsl_rng_uniform(rng);
gsl_rng_free(rng);

////////////////////////////////
// C++11
////////////////////////////////
#define XOROSHIRO_GSL
#include <xoroshiro.h> // also includes gsl_rng.h now
XoroshiroRandomNumberEngine xRng(42 /* seed */, /* thread number */ 3);
const gsl_rng* rngForGSL = xRng.getGslRngPtr();

// generates real in [0, 1[
const double real = gsl_rng_uniform(rngForGSL);

{% endhighlight %}
