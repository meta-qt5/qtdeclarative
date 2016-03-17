/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgd3d12glyphcache_p.h"
#include "qsgd3d12engine_p.h"

QT_BEGIN_NAMESPACE

// Keep it simple: allocate a large texture and never resize.
static const int TEXTURE_WIDTH = 2048;
static const int TEXTURE_HEIGHT = 2048;

QSGD3D12GlyphCache::QSGD3D12GlyphCache(QSGD3D12Engine *engine, QFontEngine::GlyphFormat format, const QTransform &matrix)
    : QTextureGlyphCache(format, matrix),
      m_engine(engine)
{
}

QSGD3D12GlyphCache::~QSGD3D12GlyphCache()
{
    if (m_id)
        m_engine->releaseTexture(m_id);
}

void QSGD3D12GlyphCache::createTextureData(int, int)
{
    m_id = m_engine->genTexture();
    Q_ASSERT(m_id);
    m_engine->createTexture(m_id, QSize(TEXTURE_WIDTH, TEXTURE_HEIGHT),
                            QImage::Format_ARGB32_Premultiplied, QSGD3D12Engine::CreateWithAlpha);
}

void QSGD3D12GlyphCache::resizeTextureData(int, int)
{
    // nothing to do here
}

void QSGD3D12GlyphCache::beginFillTexture()
{
    Q_ASSERT(m_glyphImages.isEmpty() && m_glyphPos.isEmpty());
}

void QSGD3D12GlyphCache::fillTexture(const Coord &c, glyph_t glyph, QFixed subPixelPosition)
{
    QImage mask = textureMapForGlyph(glyph, subPixelPosition);
    const int maskWidth = mask.width();
    const int maskHeight = mask.height();

    if (mask.format() == QImage::Format_Mono) {
        mask = mask.convertToFormat(QImage::Format_Indexed8);
        for (int y = 0; y < maskHeight; ++y) {
            uchar *src = mask.scanLine(y);
            for (int x = 0; x < maskWidth; ++x)
                src[x] = -src[x]; // convert 0 and 1 into 0 and 255
        }
    } else if (mask.depth() == 32) {
        if (mask.format() == QImage::Format_RGB32) {
            // We need to make the alpha component equal to the average of the RGB values.
            // This is needed when drawing sub-pixel antialiased text on translucent targets.
            for (int y = 0; y < maskHeight; ++y) {
                QRgb *src = reinterpret_cast<QRgb *>(mask.scanLine(y));
                for (int x = 0; x < maskWidth; ++x) {
                    const int r = qRed(src[x]);
                    const int g = qGreen(src[x]);
                    const int b = qBlue(src[x]);
                    int avg;
                    if (mask.format() == QImage::Format_RGB32)
                        avg = (r + g + b + 1) / 3; // "+1" for rounding.
                    else // Format_ARGB32_Premultiplied
                        avg = qAlpha(src[x]);
                    src[x] = qRgba(r, g, b, avg);
                }
            }
        }
    }

    m_glyphImages.append(mask);
    m_glyphPos.append(QPoint(c.x, c.y));
}

void QSGD3D12GlyphCache::endFillTexture()
{
    if (m_glyphImages.isEmpty())
        return;

    Q_ASSERT(m_id);

    m_engine->queueTextureUpload(m_id, m_glyphImages, m_glyphPos);

    // Nothing else left to do, it is up to the text material to call
    // activateTexture() which will then add the texture dependency to the frame.

    m_glyphImages.clear();
    m_glyphPos.clear();
}

int QSGD3D12GlyphCache::glyphPadding() const
{
    return 1;
}

int QSGD3D12GlyphCache::maxTextureWidth() const
{
    return TEXTURE_WIDTH;
}

int QSGD3D12GlyphCache::maxTextureHeight() const
{
    return TEXTURE_HEIGHT;
}

void QSGD3D12GlyphCache::activateTexture()
{
    if (m_id)
        m_engine->activateTexture(m_id);
}

int QSGD3D12GlyphCache::currentWidth() const
{
    return TEXTURE_WIDTH;
}

int QSGD3D12GlyphCache::currentHeight() const
{
    return TEXTURE_HEIGHT;
}

QT_END_NAMESPACE
