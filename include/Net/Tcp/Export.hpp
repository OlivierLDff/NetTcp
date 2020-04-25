#ifndef __NETTCP_EXPORT_HPP__
#define __NETTCP_EXPORT_HPP__

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#ifdef WIN32
#    ifdef NETTCP_DLL_EXPORT
#        define NETTCP_API_ __declspec(dllexport)
#    elif NETTCP_STATIC
#        define NETTCP_API_
#    else
#        define NETTCP_API_ __declspec(dllimport)
#    endif
#else
#    define NETTCP_API_
#endif

#endif
