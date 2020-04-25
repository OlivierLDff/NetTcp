// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Library Headers
#include <Net/Tcp/Version.hpp>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

using namespace Net::Tcp;

// ─────────────────────────────────────────────────────────────
//                  FUNCTIONS
// ─────────────────────────────────────────────────────────────

Version::Version(QObject* parent) :
    QObject(parent), _major(NETTCP_VERSION_MAJOR), _minor(NETTCP_VERSION_MINOR),
    _patch(NETTCP_VERSION_PATCH), _tag(NETTCP_VERSION_TAG_HEX),
    _readable(QString::number(_major) + QStringLiteral(".") +
              QString::number(_minor) + QStringLiteral(".") +
              QString::number(_patch) + QStringLiteral(".0x") +
              QString::number(_tag, 16).rightJustified(8, QChar('0')))
{
}
