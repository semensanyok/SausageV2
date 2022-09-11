#ifdef SAUSAGE_PROFILE_ENABLE
#define IF_PROFILE_ENABLED(...) __VA_ARGS__
#else
#define IF_PROFILE_ENABLED(...)
#endif

#ifndef NDEBUG
#define DEBUG_ASSERT(...) assert(__VA_ARGS__)
#else
#define DEBUG_ASSERT(...)
#endif

#ifndef NDEBUG
#define DEBUG_EXPR(...) __VA_ARGS__
#else
#define DEBUG_EXPR(...)
#endif
