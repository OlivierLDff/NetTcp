#ifndef __NETTCP_PROPERTY_HPP__
#define __NETTCP_PROPERTY_HPP__

// ─────────────────────────────────────────────────────────────
//                  INCLUDE
// ─────────────────────────────────────────────────────────────

#include <QQmlEngine>

// ─────────────────────────────────────────────────────────────
//                  DECLARATION
// ─────────────────────────────────────────────────────────────

#define _NETTCP_Q_PROPERTY(type, attribute, Attribute)                                                                 \
protected:                                                                                                             \
    Q_PROPERTY(type attribute READ attribute WRITE set##Attribute RESET reset##Attribute NOTIFY attribute##Changed)

#define _NETTCP_Q_PROPERTY_RO(type, attribute, Attribute)                                                              \
protected:                                                                                                             \
    Q_PROPERTY(type attribute READ attribute NOTIFY attribute##Changed)

#define _NETTCP_Q_PROPERTY_CONST(type, attribute)                                                                      \
protected:                                                                                                             \
    Q_PROPERTY(type attribute READ attribute CONSTANT)

#define _NETTCP_PROPERTY_MEMBER(type, attribute, def)                                                                  \
private:                                                                                                               \
    type _##attribute = def;

#define _NETTCP_PROPERTY_GETTER(type, attribute, override)                                                             \
public:                                                                                                                \
    type attribute() const override { return _##attribute; }

#define _NETTCP_PROPERTY_GETTER_ABSTRACT(type, attribute)                                                              \
public:                                                                                                                \
    virtual type attribute() const = 0;

#define _NETTCP_PROPERTY_SETTER(type, attribute, Attribute, override)                                                  \
public:                                                                                                                \
    virtual bool set##Attribute(const type& value) override                                                            \
    {                                                                                                                  \
        if(value != _##attribute)                                                                                      \
        {                                                                                                              \
            _##attribute = value;                                                                                      \
            Q_EMIT attribute##Changed(value);                                                                          \
            return true;                                                                                               \
        }                                                                                                              \
        return false;                                                                                                  \
    }

#define _NETTCP_ATTRIBUTE_SETTER(type, attribute, Attribute, override)                                                 \
public:                                                                                                                \
    virtual bool set##Attribute(const type& value) override                                                            \
    {                                                                                                                  \
        if(value != _##attribute)                                                                                      \
        {                                                                                                              \
            _##attribute = value;                                                                                      \
            return true;                                                                                               \
        }                                                                                                              \
        return false;                                                                                                  \
    }

#define _NETTCP_PROPERTY_SETTER_ABSTRACT(type, Attribute)                                                              \
public:                                                                                                                \
    virtual bool set##Attribute(const type& value) = 0;

#define _NETTCP_PROPERTY_RESET(type, Attribute, def)                                                                   \
public:                                                                                                                \
    void reset##Attribute() { set##Attribute(def); }

#define _NETTCP_PROPERTY_SIGNAL(type, attribute)                                                                       \
Q_SIGNALS:                                                                                                             \
    void attribute##Changed(type attribute);

#define _NETTCP_ABSTRACT_PROPERTY_SHARED(type, attribute, Attribute, def)                                              \
    _NETTCP_PROPERTY_GETTER_ABSTRACT(type, attribute)                                                                  \
    _NETTCP_PROPERTY_SETTER_ABSTRACT(type, Attribute)                                                                  \
    _NETTCP_PROPERTY_RESET(type, Attribute, def)                                                                       \
    _NETTCP_PROPERTY_SIGNAL(type, attribute)

#define NETTCP_ABSTRACT_PROPERTY_D(type, attribute, Attribute, def)                                                    \
    _NETTCP_Q_PROPERTY(type, attribute, Attribute)                                                                     \
    _NETTCP_ABSTRACT_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETTCP_ABSTRACT_PROPERTY(type, attribute, Attribute) NETTCP_ABSTRACT_PROPERTY_D(type, attribute, Attribute, {})

#define NETTCP_ABSTRACT_PROPERTY_RO_D(type, attribute, Attribute, def)                                                 \
    _NETTCP_Q_PROPERTY_RO(type, attribute, Attribute)                                                                  \
    _NETTCP_ABSTRACT_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETTCP_ABSTRACT_PROPERTY_RO(type, attribute, Attribute)                                                        \
    NETTCP_ABSTRACT_PROPERTY_RO_D(type, attribute, Attribute, {})

#define NETTCP_IMPL_PROPERTY_D(type, attribute, Attribute, def)                                                        \
    _NETTCP_PROPERTY_MEMBER(type, attribute, def)                                                                      \
    _NETTCP_PROPERTY_GETTER(type, attribute, override)                                                                 \
    _NETTCP_PROPERTY_SETTER(type, attribute, Attribute, override)

#define NETTCP_IMPL_PROPERTY(type, attribute, Attribute) NETTCP_IMPL_PROPERTY_D(type, attribute, Attribute, {})

#define _NETTCP_PROPERTY_SHARED(type, attribute, Attribute, def)                                                       \
    _NETTCP_PROPERTY_MEMBER(type, attribute, def)                                                                      \
    _NETTCP_PROPERTY_GETTER(type, attribute, )                                                                         \
    _NETTCP_PROPERTY_SETTER(type, attribute, Attribute, )                                                              \
    _NETTCP_PROPERTY_RESET(type, Attribute, def)                                                                       \
    _NETTCP_PROPERTY_SIGNAL(type, attribute)

#define NETTCP_PROPERTY_D(type, attribute, Attribute, def)                                                             \
    _NETTCP_Q_PROPERTY(type, attribute, Attribute)                                                                     \
    _NETTCP_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETTCP_PROPERTY(type, attribute, Attribute) NETTCP_PROPERTY_D(type, attribute, Attribute, {})

#define NETTCP_PROPERTY_RO_D(type, attribute, Attribute, def)                                                          \
    _NETTCP_Q_PROPERTY_RO(type, attribute, Attribute)                                                                  \
    _NETTCP_PROPERTY_SHARED(type, attribute, Attribute, def)

#define NETTCP_PROPERTY_RO(type, attribute, Attribute) NETTCP_PROPERTY_RO_D(type, attribute, Attribute, {})

#define _NETTCP_ATTRIBUTE_SHARED(type, attribute, Attribute, def)                                                      \
    _NETTCP_PROPERTY_MEMBER(type, attribute, def)                                                                      \
    _NETTCP_PROPERTY_GETTER(type, attribute, )                                                                         \
    _NETTCP_ATTRIBUTE_SETTER(type, attribute, Attribute, )

#define NETTCP_PROPERTY_CONST_D(type, attribute, Attribute, def)                                                       \
    _NETTCP_Q_PROPERTY_CONST(type, attribute)                                                                          \
    _NETTCP_ATTRIBUTE_SHARED(type, attribute, Attribute, def)

#define NETTCP_PROPERTY_CONST(type, attribute, Attribute) NETTCP_PROPERTY_CONST_D(type, attribute, Attribute, {})

#define NETTCP_ATTRIBUTE_D(type, attribute, Attribute, def) _NETTCP_ATTRIBUTE_SHARED(type, attribute, Attribute, def)

#define NETTCP_ATTRIBUTE(type, attribute, Attribute) NETTCP_ATTRIBUTE_D(type, attribute, Attribute, {})

#define NETTCP_REGISTER_TO_QML(Type)                                                                                   \
public:                                                                                                                \
    static void registerToQml(                                                                                         \
        const char* uri, const int majorVersion, const int minorVersion, const char* name = #Type)                     \
    {                                                                                                                  \
        qmlRegisterType<Type>(uri, majorVersion, minorVersion, name);                                                  \
    }                                                                                                                  \
                                                                                                                       \
private:

#define NETTCP_SINGLETON_IMPL(Class, name, Name)                                                                       \
public:                                                                                                                \
    static Class& name()                                                                                               \
    {                                                                                                                  \
        static Class ret;                                                                                              \
        return ret;                                                                                                    \
    }                                                                                                                  \
    static QObject* set##Name(QQmlEngine* qmlEngine, QJSEngine* jsEngine)                                              \
    {                                                                                                                  \
        Q_UNUSED(jsEngine)                                                                                             \
        Q_UNUSED(qmlEngine)                                                                                            \
        QObject* ret = &name();                                                                                        \
        QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);                                                 \
        return ret;                                                                                                    \
    }                                                                                                                  \
    static void registerSingleton(                                                                                     \
        const char* uri, const int majorVersion, const int minorVersion, const char* n = #Class)                       \
    {                                                                                                                  \
        qmlRegisterSingletonType<Class>(uri, majorVersion, minorVersion, n, &Class::set##Name);                        \
    }

#endif
