#ifndef __NETTCP_PCH_HPP__
#define __NETTCP_PCH_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

// Stl
#include <cstdint>
#include <memory>
#include <set>

// spdlog
#include <spdlog/logger.h>
#include <spdlog/sinks/sink.h>

// QOlm
#include <QOlm/QOlm.hpp>

// Qt Core Headers
#include <QtGlobal>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>

// Qt Qml Headers
#include <QQmlEngine>

// Qt Network Headers
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

#endif
