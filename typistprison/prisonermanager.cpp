#include "prisonermanager.h"

PrisonerManager::PrisonerManager(QObject *parent)
    : QObject(parent),
      wordGoal(0),
      baseWordCount(0),
      timerProgressLength(0),
      greyWordCount(0),
      tyingWordCount(0),
      isPrisoner(false),
      goalReached(false) {
    }

void PrisonerManager::setWordGoal(int wordGoal, int baseWordCount) {
    this->isPrisoner = true;
    this->wordGoal = wordGoal;
    this->baseWordCount = baseWordCount;
}

void PrisonerManager::clear() {
    this->isPrisoner = false;
    this->goalReached = false;
    wordGoal = 0;
    baseWordCount = 0;
    timerProgressLength = 0;
    greyWordCount = 0;
    tyingWordCount = 0;
    this->updateTypingProgress(0);
}

void PrisonerManager::updateTimerProgress(qreal timerProgressLength) {
    this->timerProgressLength = timerProgressLength;
    greyWordCount = qRound(wordGoal * timerProgressLength) + baseWordCount;
    if (isPrisoner) {
        // if chaser reach the progress, fail the prisoner mode
        if (tyingWordCount < greyWordCount) {
            emit prisonerModeFailed();
            return;
        }
        emit updateTimerProgressInEdit(greyWordCount);
    }
}

/*
update the typing/inputing progress in prisoner mode

*/
void PrisonerManager::updateTypingProgress(int wordCount) {
    this->tyingWordCount = wordCount;
    if (isPrisoner) {
        if (wordCount < baseWordCount) {
            return;
        } else {
            emit updateTypingProgressInProgressBorder(wordCount - baseWordCount);
            if (wordCount - baseWordCount > wordGoal) {
                this->goalReached = true;
                emit prisonerModeSucceeded();
            }
        }
    }
}

bool PrisonerManager::isGoalReached() {
    return this->goalReached;
}
