#ifndef XOROSHIRO_H
#define XOROSHIRO_H

#include <stdint.h>
#include <assert.h>

#if (!defined(__cplusplus) && !defined(__STDC_VERSION__)) \
    || (defined( __STDC_VERSION__) && __STDC_VERSION__ < 199901L)
#error "This sourcecode requires C++ or C99"
#endif

/**************************************************************************************************
 *                          THE XOROSHIRO 128 + RANDOM NUMBER GENERATOR                           *
 **************************************************************************************************/

// define macro to force inlining
#if defined(_MSC_VER)
#define XOROSHIRO_FORCE_INLINE __forceinline
#elif defined(__GNUC__)
#define XOROSHIRO_FORCE_INLINE __attribute__((always_inline))
#else
#define XOROSHIRO_FORCE_INLINE
#endif

/*
 * Shamelessly stolen from http://xoroshiro.di.unimi.it/xoroshiro128plus.c (creative commons C0)
 * All credit is due to Sebastiano Vigna and David Blackman
 *
 */

/* This is xoroshiro128+ 1.0, our best and fastest small-state generator
   for floating-point numbers. We suggest to use its upper bits for
   floating-point generation, as it is slightly faster than
   xoroshiro128**. It passes all tests we are aware of except for the four
   lower bits, which might fail linearity tests (and just those), so if
   low linear complexity is not considered an issue (as it is usually the
   case) it can be used to generate 64-bit outputs, too; moreover, this
   generator has a very mild Hamming-weight dependency making our test
   (http://prng.di.unimi.it/hwd.php) fail after 5 TB of output; we believe
   this slight bias cannot affect any application. If you are concerned,
   use xoroshiro128** or xoshiro256+.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s.

   NOTE: the parameters (a=24, b=16, b=37) of this version give slightly
   better results in our test than the 2016 version (a=55, b=14, c=36).
*/


const static uint64_t Xoroshiro_MinimumGeneratedNumber = 0;
const static uint64_t Xoroshiro_MaximumGeneratedNumber = UINT64_MAX;


typedef struct Xoroshiro_State_Struct {
    uint64_t lower;
    uint64_t upper;
} Xoroshiro_State_t;

static inline XOROSHIRO_FORCE_INLINE
uint64_t
Xoroshiro_RotateLeft(
    const uint64_t x,
    int k
) {
    return (x << k) | (x >> (64 - k));
}

// Returns a random uint64_t
static inline XOROSHIRO_FORCE_INLINE
uint64_t
Xoroshiro_GetNextNumber(
    Xoroshiro_State_t* state
) {
    const uint64_t s0 = state->lower;
    uint64_t s1 = state->upper;
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    state->lower = Xoroshiro_RotateLeft(s0, 24) ^ s1 ^ (s1 << 16); // a, b
    state->upper = Xoroshiro_RotateLeft(s1, 37); // c

    return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */

static inline
void
Xoroshiro_JumpNumbers(
    Xoroshiro_State_t* state
) {
    static const uint64_t JUMP[] = {
        UINT64_C(0xdf900294d8f554a5),
        UINT64_C(0x170865df4b3201fc)
    };

    uint64_t newLowerState = 0;
    uint64_t newUpperState = 0;
    for (int i = 0; i < (int)(sizeof(JUMP) / sizeof(*JUMP)); i++)
        for (int b = 0; b < 64; b++) {
            if (JUMP[i] & UINT64_C(1) << b) {
                newLowerState ^= state->lower;
                newUpperState ^= state->upper;
            }
            Xoroshiro_GetNextNumber(state);
        }

    state->lower = newLowerState;
    state->upper = newUpperState;
}

static inline
uint64_t
Xoroshiro_SplitMix64(
    uint64_t* x
) {
    uint64_t z = (*x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

static inline
Xoroshiro_State_t
Xoroshiro_GetInitialState(
    uint64_t seed,
    const unsigned threadNo
) {
    Xoroshiro_State_t initialState;
    initialState.lower = seed;
    initialState.upper = Xoroshiro_SplitMix64(&seed);

    // jump to my section; inefficient if threadNo is high.
    // Then we would need to distribute the initial state
    for (unsigned j = 0; j < threadNo; ++j) {
        Xoroshiro_JumpNumbers(&initialState);
    }

    return initialState;
}

static inline
double
Xoroshiro_GenerateRealInHalfOpenInterval(
    const double closedLowerBound,
    const double openUpperBound,
    Xoroshiro_State_t* state
) {
    const uint64_t randomInteger = Xoroshiro_GetNextNumber(state);

    double randomReal;
    do {
        const double intervalLength = openUpperBound - closedLowerBound;
        randomReal = closedLowerBound + intervalLength *
                                        (double) (randomInteger - Xoroshiro_MinimumGeneratedNumber) /
                                        (double) (Xoroshiro_MaximumGeneratedNumber - Xoroshiro_MinimumGeneratedNumber);
    } while (randomReal >= openUpperBound);
    return randomReal;
}

static inline
#if defined(__cplusplus)
bool
#else
_Bool
#endif
Xoroshiro_GenerateBool(
    Xoroshiro_State_t* state
) {
    const uint64_t randomInteger = Xoroshiro_GetNextNumber(state);
    // use most significant bit as it's supposed to be higher quality
    return ((int64_t) randomInteger) < 0;
}

/**************************************************************************************************
 *                                     XOROSHIRO AS A GSL RNG                                     *
 **************************************************************************************************/

#ifdef XOROSHIRO_GSL
#include <gsl/gsl_rng.h>

static inline
void
Xoroshiro_GSL_Set(
    void* state,
    unsigned long seed
) {
    *((Xoroshiro_State_t*) state) = Xoroshiro_GetInitialState(seed, 0);
}

static inline
unsigned long
Xoroshiro_GSL_Get(
    void* state
) {
    return (unsigned long) Xoroshiro_GetNextNumber((Xoroshiro_State_t*) state);
}

static inline
double
Xoroshiro_GSL_GetDouble(
    void* state
) {
    return Xoroshiro_GenerateRealInHalfOpenInterval(0.0, 1.0, (Xoroshiro_State_t*) state);
}

static const gsl_rng_type Xoroshiro_GSL_rng_type = {
    "Xoroshiro", /*name*/
    Xoroshiro_MinimumGeneratedNumber, /*min*/
    Xoroshiro_MaximumGeneratedNumber, /*max*/
    sizeof(Xoroshiro_State_t), /*size*/
    Xoroshiro_GSL_Set, /*set*/
    Xoroshiro_GSL_Get, /*get*/
    Xoroshiro_GSL_GetDouble, /*get_double*/
};

static const gsl_rng_type* Xoroshiro_GSL_rng = &Xoroshiro_GSL_rng_type;

#endif // end of defined(XOROSHIRO_GSL)

/**************************************************************************************************
 *             ADAPTATION OF THE XOROSHIRO RNG AS C++ 11 RANDOM NUMBER ENGINE CONCEPT             *
 **************************************************************************************************/
#if defined(__cplusplus) && __cplusplus >= 201103L

#include <stdexcept>
// satisfies the concept UniformRandomBitGenerator
class XoroshiroRandomNumberEngine {

    public:
        // named with underscore for compatibility with other RNGs in <random>
        using result_type = uint64_t;
        const static result_type default_seed = 42;


        inline
        XoroshiroRandomNumberEngine(
        ) {
            seed(default_seed);
#ifdef XOROSHIRO_GSL
            gsl_rng_ptr_M = nullptr;
#endif
        }

        inline
        XoroshiroRandomNumberEngine(
            const uint64_t seedToUse,
            const unsigned threadNo = 0
        ) {
            seed(seedToUse, threadNo);
#ifdef XOROSHIRO_GSL
            gsl_rng_ptr_M = nullptr;
#endif
        }

        inline
        XoroshiroRandomNumberEngine(
            const XoroshiroRandomNumberEngine& other
        ): state_M(other.state_M), seedUsed_M(other.seedUsed_M) {
#ifdef XOROSHIRO_GSL
            gsl_rng_ptr_M = nullptr;
#endif
        }

        inline
        XoroshiroRandomNumberEngine&
        operator=(
            const XoroshiroRandomNumberEngine& other
        ) {
            state_M = other.state_M;
            seedUsed_M = other.seedUsed_M;
#ifdef XOROSHIRO_GSL
            gsl_rng_ptr_M = nullptr;
#endif
            return *this;
        }

        inline
        XoroshiroRandomNumberEngine&
        operator=(
            XoroshiroRandomNumberEngine&& other
        ) {
            state_M = std::move(other.state_M);
            seedUsed_M = std::move(other.seedUsed_M);
#ifdef XOROSHIRO_GSL
            other.maybeFreeGSLRNG();
#endif
            return *this;
        }

        inline
        XoroshiroRandomNumberEngine(
            XoroshiroRandomNumberEngine&& other
        ): state_M(std::move(other.state_M)), seedUsed_M(std::move(other.seedUsed_M)) {
#ifdef XOROSHIRO_GSL
            other.maybeFreeGSLRNG();
#endif
        }

        inline
        ~XoroshiroRandomNumberEngine() {
#ifdef XOROSHIRO_GSL
            maybeFreeGSLRNG();
#endif
        }

        inline
        void
        seed(
            uint64_t seedToUse,
            const unsigned threadNo = 0
        ) {
            seedUsed_M = seedToUse;
            state_M = Xoroshiro_GetInitialState(seedToUse, threadNo);
        }


        inline
        result_type
        getSeed(
        ) const {
            return seedUsed_M;
        }


        inline
        Xoroshiro_State_t
        getState(
        ) const {
            return state_M;
        }


        inline static constexpr
        result_type
        min(
        ) {
            return Xoroshiro_MinimumGeneratedNumber;
        }


        inline static constexpr
        result_type
        max(
        ) {
            return Xoroshiro_MaximumGeneratedNumber;
        }


        inline
        result_type
        operator()(
        ) {
            return Xoroshiro_GetNextNumber(&state_M);
        }


#ifdef XOROSHIRO_GSL
        inline
        const gsl_rng* getGslRngPtr() {
            // contains a ptr to state, that's why we use const
            // but still have the gsl function mutate the state.
            // returning a ptr to const because we want to
            // prohibit the caller to free it by themselves
            maybeAllocGSLRNG();
            return gsl_rng_ptr_M;
        }
#endif

    private:
        Xoroshiro_State_t state_M;
        result_type seedUsed_M;

#ifdef XOROSHIRO_GSL
        gsl_rng* gsl_rng_ptr_M;

        inline
        void
        maybeAllocGSLRNG() {
            if (gsl_rng_ptr_M != nullptr) {
                return;
            }
            gsl_rng_ptr_M = gsl_rng_alloc(Xoroshiro_GSL_rng);
            if (gsl_rng_ptr_M == nullptr) {
                throw std::runtime_error("Could not allocate Xoroshiro as a GSL RNG!");
            }
            gsl_rng_ptr_M->state = static_cast<void*>(&state_M);
            assert(gsl_rng_ptr_M->state != nullptr);
        }

        inline
        void
        maybeFreeGSLRNG() {
            if (gsl_rng_ptr_M != nullptr) {
                // HACK to prevent GSL from freeing the state
                gsl_rng_ptr_M->state = nullptr;
                gsl_rng_free(gsl_rng_ptr_M);
            }
            gsl_rng_ptr_M = nullptr;
        }
#endif

};

#endif /* end of ifdef __cplusplus */
#endif /* end of include guard */