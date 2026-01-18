#ifndef PRISONERDIALOG_H
#define PRISONERDIALOG_H

#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QSplitter>
#include <QLineEdit>
#include <QWidget>
#include <QStyle>
#include <QPainter>
#include <QKeyEvent>
#include <QSplitterHandle>
#include <QPainterPath>
#include <QRegularExpression>
#include <QSlider>
#include <QTimer>
#include <QPushButton>
#include <QTextDocument>

#include "utilsprisonerdialog.h"
#include "../utils/hoverbutton.h"
#include "../utils/closebuttonwidget.h"

class PrisonerDialog : public QDialog {
    Q_OBJECT
signals:
    void prisonerSettings(int wordGoal, int timeLimit);

public:
    PrisonerDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Prisoner Dialog");
        
        // Set window flags to make it frameless and borderless
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        
        // Create a container widget for rounded corners
        QWidget *containerWidget = new QWidget(this);
        containerWidget->setObjectName("containerWidget");
        containerWidget->setStyleSheet(R"(
            #containerWidget {
                background-color: #1F2020;
                border: 1px solid #393B3B;
                border-radius: 9px;
            }
            QWidget {
                background-color: #1F2020;
            }
        )");

        // Main layout for the dialog
        QVBoxLayout *dialogLayout = new QVBoxLayout(this);
        dialogLayout->setContentsMargins(0, 0, 0, 0);
        dialogLayout->addWidget(containerWidget);

        // Main layout for the container
        QVBoxLayout *mainLayout = new QVBoxLayout(containerWidget);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(2);

        // Close Button
        CloseButtonWidget *closeButtonWidget = new CloseButtonWidget(containerWidget);
        connect(closeButtonWidget, &CloseButtonWidget::closeButtonClicked, this, [this]() {
            reject();
            close();
        });

        // Add close button to main layout with right alignment
        QHBoxLayout *topLayout = new QHBoxLayout();
        topLayout->setContentsMargins(0, 0, 0, 0);
        topLayout->addStretch();
        topLayout->addWidget(closeButtonWidget);
        mainLayout->addLayout(topLayout);

        // Door Icon
        QLabel *iconLabel = new QLabel(this);
        // Calculate size based on device pixel ratio
        qreal dpr = devicePixelRatio();
        int scaledWidth = 42 * dpr;
        int scaledHeight = 35 * dpr;
        iconLabel->setFixedSize(42, 35);

        QPixmap iconPixmap(":/icons/escape_icon.png");
        // Scale the pixmap at a higher resolution
        QPixmap scaledPixmap = iconPixmap.scaled(scaledWidth, scaledHeight, 
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // Set the device pixel ratio
        scaledPixmap.setDevicePixelRatio(dpr);
        iconLabel->setPixmap(scaledPixmap);
        iconLabel->setAlignment(Qt::AlignCenter);
        
        // Create a horizontal layout for centering the icon
        QHBoxLayout *iconLayout = new QHBoxLayout();
        iconLayout->addStretch();
        iconLayout->addWidget(iconLabel);
        iconLayout->addStretch();
        mainLayout->addLayout(iconLayout);

        // Labels 🏷
        QLabel *label1 = new QLabel("Escape", this);
        QLabel *label2 = new QLabel("Ready? Set Your Goal & Timer.", this);
        
        // Set style for labels
        label1->setStyleSheet("color: white; font-size: 24px;");
        label2->setStyleSheet("color: #BDBDBD;");
        
        label1->setAlignment(Qt::AlignCenter);
        label2->setAlignment(Qt::AlignCenter);

        QLabel *labelSplitter = new QLabel("Goal", this);
        labelSplitter->setStyleSheet("color: #BDBDBD;");
        labelSplitter->setAlignment(Qt::AlignLeft);

        mainLayout->addWidget(label1);
        mainLayout->addWidget(label2);
        mainLayout->addSpacing(32);
        mainLayout->addWidget(labelSplitter);

        // Box for Splitter
        QWidget *splitterBox = new RoundedWidget(this);
        splitterBox->setStyleSheet("border-radius: 6px; background-color: #2C2C2C;");
        QVBoxLayout *splitterLayout = new QVBoxLayout(splitterBox);
        splitterLayout->setContentsMargins(2, 2, 2, 2); // Margin inside the box

        // Custom Splitter with Custom Handle
        splitter = new CustomSplitter(Qt::Horizontal, splitterBox);
        splitter->setHandleWidth(12); // Set the handle width

        // Container for TextEdit and Label
        leftContainer = new QWidget(this);
        leftContainer->setStyleSheet( "background-color: #1F2020; "
                                     "color: white; "
                                     "border: none; "
                                     "border-top-left-radius: 4px; "
                                     "border-bottom-left-radius: 4px; "
                                     "border-top-right-radius: 0px; "
                                     "border-bottom-right-radius: 0px;");

        QVBoxLayout *leftLayout = new QVBoxLayout(leftContainer);
        leftLayout->setContentsMargins(16, 20, 16, 12);
        leftLayout->setSpacing(0);

        // TextEdit
        lineEdit = new QLineEdit(this);
        lineEdit->setStyleSheet("background-color: transparent; "
                                "color: white; "
                                "padding: 0px; "
                                "margin: -2px; "
                                "border: none; "
                                "font-size: 40px;"); // Adjust font size

        lineEdit->setAlignment(Qt::AlignLeft | Qt::AlignVCenter); // Align text to left and vertically center
        lineEdit->setMinimumWidth(84);
        lineEdit->installEventFilter(this);

        leftLayout->addWidget(lineEdit);

        // Label beneath TextEdit
        QLabel *textLabel = new QLabel(" Words", this);
        textLabel->setAlignment(Qt::AlignLeft);
        textLabel->setStyleSheet("color: #5A5A5A; ");
        leftLayout->addWidget(textLabel);

        // Right widget
        QWidget *rightWidget = new QWidget(this);

        // Add to splitter
        splitter->addWidget(leftContainer);
        splitter->addWidget(rightWidget);
        splitter->setStretchFactor(0, 0); // Left part doesn't shrink
        splitter->setStretchFactor(1, 1); // Right part takes remaining space
        splitter->setCollapsible(0, false); // Make left part non-collapsible
        QList<int> initialSizes = splitter->sizes();

        splitterLayout->addWidget(splitter);
        splitterBox->setLayout(splitterLayout);
        splitterBox->setFixedHeight(96);

        // Label under the splitter
        QLabel *infoLabel = new QLabel("Timer", this);
        infoLabel->setStyleSheet("color: #BDBDBD;");
        infoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        infoLabel->setAlignment(Qt::AlignLeft);

        // Slider container
        QWidget *sliderContainer = new QWidget(this);
        sliderContainer->setStyleSheet("border-radius: 6px; background-color: #2C2C2C;");
        QVBoxLayout *sliderLayout = new QVBoxLayout(sliderContainer);
        sliderLayout->setContentsMargins(0, 48, 0, 5);
        sliderLayout->setSpacing(0);

        // Value labels under the slider
        QHBoxLayout *valueLabelsLayout = new QHBoxLayout();
        valueLabelsLayout->setContentsMargins(0, 0, 0, 0);
        QStringList values = {"0", "15", "50", "150", "∞"};
        
        for (int i = 0; i < values.size(); ++i) {
            if (i > 0) {
                valueLabelsLayout->addStretch();
            }
            
            QLabel *valLabel = new QLabel(values[i], this);
            valLabel->setAlignment(Qt::AlignCenter);
            valLabel->setStyleSheet("font-size: 12px; color: #666;");
            valueLabelsLayout->addWidget(valLabel);
        }

        // Custom Slider
        slider = new QSlider(Qt::Horizontal, sliderContainer);
        slider->setRange(0, 100);
        slider->setTickInterval(10);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setStyleSheet(R"(
            QSlider::groove:horizontal {
                height: 2px;
                background: #5A5A5A;
            }
            QSlider::handle:horizontal {
                background: #D9D9D9;
                width: 12px; height: 12px;
                border-radius: 6px;
                margin: -5px 0;
            }
            QSlider::sub-page:horizontal {
                background: #D9D9D9;
                height: 2px;
            }
        )");

        // Value label above the slider
        QLabel *sliderValueLabel = new QLabel(this);
        sliderValueLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        // sliderValueLabel->setFixedWidth(72);
        sliderValueLabel->setAlignment(Qt::AlignLeft);
        sliderValueLabel->setStyleSheet("background: transparent; padding: 0px;");
        sliderValueLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

        // Update label dynamically
        connect(slider, &QSlider::valueChanged, this, [=](int value) {
            double ratio = value / 100.0;  // Convert to ratio between 0 and 1
            if (ratio <= 0.0) {
                actionButton->setEnabled(false);
                actionButton->setIcons(QIcon(":/icons/empty_icon.png"), QIcon(":/icons/empty_icon.png"));
                actionButton->setSilentBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                actionButton->setHoverBehavior(R"(
                    QPushButton {
                        background-color: transparent;
                        border: 1px solid #333333;
                        color: #5A5A5A;
                        border-radius: 4px;
                        padding: 4px 8px;
                    }
                )");
            } else {
                QString numericText = lineEdit->text().remove(QRegularExpression("[^0-9]"));
                if (numericText.isEmpty()) {
                    actionButton->setEnabled(false);
                } else {
                    actionButton->setEnabled(true);
                    actionButton->setIcons(QIcon(":/icons/right_arrow.png"), QIcon(":/icons/right_arrow_hover.png"));
                    actionButton->setSilentBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #5A5A5A;
                            color: #BDBDBD;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                    actionButton->setHoverBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #999999;
                            color: #DEDEDE;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                }
            }
            if (ratio == 1.0) {
                sliderValueLabel->setText(QString("<span style='font-size: 24px; color: white;'>%1</span>")
                                  .arg("Unlimited"));
            } else {
                int mappedValue = mapToTime(ratio);
                sliderValueLabel->setText(QString("<span style='font-size: 24px; color: white;'>%1</span>"
                                               "<span style='font-size: 12px; color: #5A5A5A;'>mins</span>")
                                  .arg(mappedValue));
            }
            
            // Get global position of the slider
            QPoint sliderPos = slider->mapToGlobal(QPoint(0, 0));
            QPoint dialogPos = this->mapFromGlobal(sliderPos);
            sliderValueLabel->adjustSize();
            int textWidth = sliderValueLabel->width();
            
            // Calculate handle position
            const int handleWidth = 16;
            int handlePosition = ((float)value / slider->maximum()) * (slider->width() - handleWidth);
            
            // Position label above handle
            int labelX = dialogPos.x() + handlePosition - (textWidth / 2) + handleWidth/2;
            int labelY = dialogPos.y() - sliderValueLabel->height() - 5;
            
            // Ensure label stays within container bounds with proper min/max check
            int minX = dialogPos.x();
            int maxX = dialogPos.x() + slider->width() - textWidth;
            
            // Add safety check to prevent min > max assertion
            if (maxX < minX) {
                maxX = minX;
            }
            
            labelX = qBound(minX, labelX, maxX);
            
            sliderValueLabel->move(labelX, labelY);
        });

        // Add widgets to layout (but not the value label)
        sliderLayout->addWidget(slider);
        sliderLayout->addLayout(valueLabelsLayout);

        // Initial position update
        QTimer::singleShot(0, [=]() {
            slider->valueChanged(slider->value());
        });

        // Set main layout
        mainLayout->addWidget(splitterBox);
        mainLayout->addSpacing(16);
        // Add widgets to main layout
        mainLayout->addWidget(infoLabel);
        mainLayout->addWidget(sliderContainer);

        // Create button container for right alignment
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();  // Push button to the right

        // Create hover icon button with icon and text
        actionButton = new HoverButton("Launch ", this);
        actionButton->setEnabled(false);
        actionButton->setIcons(QIcon(":/icons/empty_icon.png"), QIcon(":/icons/empty_icon.png"));
        actionButton->setSilentBehavior(R"(
            QPushButton {
                background-color: transparent;
                border: 1px solid #333333;
                color: #5A5A5A;
                border-radius: 4px;
                padding: 4px 8px;
            }
        )");
        actionButton->setHoverBehavior(R"(
            QPushButton {
                background-color: transparent;
                border: 1px solid #333333;
                color: #5A5A5A;
                border-radius: 4px;
                padding: 4px 8px;
            }
        )");
        actionButton->setLayoutDirection(Qt::RightToLeft);

        connect(actionButton, &QPushButton::clicked, this, [=]() {
            // Get word goal from lineEdit
            QString numericText = lineEdit->text().remove(QRegularExpression("[^0-9]"));
            int wordGoal = numericText.toInt();
            
            // Get time limit from slider
            double ratio = slider->value() / 100.0;
            int timeLimit = (ratio == 1.0) ? -1 : mapToTime(ratio);  // -1 for unlimited
            
            emit prisonerSettings(wordGoal, timeLimit);
            accept();
        });
        connect(actionButton, &QPushButton::clicked, this, &QDialog::accept);
        
        mainLayout->addSpacing(32);

        buttonLayout->addWidget(actionButton);
        mainLayout->addLayout(buttonLayout);
        setLayout(mainLayout);
        adjustSplitter(mapToRange(100));
        connect(splitter, &QSplitter::splitterMoved, this, &PrisonerDialog::onSplitterMoved);
        
        // Make dialog non-resizable by fixing its size
        setFixedSize(sizeHint());
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        // Handle existing line edit events
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            int key = keyEvent->key();
            QString text = keyEvent->text(); // Get the actual input text

            // ✅ Allow navigation keys and backspace
            if (key == Qt::Key_Left || key == Qt::Key_Right ) {
                return false; // Allow default handling
            }

            // Extract numeric text
            QString numericText = lineEdit->text().remove(QRegularExpression("[^0-9]"));
            int numericValue = numericText.toInt();

            // Handle Backspace and Delete keys
            if (key == Qt::Key_Backspace || key == Qt::Key_Delete) {
                QString currentText = lineEdit->text();

                if (key == Qt::Key_Backspace) {
                    // Remove the last character (backspace effect)
                    currentText.chop(1);
                } else if (key == Qt::Key_Delete) {
                    // Clear the selection or delete one character
                    int cursorPos = lineEdit->cursorPosition();
                    if (cursorPos < currentText.length()) {
                        currentText.remove(cursorPos, 1);
                    }
                }

                numericText = currentText.remove(QRegularExpression("[^0-9]"));
                numericValue = numericText.toInt();

                // Update text field
                lineEdit->setText(numericText);

                // Map and update splitter
                double mappedValue = mapToRange(numericValue);
                adjustSplitter(mappedValue);

                // Change visibility of actionbutton ⏯️
                if (numericText.isEmpty()) {
                    actionButton->setEnabled(false);
                    actionButton->setIcons(QIcon(":/icons/empty_icon.png"), QIcon(":/icons/empty_icon.png"));
                    actionButton->setSilentBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                    actionButton->setHoverBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                } else if (numericValue < 100) {
                    actionButton->setEnabled(false);
                    actionButton->setIcons(QIcon(":/icons/empty_icon.png"), QIcon(":/icons/empty_icon.png"));
                    actionButton->setSilentBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                    actionButton->setHoverBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                }
                return true;
            }

            // Check if input is strictly numeric (0-9)
            // ⬆️ key pressed
            if (key == Qt::Key_Up) {
                numericValue += 1; // Increase value
                lineEdit->setText(QString::number(numericValue));
            // ⬇️ key pressed
            } else if (key == Qt::Key_Down) {
                if (numericValue > 100) {
                    numericValue -= 1; // Decrease value (not below 100)
                    lineEdit->setText(QString::number(numericValue));
                }
            // 🔢 number key pressed
            } else if (!text.isEmpty() && text.at(0).isDigit()) {
                lineEdit->insert(text);

                // Adjust the splitter position dynamically
                numericText = lineEdit->text().remove(QRegularExpression("[^0-9]"));
                numericValue = numericText.toInt();

            }
            // Map numeric value to a range
            double mappedValue = mapToRange(numericValue);

            // Adjust splitter size dynamically
            adjustSplitter(mappedValue);

            // Change visibility of actionbutton ⏯️
            if (numericText.isEmpty()) {
                actionButton->setEnabled(false);
                actionButton->setIcons(QIcon(":/icons/empty_icon.png"), QIcon(":/icons/empty_icon.png"));
                actionButton->setSilentBehavior(R"(
                    QPushButton {
                        background-color: transparent;
                        border: 1px solid #333333;
                        color: #5A5A5A;
                        border-radius: 4px;
                        padding: 4px 8px;
                    }
                )");
                actionButton->setHoverBehavior(R"(
                    QPushButton {
                        background-color: transparent;
                        border: 1px solid #333333;
                        color: #5A5A5A;
                        border-radius: 4px;
                        padding: 4px 8px;
                    }
                )");
            } else if (numericValue < 100) {
                actionButton->setEnabled(false);
                actionButton->setIcons(QIcon(":/icons/empty_icon.png"), QIcon(":/icons/empty_icon.png"));
                actionButton->setSilentBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                    actionButton->setHoverBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #333333;
                            color: #5A5A5A;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");

            } else if (!numericText.isEmpty()) {
                if (numericValue >= 100) {
                    actionButton->setEnabled(true);
                    actionButton->setIcons(QIcon(":/icons/right_arrow.png"), QIcon(":/icons/right_arrow_hover.png"));
                    actionButton->setSilentBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #5A5A5A;
                            color: #BDBDBD;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                    actionButton->setHoverBehavior(R"(
                        QPushButton {
                            background-color: transparent;
                            border: 1px solid #999999;
                            color: #DEDEDE;
                            border-radius: 4px;
                            padding: 4px 8px;
                        }
                    )");
                }
            }

            return true; // Ignore non-numeric input
        }

        return QDialog::eventFilter(obj, event);
    }

private:
    QLineEdit *lineEdit;
    CustomSplitter *splitter;
    QWidget *leftContainer;
    QSlider *slider;
    HoverButton *actionButton;

    /**
     * Maps a word count value to a normalized range [0, 1) for splitter positioning.
     * 
     * This function creates a non-linear mapping that:
     * - Returns 0.0 for word counts <= 100 (minimum threshold)
     * - For word counts > 100, applies a sigmoid-like curve that approaches but never reaches 1.0
     * - The curve provides more granular control for lower word counts and compresses higher values
     * 
     * @param x The input word count value
     * @param k Controls the steepness of the curve (default: 100)
     * @return A value in range [0, 1) representing the relative position for UI elements
     */
    double mapToRange(double x, double k = 100) {
        if (x <= 100) {
            return 0.0;
        }
        double y = (x - 100) / 10;
        return ((y + 100) / (y + k + 100) - 0.5) * 2;
    }

    int reverseMapFromRange(double spliterRatio, int minLeftWidth = 116, int k = 100) {
        // Reverse the mapping function
        int y = 200 * spliterRatio / (1 - spliterRatio);
        return y * 10 + 100;  // Reverse the mapping logic
    }

    int mapToTime(double sliderValue) {
        int y = 50 * sliderValue / (1 - sliderValue);

        if (y <= 30) {
            return y;  // Return original value if <= 30
        } else if (y <= 90) {
            return ((y + 2) / 5) * 5;  // Round to nearest multiple of 5
        } else {
            return ((y + 5) / 10) * 10;  // Round to nearest multiple of 10
        }
    }

private slots:
    void adjustSplitter(double rangeValue) {
        QList<int> sizes = splitter->sizes();
        int minLeftWidth = 116;
        int totalWidth = sizes[0] + sizes[1];  // Total width of splitter
        int maxLeftWidth = totalWidth;  // Limit max left width to 70% of total width

        // Calculate new left size using range mapping
        int newLeftSize = minLeftWidth + (maxLeftWidth - minLeftWidth) * rangeValue;
        int newRightSize = totalWidth - newLeftSize;

        splitter->setSizes({newLeftSize, newRightSize});
        double sliderRatio = static_cast<double>(newLeftSize) / totalWidth;
        slider->setValue(sliderRatio * 100);
    }

    void onSplitterMoved(int pos, int index) {
        Q_UNUSED(index);
        int minLeftWidth = 116;

        // Get current width of the left container
        QList<int> sizes = splitter->sizes();
        int totalWidth = sizes[0] + sizes[1];  // Total width of splitter

        double spliterRatio = static_cast<double>(sizes[0] - minLeftWidth) / (totalWidth - minLeftWidth);
        if (spliterRatio == 1.0) {
            lineEdit->setText("+∞");
            slider->setValue(100);            // Set slider to maximum
            slider->setEnabled(false);        // Disable slider interaction
            return;
        }
        double sliderRatio = static_cast<double>(sizes[0]) / totalWidth;
        slider->setValue(sliderRatio * 100);  // Set slider value
        slider->setEnabled(true);             // Enable slider interaction

        // Reverse map to get the numeric value from width
        int numericValue = reverseMapFromRange(spliterRatio);

        // Update the QLineEdit content
        lineEdit->setText(QString::number(numericValue));

        // Change visibility of actionbutton ⏯️
        if (numericValue >= 100) {
            actionButton->setEnabled(true);
            actionButton->setIcons(QIcon(":/icons/right_arrow.png"), QIcon(":/icons/right_arrow_hover.png"));
            actionButton->setSilentBehavior(R"(
                QPushButton {
                    background-color: transparent;
                    border: 1px solid #5A5A5A;
                    color: #BDBDBD;
                    border-radius: 4px;
                    padding: 4px 8px;
                }
            )");
            actionButton->setHoverBehavior(R"(
                QPushButton {
                    background-color: transparent;
                    border: 1px solid #999999;
                    color: #DEDEDE;
                    border-radius: 4px;
                    padding: 4px 8px;
                }
            )");
        }
    }

    void enterButtonEvent() {
        
    }
};
#endif // PRISONERDIALOG_H