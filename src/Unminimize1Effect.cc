/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Unminimize1Effect.h"

// KConfigSkeleton
#include "unminimize1config.h"

Unminimize1Effect::Unminimize1Effect()
{
    initConfig<Unminimize1Config>();
    reconfigure(ReconfigureAll);

    connect(KWin::effects, &KWin::EffectsHandler::windowUnminimized,
        this, &Unminimize1Effect::start);
    connect(KWin::effects, &KWin::EffectsHandler::windowMinimized,
        this, &Unminimize1Effect::stop);
    connect(KWin::effects, &KWin::EffectsHandler::windowDeleted,
        this, &Unminimize1Effect::stop);
}

Unminimize1Effect::~Unminimize1Effect()
{
}

void Unminimize1Effect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags);

    Unminimize1Config::self()->read();
    m_duration = animationTime(Unminimize1Config::duration() > 0
            ? Unminimize1Config::duration()
            : 160);
    m_opacity = Unminimize1Config::opacity();
    m_scale = Unminimize1Config::scale();
}

void Unminimize1Effect::prePaintScreen(KWin::ScreenPrePaintData& data, int time)
{
    auto it = m_animations.begin();
    while (it != m_animations.end()) {
        Timeline& t = *it;
        t.update(time);
        if (t.done()) {
            it = m_animations.erase(it);
        } else {
            ++it;
        }
    }

    if (!m_animations.isEmpty()) {
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
    }

    KWin::effects->prePaintScreen(data, time);
}

void Unminimize1Effect::prePaintWindow(KWin::EffectWindow* w, KWin::WindowPrePaintData& data, int time)
{
    if (m_animations.contains(w)) {
        data.setTransformed();
    }

    KWin::effects->prePaintWindow(w, data, time);
}

void Unminimize1Effect::paintWindow(KWin::EffectWindow* w, int mask, QRegion region, KWin::WindowPaintData& data)
{
    const auto it = m_animations.constFind(w);
    if (it == m_animations.cend()) {
        KWin::effects->paintWindow(w, mask, region, data);
        return;
    }

    const QRect geometry(w->geometry());
    const QRect icon(w->iconGeometry());

    if (icon.isValid() && geometry.isValid()) {
        const qreal t = (*it).value();
        const qreal opacityProgress = t;
        const qreal scaleProgress = interpolate(m_scale, 1, t);

        const qreal fromScale = icon.width() > icon.height()
            ? static_cast<qreal>(icon.width()) / geometry.width()
            : static_cast<qreal>(icon.height()) / geometry.height();

        data.setXScale(interpolate(fromScale, data.xScale(), scaleProgress));
        data.setYScale(interpolate(fromScale, data.yScale(), scaleProgress));
        data.setXTranslation(interpolate(icon.x() - geometry.x(), data.xTranslation(), scaleProgress));
        data.setYTranslation(interpolate(icon.y() - geometry.y(), data.yTranslation(), scaleProgress));

        data.multiplyOpacity(interpolate(m_opacity, 1, opacityProgress));
    }

    KWin::effects->paintWindow(w, mask, region, data);
}

void Unminimize1Effect::postPaintScreen()
{
    if (!m_animations.isEmpty()) {
        KWin::effects->addRepaintFull();
    }

    KWin::effects->postPaintScreen();
}

bool Unminimize1Effect::isActive() const
{
    return !m_animations.isEmpty();
}

bool Unminimize1Effect::supported()
{
    return KWin::effects->animationsSupported();
}

void Unminimize1Effect::start(KWin::EffectWindow* w)
{
    if (KWin::effects->activeFullScreenEffect() != nullptr) {
        return;
    }

    Timeline& t = m_animations[w];
    t.setDuration(m_duration);
    t.setEasingCurve(QEasingCurve::InCurve);
}

void Unminimize1Effect::stop(KWin::EffectWindow* w)
{
    m_animations.remove(w);
}
