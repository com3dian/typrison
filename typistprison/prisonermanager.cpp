#include "prisonermanager.h"

PrisonerManager::PrisonerManager(QObject *parent)
    : QObject(parent),
      wordGoal(0),
      baseWordCount(0),
      timerProgressLength(0),
      greyWordCount(0) {

    }

void PrisonerManager::setWordGoal(int wordGoal, int baseWordCount) {
    this->wordGoal = wordGoal;
    this->baseWordCount = baseWordCount;
}

void PrisonerManager::clear() {
    wordGoal = 0;
    baseWordCount = 0;
    timerProgressLength = 0;
    greyWordCount = 0;
    this->updateTypingProgress(0);
}

void PrisonerManager::updateTimerProgress(qreal timerProgressLength) {
    this->timerProgressLength = timerProgressLength;
    greyWordCount = qRound(wordGoal * timerProgressLength) + baseWordCount;
    emit updateTimerProgressInEdit(greyWordCount);
}

void PrisonerManager::updateTypingProgress(int wordCount) {
    emit updateTypingProgressInProgressBorder(wordCount - baseWordCount);
}