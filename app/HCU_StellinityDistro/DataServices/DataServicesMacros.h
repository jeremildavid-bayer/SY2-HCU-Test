#ifndef DATA_SERVICES_MACROS_H
#define DATA_SERVICES_MACROS_H

#include <QMutex>

// ===========================================
// Type: Data Service

// Macro to set Last Data
#define SET_LAST_DATA(NAME) \
    do {\
        m_Last_ ## NAME = m_ ## NAME; }while(0);

// Macro to emit Data Changed signal
#define EMIT_DATA_CHANGED_SIGNAL(NAME) \
    do {\
        m_Last_ ## NAME = m_ ## NAME; \
        emit signalDataChanged_ ## NAME(m_ ## NAME, m_Last_ ## NAME); }while(0);

// Macro to create Data Changed signal
#define CREATE_DATA_CHANGED_SIGNAL(TYPE, NAME) \
    void signalDataChanged_ ## NAME(const TYPE &val, const TYPE &prevVal);

// Macro to create members and methods
#define CREATE_DATA_MEMBERS(TYPE, NAME) \
    private: \
        TYPE m_ ## NAME; \
        TYPE m_Last_ ## NAME; \
        QMutex mutex_ ## NAME; \
    public: \
        void set ## NAME(const TYPE &val) { \
            mutex_ ## NAME.lock(); \
            if (m_ ## NAME != val) { \
                m_Last_ ## NAME = m_ ## NAME; \
                m_ ## NAME = val;  \
                mutex_ ## NAME.unlock(); \
                if (!m_DataLocked) { emit signalDataChanged_ ## NAME(m_ ## NAME, m_Last_ ## NAME); } \
            } \
            else { mutex_ ## NAME.unlock(); } \
        } \
        \
        TYPE get ## NAME() { \
            mutex_ ## NAME.lock();  \
            TYPE buf = m_ ## NAME; \
            mutex_ ## NAME.unlock(); \
            return buf; }

// ===========================================
// Type: Config Service

#define EMIT_CONFIG_CHANGED_SIGNAL(NAME) \
    do {\
        db->get(#NAME, m_ ## NAME); \
        emit signalConfigChanged_ ## NAME(m_ ## NAME, m_ ## NAME); }while(0);


#define SET_CONFIG_BY_NAME(cfgItem, NAME, setChangedAt) \
    do {\
        if (cfgItem.keyName == _L(#NAME)) {\
            mutex_ ## NAME.lock(); \
            db->get(#NAME, m_Last_ ## NAME); \
            if (!m_Last_ ## NAME.compareConfigValue(cfgItem)) { \
                m_ ## NAME = cfgItem; \
                if (setChangedAt) {\
                    db->set(#NAME, m_ ## NAME); \
                } else {\
                    db->set(m_ ## NAME);\
                }\
                mutex_ ## NAME.unlock(); \
                emit signalConfigChanged_ ## NAME(m_ ## NAME, m_Last_ ## NAME);\
            } else { \
                mutex_ ## NAME.unlock();\
            } \
        }\
    }while(0);

// Macro to create property MEMBER and GET/SET functions - magic!
#define CREATE_CONFIG_CHANGED_SIGNAL(NAME) \
    void signalConfigChanged_ ## NAME(const Config::Item &val, const Config::Item &prevVal);

#define CREATE_CONFIG_MEMBERS(NAME) \
    private: \
    QMutex mutex_ ## NAME; \
    Config::Item m_ ## NAME; \
    Config::Item m_Last_ ## NAME; \
    public: \
    void set ## NAME(const Config::Item &val) { \
        mutex_ ## NAME.lock(); \
        db->get(#NAME, m_Last_ ## NAME); \
        if (!m_Last_ ## NAME.compareConfigValue(val)) { \
            m_ ## NAME = val; \
            db->set(#NAME, m_ ## NAME); \
            mutex_ ## NAME.unlock(); \
            emit signalConfigChanged_ ## NAME(m_ ## NAME, m_Last_ ## NAME);\
            emit signalConfigChanged(); \
        } else { \
            mutex_ ## NAME.unlock(); } \
    } \
    \
    Config::Item get ## NAME() { \
        mutex_ ## NAME.lock();  \
        Config::Item valBuf; \
        db->get(#NAME, valBuf); \
        mutex_ ## NAME.unlock(); \
        return valBuf; }

//e.g  for bool type: C_TYPE=bool, QVARIANT_TO_C_FN=toBool
//     for int type:  C_TYPE=int,  QVARIANT_TO_C_FN=toInt
#define CREATE_CONFIG_MEMBERS_EX(NAME, C_TYPE, VARIANT_TO_C) \
    private: \
    QMutex mutex_ ## NAME; \
    Config::Item m_ ## NAME; \
    Config::Item m_Last_ ## NAME; \
    C_TYPE m_cpp_ ## NAME;\
    public: \
    void set ## NAME(const Config::Item &val) { \
        mutex_ ## NAME.lock(); \
        db->get(#NAME, m_Last_ ## NAME); \
        if (!m_Last_ ## NAME.compareConfigValue(val)) { \
            m_ ## NAME = val; \
            db->set(#NAME, m_ ## NAME); \
            mutex_ ## NAME.unlock(); \
            emit signalConfigChanged_ ## NAME(m_ ## NAME, m_Last_ ## NAME);\
            emit signalConfigChanged(); \
        } else { \
            mutex_ ## NAME.unlock(); } \
    } \
    Config::Item getItem_ ## NAME() { \
        mutex_ ## NAME.lock();  \
        Config::Item valBuf; \
        db->get(#NAME, valBuf); \
        mutex_ ## NAME.unlock(); \
        return valBuf; }                                                \
    C_TYPE get_ ## NAME () {                                            \
        QMutexLocker locker(&mutex_ ## NAME);                           \
        if (!m_ ## NAME.value.isValid())                                \
        {                                                               \
            db->get(#NAME, m_ ## NAME);                                 \
            m_cpp_ ## NAME = m_ ## NAME. value. VARIANT_TO_C();         \
        }                                                               \
        m_cpp_ ## NAME = m_ ## NAME. value. VARIANT_TO_C();             \
        return m_cpp_ ## NAME;                                          \
    }                                                                   \
    void set_ ## NAME (C_TYPE new_val) {                                \
        mutex_ ## NAME.lock(); \
        if (!m_ ## NAME.value.isValid())                                \
        {                                                               \
            db->get(#NAME, m_ ## NAME);                                 \
        }                                                               \
        m_cpp_ ## NAME = m_ ## NAME. value. VARIANT_TO_C();             \
        if (m_cpp_ ## NAME != new_val)                                  \
        {                                                               \
            m_Last_ ## NAME = m_ ## NAME;                               \
            m_ ## NAME. value = new_val;                                \
            db->set(#NAME, m_ ## NAME);                                 \
            mutex_ ## NAME.unlock();                                    \
            emit signalConfigChanged_ ## NAME(m_ ## NAME, m_Last_ ## NAME);\
            emit signalConfigChanged();                                 \
        }                                                               \
        else                                                            \
            mutex_ ## NAME.unlock();                                    \
    }                                                                   \

#endif // DATA_SERVICES_MACROS_H
