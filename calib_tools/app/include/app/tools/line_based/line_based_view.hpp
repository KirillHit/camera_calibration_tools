#ifndef LINE_BASED_VIEW_HPP
#define LINE_BASED_VIEW_HPP

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWheelEvent>


class LineBasedView : public QGraphicsView
{
    Q_OBJECT

public:
    LineBasedView(QWidget* parent = nullptr);

    void setFrame(const QPixmap& frame);
    std::vector<std::vector<std::array<double, 2>>> get_lines();

public slots:
    void addLine();
    void removeLine(std::ptrdiff_t id);
    void removeAllLines();
    void setActiveLine(std::ptrdiff_t id);
    void updateLine(std::ptrdiff_t id);
    void updateAllLines();

signals:
    void linesUpdated(std::size_t totalLines, std::ptrdiff_t activeLine);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QGraphicsScene scene_;
    QGraphicsPixmapItem* pixmap_item_ = nullptr;

    bool dragging_ = false;
    QPoint last_mouse_pos_;

    std::ptrdiff_t active_line_ = -1;
    std::vector<std::pair<std::vector<QPointF>, QGraphicsPathItem*>> lines_items_;
};

#endif
