#include "app/tools/line_based/line_based_view.hpp"

#include <QCursor>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

LineBasedView::LineBasedView(QWidget* parent) : QGraphicsView(parent)
{
    setScene(&scene_);
    pixmap_item_ = scene_.addPixmap(QPixmap());
    setDragMode(QGraphicsView::NoDrag);
}

void LineBasedView::setFrame(const QPixmap& frame)
{
    pixmap_item_->setPixmap(frame);
    scene_.setSceneRect(pixmap_item_->boundingRect());
}

void LineBasedView::addLine()
{
    QGraphicsPathItem* item = scene_.addPath(QPainterPath(), QPen(Qt::blue, 2));
    lines_items_.emplace_back(std::vector<QPointF> {}, item);

    std::ptrdiff_t prev_active = active_line_;
    active_line_ = static_cast<std::ptrdiff_t>(lines_items_.size()) - 1;
    updateLine(prev_active);
    updateLine(active_line_);

    emit linesUpdated(lines_items_.size(), active_line_);
}

void LineBasedView::removeLine(std::ptrdiff_t id)
{
    if (id < 0 || id >= static_cast<std::ptrdiff_t>(lines_items_.size()))
        return;

    scene_.removeItem(lines_items_[id].second);
    delete lines_items_[id].second;
    lines_items_.erase(lines_items_.begin() + id);

    active_line_ = -1;

    emit linesUpdated(lines_items_.size(), active_line_);
}

void LineBasedView::removeAllLines()
{
    for (auto& line : lines_items_)
    {
        scene_.removeItem(line.second);
        delete line.second;
    }

    lines_items_.clear();
    active_line_ = -1;
    emit linesUpdated(lines_items_.size(), active_line_);
}

void LineBasedView::setActiveLine(std::ptrdiff_t id)
{
    if (id < 0 || id >= static_cast<std::ptrdiff_t>(lines_items_.size()))
        return;

    std::ptrdiff_t prev_active = active_line_;
    active_line_ = id;
    updateLine(prev_active);
    updateLine(active_line_);
}

void LineBasedView::updateLine(std::ptrdiff_t id)
{
    if (id < 0 || id >= static_cast<std::ptrdiff_t>(lines_items_.size()))
        return;


    auto& [points, item] = lines_items_[id];

    if (points.empty())
    {
        item->setPath(QPainterPath());
        return;
    }

    QPainterPath path;
    path.moveTo(points[0]);
    for (size_t idx = 1; idx < points.size(); ++idx)
        path.lineTo(points[idx]);

    if (id == active_line_)
    {
        item->setPen(QPen(Qt::red, 2));
        path.lineTo(mapToScene(last_mouse_pos_));
    }
    else
    {
        item->setPen(QPen(Qt::blue, 2));
    }

    item->setPath(path);
}

void LineBasedView::updateAllLines()
{
    for (size_t idx = 0; idx < lines_items_.size(); ++idx)
        updateLine(idx);
}

void LineBasedView::wheelEvent(QWheelEvent* event)
{
    constexpr double zoom_step = 1.2;
    const double factor = event->angleDelta().y() > 0 ? zoom_step : 1 / zoom_step;
    scale(factor, factor);
}

void LineBasedView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        dragging_ = true;
        last_mouse_pos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton && active_line_ >= 0 &&
        active_line_ < static_cast<std::ptrdiff_t>(lines_items_.size()))
    {
        QPointF p = mapToScene(event->pos());
        lines_items_[active_line_].first.push_back(p);
        updateLine(active_line_);
        event->accept();
        return;
    }
    if (event->button() == Qt::RightButton && active_line_ >= 0 &&
        active_line_ < static_cast<std::ptrdiff_t>(lines_items_.size()))
    {
        auto& points = lines_items_[active_line_].first;
        if (!points.empty())
        {
            points.pop_back();
            updateLine(active_line_);
        }
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void LineBasedView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint delta = event->pos() - last_mouse_pos_;
    last_mouse_pos_ = event->pos();

    if (dragging_)
    {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }

    updateLine(active_line_);
    QGraphicsView::mouseMoveEvent(event);
}

void LineBasedView::mouseReleaseEvent(QMouseEvent* event)
{
    if (dragging_ && event->button() == Qt::MiddleButton)
    {
        dragging_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

std::vector<std::vector<std::array<double, 2>>> LineBasedView::get_lines()
{
    std::vector<std::vector<std::array<double, 2>>> result;

    if (!pixmap_item_)
        return result;

    for (const auto& [points, item] : lines_items_)
    {
        std::vector<std::array<double, 2>> line_pixels;
        for (const QPointF& pt : points)
        {
            line_pixels.push_back({pt.x(), pt.y()});
        }
        result.push_back(std::move(line_pixels));
    }

    return result;
}
