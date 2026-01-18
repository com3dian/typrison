#include "fontmanager.h"
#include <QDebug>

FontManager::FontManager() {
    loadFonts();
}

void FontManager::loadFonts() {
    int notoSansLightId = QFontDatabase::addApplicationFont(":/fonts/fonts/NotoSansSC-Light.ttf");
    int notoSansRegularId = QFontDatabase::addApplicationFont(":/fonts/fonts/NotoSansSC-Regular.ttf");
    int notoSansDemiLightId = QFontDatabase::addApplicationFont(":/fonts/fonts/NotoSansSC-DemiLight.otf");
    int notoSansMonoId = QFontDatabase::addApplicationFont(":/fonts/fonts/NotoSansMono-Regular.ttf");
    int notoSansSerifId = QFontDatabase::addApplicationFont(":/fonts/fonts/NotoSerifTC-Regular.ttf");

    // Load each font safely
    QStringList notoSansLightFamilies = QFontDatabase::applicationFontFamilies(notoSansLightId);
    notoSansLightFamily = notoSansLightFamilies.isEmpty() ? "DefaultFont" : notoSansLightFamilies.at(0);

    QStringList notoSansRegularFamilies = QFontDatabase::applicationFontFamilies(notoSansRegularId);
    notoSansRegularFamily = notoSansRegularFamilies.isEmpty() ? "DefaultFont" : notoSansRegularFamilies.at(0);

    QStringList notoSansDemiLightFamilies = QFontDatabase::applicationFontFamilies(notoSansDemiLightId);
    notoSansDemiLightFamily = notoSansDemiLightFamilies.isEmpty() ? "DefaultFont" : notoSansDemiLightFamilies.at(0);

    QStringList notoSansMonoFamilies = QFontDatabase::applicationFontFamilies(notoSansMonoId);
    notoSansMonoFamily = notoSansMonoFamilies.isEmpty() ? "DefaultFont" : notoSansMonoFamilies.at(0);

    QStringList notoSansSerifFamilies = QFontDatabase::applicationFontFamilies(notoSansSerifId);
    notoSansSerifFamily = notoSansSerifFamilies.isEmpty() ? "DefaultFont" : notoSansSerifFamilies.at(0);
}

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

QFont FontManager::getFont(const QString& fontFamily, int pointSize, QFont::Weight weight, bool italic) {
    QFont font(fontFamily);
    font.setPointSize(pointSize);
    font.setWeight(weight);
    font.setItalic(italic);
    return font;
}