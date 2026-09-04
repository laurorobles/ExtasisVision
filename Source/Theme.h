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
}
