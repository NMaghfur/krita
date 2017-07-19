/*
 *  Copyright (c) 2003-2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_canvas_controls_manager.h"

#include <kactioncollection.h>
#include <QAction>

#include <KoCanvasResourceManager.h>

#include "kis_action.h"
#include "kis_action_manager.h"
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"

#include <brushengine/kis_locked_properties_proxy.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <math.h>

const int STEP = 25;

KisCanvasControlsManager::KisCanvasControlsManager(KisViewManager * view) : m_view(view)
{

}

KisCanvasControlsManager::~KisCanvasControlsManager()
{

}

void KisCanvasControlsManager::setup(KisActionManager *actionManager)
{
    KisAction *lighterColor = actionManager->createAction("make_brush_color_lighter");
    connect(lighterColor, SIGNAL(triggered()), SLOT(makeColorLighter()));

    KisAction *darkerColor = actionManager->createAction("make_brush_color_darker");
    connect(darkerColor, SIGNAL(triggered()), SLOT(makeColorDarker()));
    KisAction *saturatedColor = actionManager->createAction("make_brush_color_saturated");
    connect(saturatedColor, SIGNAL(triggered()), SLOT(makeColorSaturated()));

    KisAction *desaturatedColor = actionManager->createAction("make_brush_color_desaturated");
    connect(desaturatedColor, SIGNAL(triggered()), SLOT(makeColorDesaturated()));

    KisAction *hueClockwise = actionManager->createAction("shift_brush_color_clockwise");
    connect(hueClockwise, SIGNAL(triggered()), SLOT(shiftHueClockWise()));

    KisAction *hueCounterClockwise = actionManager->createAction("shift_brush_color_counter_clockwise");
    connect(hueCounterClockwise, SIGNAL(triggered()), SLOT(shiftHueCounterClockWise()));

    KisAction *moreRed = actionManager->createAction("make_brush_color_redder");
    connect(moreRed, SIGNAL(triggered()), SLOT(makeColorRed()));

    KisAction *moreGreen = actionManager->createAction("make_brush_color_greener");
    connect(moreGreen, SIGNAL(triggered()), SLOT(makeColorGreen()));

    KisAction *moreBlue = actionManager->createAction("make_brush_color_bluer");
    connect(moreBlue, SIGNAL(triggered()), SLOT(makeColorBlue()));

    KisAction *moreYellow = actionManager->createAction("make_brush_color_yellower");
    connect(moreYellow, SIGNAL(triggered()), SLOT(makeColorYellow()));

    KisAction *increaseOpacity = actionManager->createAction("increase_opacity");
    connect(increaseOpacity, SIGNAL(triggered()), SLOT(increaseOpacity()));

    KisAction *decreaseOpacity = actionManager->createAction("decrease_opacity");
    connect(decreaseOpacity, SIGNAL(triggered()), SLOT(decreaseOpacity()));

    KisAction *setOpacityOpaque = actionManager->createAction("set_opacity_opaque");
    connect(setOpacityOpaque, SIGNAL(triggered()), SLOT(setOpacityOpaque()));

    KisAction *setOpacityHigh = actionManager->createAction("set_opacity_high");
    connect(setOpacityHigh, SIGNAL(triggered()), SLOT(setOpacityHigh()));

    KisAction *setOpacityMedium = actionManager->createAction("set_opacity_medium");
    connect(setOpacityMedium, SIGNAL(triggered()), SLOT(setOpacityMedium()));

    KisAction *setOpacityLow = actionManager->createAction("set_opacity_low");
    connect(setOpacityLow, SIGNAL(triggered()), SLOT(setOpacityLow()));

    KisAction *setOpacityVeryLow = actionManager->createAction("set_opacity_very_low");
    connect(setOpacityVeryLow, SIGNAL(triggered()), SLOT(setOpacityVeryLow()));

    KisAction *setOpacityExtremelyLow = actionManager->createAction("set_opacity_extremely_low");
    connect(setOpacityExtremelyLow, SIGNAL(triggered()), SLOT(setOpacityExtremelyLow()));

    KisAction *halfOpacity = actionManager->createAction("half_opacity");
    connect(halfOpacity, SIGNAL(triggered()), SLOT(halfOpacity()));

    KisAction *doubleOpacity = actionManager->createAction("double_opacity");
    connect(doubleOpacity, SIGNAL(triggered()), SLOT(doubleOpacity()));
}

void KisCanvasControlsManager::setView(QPointer<KisView>imageView)
{
    Q_UNUSED(imageView);
}

void KisCanvasControlsManager::transformColor(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_lightness", 10);


    KoColor color = m_view->resourceProvider()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    if (color.colorSpace()->colorModelId().id()=="CMYKA" || color.colorSpace()->colorModelId().id()=="XYZA"){
        QColor rgb = color.toQColor();
        int h = 0, s = 0, v = 0;
        rgb.getHsv(&h,&s,&v);
        if ((v < 255) || ((s == 0) || (s == 255))) {
            v += step;
            v = qBound(0,v,255);
        } else {
            s += -step;
            s = qBound(0,s,255);
        }
        rgb.setHsv(h,s,v);
        color.fromQColor(rgb);
    } else if (step<0){
        color.colorSpace()->decreaseLuminosity(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseLuminosity(color.data(), 1.0/steps);
    }
    m_view->resourceProvider()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
}
void KisCanvasControlsManager::transformSaturation(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_saturation", 10);

    KoColor color = m_view->resourceProvider()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    if (color.colorSpace()->colorModelId().id()=="CMYKA" || color.colorSpace()->colorModelId().id()=="XYZA"){
        QColor rgb = color.toQColor();
        int h = 0, s = 0, v = 0;
        rgb.getHsl(&h,&s,&v);
        s += step;
        s = qBound(0,s,255);
        rgb.setHsl(h,s,v);
        color.fromQColor(rgb);
    } else if (step<0){
        color.colorSpace()->decreaseSaturation(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseSaturation(color.data(), 1.0/steps);
    }
    m_view->resourceProvider()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
}
void KisCanvasControlsManager::transformHue(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_hue", 36);

    KoColor color = m_view->resourceProvider()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    if (color.colorSpace()->colorModelId().id()=="CMYKA" || color.colorSpace()->colorModelId().id()=="XYZA"){
        QColor rgb = color.toQColor();
        int h = 0, s = 0, v = 0;
        rgb.getHsl(&h,&s,&v);
        h += step;
        if (h>360.0 || h<0.0){h=fmod(h, 360.0);}
        rgb.setHsl(h,s,v);
        color.fromQColor(rgb);
    } else if (step<0){
        color.colorSpace()->increaseHue(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->decreaseHue(color.data(), 1.0/steps);
    }
    m_view->resourceProvider()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
}
void KisCanvasControlsManager::transformRed(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_redgreen", 10);

    KoColor color = m_view->resourceProvider()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    if (step<0){
        color.colorSpace()->increaseGreen(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseRed(color.data(), 1.0/steps);
    }
    m_view->resourceProvider()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
}
void KisCanvasControlsManager::transformBlue(int step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;
    KConfigGroup hotkeycfg =  KSharedConfig::openConfig()->group("colorhotkeys");
    int steps = hotkeycfg.readEntry("steps_blueyellow", 10);

    KoColor color = m_view->resourceProvider()->resourceManager()->resource(KoCanvasResourceManager::ForegroundColor).value<KoColor>();
    if (step<0){
        color.colorSpace()->increaseYellow(color.data(), 1.0/steps);
    } else {
        color.colorSpace()->increaseBlue(color.data(), 1.0/steps);
    }
    m_view->resourceProvider()->resourceManager()->setResource(KoCanvasResourceManager::ForegroundColor, color);
}


void KisCanvasControlsManager::makeColorDarker()
{
    transformColor(-STEP);
}

void KisCanvasControlsManager::makeColorLighter()
{
    transformColor(STEP);
}

void KisCanvasControlsManager::makeColorDesaturated()
{
    transformSaturation(-STEP);
}

void KisCanvasControlsManager::makeColorSaturated()
{
    transformSaturation(STEP);
}
void KisCanvasControlsManager::shiftHueClockWise()
{
    transformHue(STEP);
}

void KisCanvasControlsManager::shiftHueCounterClockWise()
{
    transformHue(-STEP);
}
void KisCanvasControlsManager::makeColorRed()
{
    transformRed(STEP);
}
void KisCanvasControlsManager::makeColorGreen()
{
    transformRed(-STEP);
}
void KisCanvasControlsManager::makeColorBlue()
{
    transformBlue(STEP);
}
void KisCanvasControlsManager::makeColorYellow()
{
    transformBlue(-STEP);
}

void KisCanvasControlsManager::stepAlpha(float step)
{
    if (!m_view) return;
    if (!m_view->canvasBase()) return;
    if (!m_view->resourceProvider()->resourceManager()) return;

    qreal alpha = m_view->resourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::Opacity).toDouble();
    alpha += step;
    alpha = qBound<qreal>(0.0, alpha, 1.0);
    m_view->canvasBase()->resourceManager ()->setResource(KisCanvasResourceProvider::Opacity, alpha);

    // FIXME: DK: should we uncomment it back?
    //KisLockedPropertiesProxySP p = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(m_view->resourceProvider()->currentPreset()->settings());
    //p->setProperty("OpacityValue", alpha);
}

void KisCanvasControlsManager::increaseOpacity()
{
    stepAlpha(0.1f);
}

void KisCanvasControlsManager::decreaseOpacity()
{
    stepAlpha(-0.1f);
}

void KisCanvasControlsManager::setAlpha(float value)
{
  if (!m_view) return;
  if (!m_view->canvas()) return;
  if (!m_view->resourceProvider()->resourceManager()) return;

  qreal alpha = m_view->resourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::Opacity).toDouble();
  alpha = value;
  alpha = qBound<qreal>(0.0, value, 1.0);

  m_view->canvasBase()->resourceManager ()->setResource(KisCanvasResourceProvider::Opacity, alpha);

}

void KisCanvasControlsManager::setOpacityOpaque()
{
    setAlpha(1.0f);
}

void KisCanvasControlsManager::setOpacityHigh()
{
    setAlpha(0.8f);
}

void KisCanvasControlsManager::setOpacityMedium()
{
    setAlpha(0.5f);
}

void KisCanvasControlsManager::setOpacityLow()
{
    setAlpha(0.3f);
}

void KisCanvasControlsManager::setOpacityVeryLow()
{
    setAlpha(0.1f);
}

void KisCanvasControlsManager::setOpacityExtremelyLow()
{
    setAlpha(0.01f);
}

void KisCanvasControlsManager::multiplyAlpha(float factor)
{
  if (!m_view) return;
  if (!m_view->canvas()) return;
  if (!m_view->resourceProvider()->resourceManager()) return;

  qreal alpha = m_view->resourceProvider()->resourceManager()->resource(KisCanvasResourceProvider::Opacity).toDouble();
  alpha = alpha * factor;
  alpha = qBound<qreal>(0.0, alpha, 1.0);

  m_view->canvasBase()->resourceManager ()->setResource(KisCanvasResourceProvider::Opacity, alpha);

}

void KisCanvasControlsManager::halfOpacity()
{
  multiplyAlpha(0.5f);
}

void KisCanvasControlsManager::doubleOpacity()
{
  multiplyAlpha(2.0f);
}

