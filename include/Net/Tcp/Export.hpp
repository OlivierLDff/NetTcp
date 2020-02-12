#ifndef __NETTCP_EXPORT_HPP__
#define __NETTCP_EXPORT_HPP__

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#ifdef WIN32
    #ifdef NETTCP_DLL_EXPORT  // Shared build
        #define NETTCP_API_ __declspec(dllexport)
    #elif NETTCP_STATIC       // No decoration when building staticlly
        #define NETTCP_API_
    #else                       // Link to lib
        #define NETTCP_API_ __declspec(dllimport)
    #endif
#else
    #define NETTCP_API_
#endif // WIN32

#endif // __NETTCP_EXPORT_HPP__
