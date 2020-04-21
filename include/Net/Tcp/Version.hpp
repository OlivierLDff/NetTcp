#ifndef __NETTCP_COMMON_HPP__
#define __NETTCP_COMMON_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Library Headers
#include <Net/Tcp/Export.hpp>
#include <Net/Tcp/Property.hpp>

// Qt Headers
#include <QObject>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

namespace Net {
namespace Tcp {

class NETTCP_API_ Version : public QObject
{
    Q_OBJECT
    NETTCP_SINGLETON_IMPL(Version, version, Version);

    // ──────── CONSTRUCTOR ────────────────
public:
    Version(QObject* parent = nullptr);

    // ──────── ATTRIBUTES ────────────────
private:
    /** \brief Library Major Version */
    NETTCP_PROPERTY_CONST(quint32, major, Major);

    /** \brief Library Minor Version */
    NETTCP_PROPERTY_CONST(quint32, minor, Minor);

    /** \brief Library Patch Version */
    NETTCP_PROPERTY_CONST(quint32, patch, Patch);

    /** \brief Library Tag Version */
    NETTCP_PROPERTY_CONST(quint32, tag, Tag);

    /** \brief Library Version as major.minor.patch.tag */
    NETTCP_PROPERTY_CONST(QString, readable, Readable);
};

}
}

#endif
