#ifdef SAUSAGE_PROFILE_ENABLE
#define IF_PROFILE_ENABLED(...)
#else
#define IF_PROFILE_ENABLED(...) #__VA_ARGS__
#endif

#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
#define IF_SAUSAGE_DEBUG_DRAW_PHYSICS(...) #__VA_ARGS__
#else
#define IF_SAUSAGE_DEBUG_DRAW_PHYSICS(...)
#endif

#ifndef NDEBUG
#define DEBUG_ASSERT(...) assert(#__VA_ARGS__)
#else
#define DEBUG_ASSERT(...)
#endif

#ifndef NDEBUG
#define DEBUG_EXPR(...) assert(#__VA_ARGS__)
#else
#define DEBUG_EXPR(...)
#endif
