#ifndef PRISONERMANAGER_H
#define PRISONERMANAGER_H

#include <QObject>
#include <QtMath>
#include <QDebug>

class PrisonerManager : public QObject {
    Q_OBJECT

public:
    PrisonerManager(QObject *parent = nullptr);
    void setWordGoal(int wordGoal, int baseWordCount);
    void updateTimerProgress(qreal timerProgressLength);
    void updateTypingProgress(int typedWordCount);
    void clear();
    bool isGoalReached();
    bool isInPrisonerMode() const;

private:
    int wordGoal;
    int baseWordCount;
    qreal timerProgressLength;
    int greyWordCount;
    int tyingWordCount;
    bool isPrisoner;
    bool goalReached;

signals:
    void updateTimerProgressInEdit(int greyWordCount);
    void updateTypingProgressInProgressBorder(int typingWordCount);
    void startPrisonerMode();
    void stopPrisonerMode();
    void prisonerModeFailed();
    void prisonerModeSucceeded();
};

#endif // PRISONERMANAGER_H