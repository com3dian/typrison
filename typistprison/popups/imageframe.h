#include <QFrame>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QMovie>

class ImageFrame : public QFrame {
    QPixmap pixmap;
    QMovie *movie = nullptr;

public:
    explicit ImageFrame(QWidget *parent = nullptr) : QFrame(parent) {
        setFrameStyle(QFrame::NoFrame); // we will paint the border ourselves
        setStyleSheet("background-color: transparent;"); // no border in CSS
    }

    ~ImageFrame() {
        if (movie) {
            movie->stop();
            delete movie;
        }
    }

    void setImage(const QPixmap &image) {
        if (movie) {
            movie->stop();
            delete movie;
            movie = nullptr;
        }
        pixmap = image;
        update();
    }

    void setMovie(const QString &filePath) {
        if (movie) {
            movie->stop();
            delete movie;
            movie = nullptr;
        }

        movie = new QMovie(filePath, QByteArray(), this);
        if (movie->isValid()) {
            connect(movie, &QMovie::frameChanged, this, [this](int) {
                update();
            });
            movie->start();
        } else {
            delete movie;
            movie = nullptr;
        }
        update();
    }

    void paintCheckerboard(QPainter &painter, const QRect &rect, int gridSize = 10) {
        const QColor light("#3C3C3C");
        const QColor dark("#1F2020");
        for (int y = rect.top(); y < rect.bottom(); y += gridSize) {
            for (int x = rect.left(); x < rect.right(); x += gridSize) {
                int w = std::min(gridSize, rect.right() - x + 1);
                int h = std::min(gridSize, rect.bottom() - y + 1);

                QRect square(x, y, w, h);

                const bool isDark = ((x / gridSize) + (y / gridSize)) % 2;
                painter.fillRect(square, isDark ? dark : light);
            }
        }
    }

protected:
    /*
    Handle paint event logic, paint
        1. image;
        2. checkboard background;
        2. borderline;
    +-----------------+
    |■ ■ ■ ■ ■ ■ ■ ■ ■|
    | ■ ■ ■ ■ ■ ■ ■ ■ |
    |■ ■ ■ ■ ■ ■ ■ ■ ■|
    | ■ ■ ■ ■ ■ ■ ■ ■ |
    |■ ■ ■ ■ ■ ■ ■ ■ ■|
    +-----------------+
    */
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);

        // Tunables
        const int borderW = 6;   // border thickness
        const int radius  = 6;   // border corner radius
        const int margin  = 6;   // inner margin for checkerboard/image

        // --- Content (no rounding) ---
        const QRect contentRect = rect().adjusted(margin, margin, -margin, -margin);
        paintCheckerboard(p, contentRect, 10);
        
        if (movie && movie->isValid() && movie->state() == QMovie::Running) {
            QPixmap currentFrame = movie->currentPixmap();
            if (!currentFrame.isNull()) {
                 p.drawPixmap(QRectF(contentRect), currentFrame, QRectF(currentFrame.rect()));
            }
        } else if (!pixmap.isNull()) {
            // draw scaled image exactly over the checkerboard
            p.drawPixmap(QRectF(contentRect), pixmap, QRectF(pixmap.rect()));
        }

        // --- Border (rounded), painted last ---
        QPainterPath borderPath;
        QRectF outerRect = rect();
        QRectF innerRect = outerRect.adjusted(borderW, borderW, -borderW, -borderW);

        // Outer rect with radius = 6px
        borderPath.addRoundedRect(outerRect, radius, radius);

        // Inner rect (square, no radius)
        borderPath.addRect(innerRect);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#1F2020"));
        p.drawPath(borderPath);
    }
};
