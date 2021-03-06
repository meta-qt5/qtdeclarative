/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQMLDATA_P_H
#define QQMLDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qtqmlglobal_p.h>
#include <private/qobject_p.h>
#include <private/qv8_p.h>

QT_BEGIN_NAMESPACE

template <class Key, class T> class QHash;
class QQmlGuardImpl;
class QQmlCompiledData;
class QQmlAbstractBinding;
class QQmlAbstractBoundSignal;
class QQmlContext;
class QQmlPropertyCache;
class QQmlContextData;
class QQmlNotifier;
class QQmlDataExtended;
class QQmlNotifierEndpoint;
// This class is structured in such a way, that simply zero'ing it is the
// default state for elemental object allocations.  This is crucial in the
// workings of the QQmlInstruction::CreateSimpleObject instruction.
// Don't change anything here without first considering that case!
class Q_QML_PRIVATE_EXPORT QQmlData : public QAbstractDeclarativeData
{
public:
    QQmlData()
        : ownMemory(true), ownContext(false), indestructible(true), explicitIndestructibleSet(false), 
          hasTaintedV8Object(false), isQueuedForDeletion(false), rootObjectInCreation(false),
          hasVMEMetaObject(false), parentFrozen(false), notifyList(0), context(0), outerContext(0),
          bindings(0), signalHandlers(0), nextContextObject(0), prevContextObject(0), bindingBitsSize(0), bindingBits(0),
          lineNumber(0), columnNumber(0), compiledData(0), deferredIdx(0), v8objectid(0),
          propertyCache(0), guards(0), extendedData(0) {
        init();
    }

    static inline void init() {
        static bool initialized = false;
        if (!initialized) {
            initialized = true;
            QAbstractDeclarativeData::destroyed = destroyed;
            QAbstractDeclarativeData::parentChanged = parentChanged;
            QAbstractDeclarativeData::signalEmitted = signalEmitted;
            QAbstractDeclarativeData::receivers = receivers;
            QAbstractDeclarativeData::isSignalConnected = isSignalConnected;
        }
    }

    static void destroyed(QAbstractDeclarativeData *, QObject *);
    static void parentChanged(QAbstractDeclarativeData *, QObject *, QObject *);
    static void signalEmitted(QAbstractDeclarativeData *, QObject *, int, void **);
    static int receivers(QAbstractDeclarativeData *, const QObject *, int);
    static bool isSignalConnected(QAbstractDeclarativeData *, const QObject *, int);

    void destroyed(QObject *);
    void parentChanged(QObject *, QObject *);

    void setImplicitDestructible() {
        if (!explicitIndestructibleSet) indestructible = false;
    }

    quint32 ownMemory:1;
    quint32 ownContext:1;
    quint32 indestructible:1;
    quint32 explicitIndestructibleSet:1;
    quint32 hasTaintedV8Object:1;
    quint32 isQueuedForDeletion:1;
    /*
     * rootObjectInCreation should be true only when creating top level CPP and QML objects,
     * v8 GC will check this flag, only deletes the objects when rootObjectInCreation is false.
     */
    quint32 rootObjectInCreation:1;
    quint32 hasVMEMetaObject:1;
    quint32 parentFrozen:1;
    quint32 dummy:23;

    struct NotifyList {
        quint64 connectionMask;

        quint16 maximumTodoIndex;
        quint16 notifiesSize;

        QQmlNotifierEndpoint *todo;
        QQmlNotifierEndpoint**notifies;
        void layout();
    private:
        void layout(QQmlNotifierEndpoint*);
    };
    NotifyList *notifyList;
    
    inline QQmlNotifierEndpoint *notify(int index);
    void addNotify(int index, QQmlNotifierEndpoint *);
    int endpointCount(int index);
    bool signalHasEndpoint(int index);
    void disconnectNotifiers();

    // The context that created the C++ object
    QQmlContextData *context; 
    // The outermost context in which this object lives
    QQmlContextData *outerContext;

    QQmlAbstractBinding *bindings;
    QQmlAbstractBoundSignal *signalHandlers;

    // Linked list for QQmlContext::contextObjects
    QQmlData *nextContextObject;
    QQmlData**prevContextObject;

    int bindingBitsSize;
    quint32 *bindingBits; 

    inline bool hasBindingBit(int) const;
    void clearBindingBit(int);
    void setBindingBit(QObject *obj, int);

    inline bool hasPendingBindingBit(int) const;
    void setPendingBindingBit(QObject *obj, int);
    void clearPendingBindingBit(int);

    quint16 lineNumber;
    quint16 columnNumber;

    QQmlCompiledData *compiledData;
    unsigned int deferredIdx;

    quint32 v8objectid;
    v8::Persistent<v8::Object> v8object;

    QQmlPropertyCache *propertyCache;

    QQmlGuardImpl *guards;

    static QQmlData *get(const QObject *object, bool create = false) {
        QObjectPrivate *priv = QObjectPrivate::get(const_cast<QObject *>(object));
        if (priv->wasDeleted) {
            Q_ASSERT(!create);
            return 0;
        } else if (priv->declarativeData) {
            return static_cast<QQmlData *>(priv->declarativeData);
        } else if (create) {
            priv->declarativeData = new QQmlData;
            return static_cast<QQmlData *>(priv->declarativeData);
        } else {
            return 0;
        }
    }

    bool hasExtendedData() const { return extendedData != 0; }
    QHash<int, QObject *> *attachedProperties() const;

    static inline bool wasDeleted(QObject *);

    static void markAsDeleted(QObject *);
    static void setQueuedForDeletion(QObject *);

    static inline void flushPendingBinding(QObject *, int coreIndex);

private:
    // For attachedProperties
    mutable QQmlDataExtended *extendedData;

    void flushPendingBindingImpl(int coreIndex);
};

bool QQmlData::wasDeleted(QObject *object)
{
    if (!object)
        return true;

    QObjectPrivate *priv = QObjectPrivate::get(object);
    if (priv->wasDeleted)
        return true;

    return priv->declarativeData &&
           static_cast<QQmlData *>(priv->declarativeData)->isQueuedForDeletion;
}

QQmlNotifierEndpoint *QQmlData::notify(int index)
{
    Q_ASSERT(index <= 0xFFFF);

    if (!notifyList || !(notifyList->connectionMask & (1ULL << quint64(index % 64)))) {
        return 0;
    } else if (index < notifyList->notifiesSize) {
        return notifyList->notifies[index];
    } else if (index <= notifyList->maximumTodoIndex) {
        notifyList->layout();
    }

    if (index < notifyList->notifiesSize) {
        return notifyList->notifies[index];
    } else {
        return 0;
    }
}

bool QQmlData::hasBindingBit(int coreIndex) const
{
    int bit = coreIndex * 2;
    if (bindingBitsSize > bit)
        return bindingBits[bit / 32] & (1 << (bit % 32));
    else
        return false;
}

bool QQmlData::hasPendingBindingBit(int coreIndex) const
{
    int bit = coreIndex * 2 + 1;
    if (bindingBitsSize > bit)
        return bindingBits[bit / 32] & (1 << (bit % 32));
    else
        return false;
}

void QQmlData::flushPendingBinding(QObject *o, int coreIndex)
{
    QQmlData *data = QQmlData::get(o, false);
    if (data && data->hasPendingBindingBit(coreIndex))
        data->flushPendingBindingImpl(coreIndex);
}

QT_END_NAMESPACE

#endif // QQMLDATA_P_H
