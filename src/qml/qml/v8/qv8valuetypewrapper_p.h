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

#ifndef QV8VALUETYPEWRAPPER_P_H
#define QV8VALUETYPEWRAPPER_P_H

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

#include <QtCore/qglobal.h>
#include <QtQml/qqmllist.h>
#include <private/qtqmlglobal_p.h>
#include <private/qv8_p.h>
#include <private/qhashedstring_p.h>

QT_BEGIN_NAMESPACE

class QV8Engine;
class QV8ObjectResource;
class QQmlValueType;
class Q_QML_PRIVATE_EXPORT QV8ValueTypeWrapper
{
public:
    QV8ValueTypeWrapper();
    ~QV8ValueTypeWrapper();

    void init(QV8Engine *);
    void destroy();

    v8::Local<v8::Object> newValueType(QObject *, int, QQmlValueType *);
    v8::Local<v8::Object> newValueType(const QVariant &, QQmlValueType *);

    bool isValueType(v8::Handle<v8::Object>) const;

    QVariant toVariant(v8::Handle<v8::Object>, int typeHint, bool *succeeded);
    QVariant toVariant(v8::Handle<v8::Object>);
    QVariant toVariant(QV8ObjectResource *);

    static bool isEqual(QV8ObjectResource *, const QVariant& value);

private:
    static v8::Handle<v8::Value> ToStringGetter(v8::Local<v8::String> property,
                                                const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> ToString(const v8::Arguments &args);
    static v8::Handle<v8::Value> Getter(v8::Local<v8::String> property, 
                                        const v8::AccessorInfo &info);
    static v8::Handle<v8::Value> Setter(v8::Local<v8::String> property, 
                                        v8::Local<v8::Value> value,
                                        const v8::AccessorInfo &info);

    QV8Engine *m_engine;
    v8::Persistent<v8::Function> m_constructor;
    v8::Persistent<v8::Function> m_toString;
    v8::Persistent<v8::String> m_toStringSymbol;
    QHashedV8String m_toStringString;
};

QT_END_NAMESPACE

#endif // QV8VALUETYPEWRAPPER_P_H


