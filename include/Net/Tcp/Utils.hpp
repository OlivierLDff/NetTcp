#ifndef __NETTCP_UTILS_HPP__
#define __NETTCP_UTILS_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Stl Headers
#include <QtGlobal>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace net {
namespace tcp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

void registerQmlTypes(const char* uri = nullptr, const quint8 major = 1, const quint8 minor = 0);
void loadQmlResources();

}
}

#endif
