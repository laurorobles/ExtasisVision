#pragma once
#include <JuceHeader.h>

namespace ExtasisDesign
{
    // =========================================================================
    // REGLAS RÍGIDAS DE ESPACIADO Y MÁRGENES (Múltiplos de 4/8)
    // =========================================================================
    constexpr int marginBase = 8;
    constexpr int marginSmall = marginBase;         // 8px
    constexpr int marginMedium = marginBase * 2;    // 16px
    constexpr int marginLarge = marginBase * 4;     // 32px

    constexpr int panelCornerRadius = 4;
    constexpr int defaultControlHeight = 32;

    // =========================================================================
    // PALETA DE COLORES (Estética T-1000 / Cyber-Dark)
    // =========================================================================
    const juce::Colour bgBase        = juce::Colour::fromRGB (15, 15, 18);   // Negro profundo casi azulado
    const juce::Colour bgPanel       = juce::Colour::fromRGB (28, 30, 34);   // Gris muy oscuro
    const juce::Colour metalChrome   = juce::Colour::fromRGB (180, 185, 190);// Metal reflejante
    const juce::Colour metalDark     = juce::Colour::fromRGB (50, 55, 60);   // Cromo oscuro
    const juce::Colour hudRed        = juce::Colour::fromRGB (235, 40, 40);  // Rojo Terminator (Alertas/Acentos)
    const juce::Colour hudCyan       = juce::Colour::fromRGB (0, 235, 255);  // Cian Sci-Fi (Selección)

    // =========================================================================
    // TIPOGRAFÍA
    // =========================================================================
    constexpr float fontTitleSize = 32.0f;
    constexpr float fontHeaderSize = 18.0f;
    constexpr float fontBodySize = 13.0f;

    // Función auxiliar para obtener la fuente estándar del sistema con peso específico
    inline juce::Font getFontBody() { return juce::Font (fontBodySize); }
    inline juce::Font getFontHeader() { return juce::Font (fontHeaderSize, juce::Font::bold); }
    inline juce::Font getFontTitle() { return juce::Font (fontTitleSize, juce::Font::bold); }

    // =========================================================================
    // LOOK AND FEEL PERSONALIZADO (Metal Líquido)
    // =========================================================================
    class ExtasisLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        ExtasisLookAndFeel()
        {
            setColour (juce::TooltipWindow::backgroundColourId, bgPanel.darker());
            setColour (juce::TooltipWindow::textColourId, hudCyan);
            setColour (juce::TooltipWindow::outlineColourId, metalDark);
        }

        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                               juce::Slider& /*slider*/) override
        {
            auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
            auto centreX = (float) x + (float) width  * 0.5f;
            auto centreY = (float) y + (float) height * 0.5f;
            auto rx = centreX - radius;
            auto ry = centreY - radius;
            auto rw = radius * 2.0f;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            // Efecto de cromo líquido (Metal Gradient)
            juce::ColourGradient cg (metalChrome.brighter(0.2f), centreX, ry, 
                                     metalDark.darker(0.8f), centreX, ry + rw, false);
            g.setGradientFill (cg);
            g.fillEllipse (rx, ry, rw, rw);

            // Borde reflectante interior
            g.setColour (juce::Colours::white.withAlpha(0.15f));
            g.drawEllipse (rx + 1, ry + 1, rw - 2, rw - 2, 1.0f);

            // Borde exterior
            g.setColour (bgBase);
            g.drawEllipse (rx, ry, rw, rw, 1.5f);

            // Indicador estilo HUD Terminator (Rojo / Cian)
            juce::Path p;
            auto pointerLength = radius * 0.6f;
            auto pointerThickness = 3.0f;
            p.addRectangle (-pointerThickness * 0.5f, -radius + 4.0f, pointerThickness, pointerLength);
            p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
            
            // Brillo resplandeciente del indicador
            g.setColour (hudRed.withAlpha(0.3f));
            g.fillPath (p);
            g.setColour (hudRed);
            g.strokePath (p, juce::PathStrokeType(1.5f));
        }
    };
}
