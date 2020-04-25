#ifndef __NETTCP_UTILS_HPP__
#define __NETTCP_UTILS_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Library Headers
#include <Net/Tcp/Export.hpp>

// Stl Headers
#include <QtGlobal>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Tcp {

// ─────────────────────────────────────────────────────────────
//                  CLASS
// ─────────────────────────────────────────────────────────────

/**
 */
class NETTCP_API_ Utils
{
public:
    /**
     * Register type to the qml engines
     * Registered types are:
     * - AbstractServer
     * - Server
     * - SharedDatagram
     */
    static void registerTypes(const char* uri = nullptr, const quint8 major = 1,
        const quint8 minor = 0);
    static void loadResources();
};

}
}

#endif
