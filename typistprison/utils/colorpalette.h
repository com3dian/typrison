#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QColor>
#include <QString>

/**
 * Centralized color palette for the application.
 * All colors used throughout the codebase should be defined here.
 */
namespace ColorPalette {

    // ===== Background Colors =====
    namespace Background {
        const QString Dark = "#1F2020";           // Main dark background
        const QString DarkGray = "#2C2C2C";       // Darker gray background (selected tabs, containers)
        const QString MediumGray = "#262626";     // Medium gray (scrollbars, hover states)
        const QString LightGray = "#3C3C3C";      // Light gray background
        const QString Transparent = "transparent";
        
        inline QColor dark() { return QColor(Dark); }
        inline QColor darkGray() { return QColor(DarkGray); }
        inline QColor mediumGray() { return QColor(MediumGray); }
        inline QColor lightGray() { return QColor(LightGray); }
    }

    // ===== Text Colors =====
    namespace Text {
        const QString White = "#FFFFFF";          // Primary text color
        const QString Grey = "#BDBDBD";           // Secondary text color (most common grey)
        const QString LightGrey = "#DEDEDE";      // Light grey text
        const QString DarkGrey = "#656565";       // Dark grey text (disabled states)
        const QString MediumGrey = "#5A5A5A";     // Medium grey text
        const QString VeryDarkGrey = "#545555";   // Very dark grey
        const QString Light = "#B0B0B0";          // Light text (countdown timer)
        
        inline QColor white() { return QColor(White); }
        inline QColor grey() { return QColor(Grey); }
        inline QColor lightGrey() { return QColor(LightGrey); }
        inline QColor darkGrey() { return QColor(DarkGrey); }
        inline QColor mediumGrey() { return QColor(MediumGrey); }
    }

    // ===== Border Colors =====
    namespace Border {
        const QString Default = "#393B3B";        // Default border color
        const QString Light = "#BDBDBD";          // Light border (hover states)
        const QString Medium = "#5A5A5A";         // Medium border
        const QString Dark = "#333333";           // Dark border
        const QString Darker = "#3A3A3A";         // Darker border
        const QString LightGrey = "#999999";      // Light grey border
        const QString Scrollbar = "#2c2c2c";      // Scrollbar border
        const QString Accent = "#4a4a4a";         // Accent border
        const QString VeryDark = "#545555";       // Very dark border
        
        inline QColor defaultColor() { return QColor(Default); }
        inline QColor light() { return QColor(Light); }
        inline QColor medium() { return QColor(Medium); }
        inline QColor dark() { return QColor(Dark); }
    }

    // ===== Accent Colors =====
    namespace Accent {
        const QString Green = "#84e0a5";          // Primary accent green (selection, highlights)
        const QString GreenDark = "#80bd96";      // Darker green (borders)
        const QString GreenBackground = "#2C2C2C"; // Green selection background text
        
        const QString Red = "#E0715C";            // Primary red (error, escape)
        const QString RedDark = "#BA6757";        // Darker red (borders)
        const QString RedProgressStart = "#E84A2C"; // Red progress start
        const QString RedProgressEnd = "#804337";   // Red progress end
        
        const QString GreenProgressStart = "#84E0A5"; // Green progress start
        const QString GreenProgressEnd = "#3F7050";   // Green progress end
        
        inline QColor green() { return QColor(Green); }
        inline QColor greenDark() { return QColor(GreenDark); }
        inline QColor red() { return QColor(Red); }
        inline QColor redDark() { return QColor(RedDark); }
    }

    // ===== Traffic Light Colors =====
    namespace TrafficLight {
        const QString Yellow = "#F5BD4E";         // Yellow (minimize)
        const QString YellowHover = "#F5CD7D";    // Yellow hover
        const QString Green = "#62C454";          // Green (maximize)
        const QString GreenHover = "#83C779";     // Green hover
        const QString Red = "#EE6A5E";            // Red (close)
        const QString RedHover = "#F08075";       // Red hover
        const QString Inactive = "#333333";       // Inactive state
        
        inline QColor yellow() { return QColor(Yellow); }
        inline QColor yellowHover() { return QColor(YellowHover); }
        inline QColor green() { return QColor(Green); }
        inline QColor greenHover() { return QColor(GreenHover); }
        inline QColor red() { return QColor(Red); }
        inline QColor redHover() { return QColor(RedHover); }
        inline QColor inactive() { return QColor(Inactive); }
    }

    // ===== Search & Highlight Colors =====
    namespace Highlight {
        const QString SearchBackground = "#4F726C"; // Search highlight background
        const QString Selection = "#84E0A5";        // Selection color
        const QString SelectionText = "#2C2C2C";    // Selection text color
        
        inline QColor searchBackground() { return QColor(SearchBackground); }
        inline QColor selection() { return QColor(Selection); }
        inline QColor selectionText() { return QColor(SelectionText); }
    }

    // ===== Shadow Colors =====
    namespace Shadow {
        const QString Default = "#1F2020";        // Default shadow color
        
        inline QColor defaultColor() { return QColor(Default); }
    }

    // ===== Markdown Syntax Highlighting Colors =====
    namespace Markdown {
        const QString Header = "#A1C2B2";
        const QString HorizontalRuleFore = "#2C2C2C";
        const QString HorizontalRuleBack = "#787878";
        const QString List = "#B4C1A1";
        const QString CheckedBox = "#B7C7CE";
        const QString Link = "#ACD3A8";
        const QString ImageForeground = "#FEFFBF";
        const QString ImageBackground = "#FF8A8A";
        const QString CodeBackground = "#262626";
        const QString CodeForeground = "#cbd0d4";
        const QString Italic = "#72bafc";
        const QString Bold = "#f4d89b";
        const QString Comment = "#787878";
        const QString Quote = "#b4c1a1";
        const QString CodeKeyword = "#e8cbd0";    // red
        const QString CodeString = "#a2db95";     // green
        const QString CodeType = "#bf8cb6";       // purple
        const QString CodeOther = "#e8d0a8";      // yellow
        const QString CodeNumLiteral = "#efc3c3"; // pink
        const QString CodeBuiltIn = "#a7bec6";    // blue
        
        inline QColor header() { return QColor(Header); }
        inline QColor italic() { return QColor(Italic); }
        inline QColor bold() { return QColor(Bold); }
        inline QColor codeBackground() { return QColor(CodeBackground); }
        inline QColor codeForeground() { return QColor(CodeForeground); }
    }

    // ===== Utility Functions =====
    /**
     * Get a QColor from a hex string
     */
    inline QColor fromHex(const QString& hex) {
        return QColor(hex);
    }

    /**
     * Get a hex string from a QColor
     */
    inline QString toHex(const QColor& color) {
        return color.name();
    }
}

#endif // COLORPALETTE_H
