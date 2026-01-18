#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QFontDatabase>
#include <QString>

class FontManager
{
public:
    QString notoSansLightFamily;
    QString notoSansRegularFamily;
    QString notoSansDemiLightFamily;
    QString notoSansMonoFamily;
    QString notoSansSerifFamily;
    
    static FontManager& instance();
    
    QString getNotoSansLightFamily() const { return notoSansLightFamily; }
    QString getNotoSansRegularFamily() const { return notoSansRegularFamily; }
    QString getNotoSansDemiLightFamily() const { return notoSansDemiLightFamily; }
    QString getNotoSansMonoFamily() const { return notoSansMonoFamily; }
    QString getNotoSansSerifFamily() const { return notoSansSerifFamily; }

    QFont getFont(const QString& fontFamily, int pointSize, QFont::Weight weight, bool italic);

private:
    FontManager();
    void loadFonts();
};

#endif // FONTMANAGER_H
