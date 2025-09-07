#ifndef PRISONERMANAGER_H
#define PRISONERMANAGER_H

#include <QObject>
#include <QtMath>

class PrisonerManager : public QObject {
    Q_OBJECT

public:
    PrisonerManager(QObject *parent = nullptr);
    void setWordGoal(int wordGoal, int baseWordCount);
    void updateTimerProgress(qreal timerProgressLength);
    void updateTypingProgress(int typedWordCount);
    void clear();

private:
    int wordGoal;
    int baseWordCount;
    qreal timerProgressLength;
    int greyWordCount;

signals:
    void updateTimerProgressInEdit(int greyWordCount);
    void updateTypingProgressInProgressBorder(int typingWordCount);

};

#endif // PRISONERMANAGER_H