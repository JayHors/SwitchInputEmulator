#include <QtGui>
#include <QPixmap>
#include <QLabel>
#include <QImage>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QLayout>
#include <QtGamepad/QGamepad>
#include "controllerwindow.h"
#include "ui_controllerwindow.h"
#include <iostream>

using std::cout;
using std::endl;

ControllerWindow::ControllerWindow(std::shared_ptr<ControllerInput> controller, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ControllerWindow),
    controller(controller)
{
    ui->setupUi(this);

    this->setSizeGripEnabled(false);

    image = std::unique_ptr<QImage>(new QImage(":/images/switch.png"));
    zl = std::unique_ptr<QImage>(new QImage(":/images/btn_ZL.png"));
    zr = std::unique_ptr<QImage>(new QImage(":/images/btn_ZR.png"));

    QImage scaledImage = image->scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaleFactor = scaledImage.width() / (double) image->width();
    QImage zlScaled = zl->scaled(zl->width() * scaleFactor, zl->height() * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QImage zrScaled = zr->scaled(zr->width() * scaleFactor, zr->height() * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image_scaled.convertFromImage(scaledImage);
    zl_scaled.convertFromImage(zlScaled);
    zr_scaled.convertFromImage(zrScaled);
    zl_mask = zl_scaled.createMaskFromColor(QColor(85, 85, 85), Qt::MaskOutColor);
    zr_mask = zr_scaled.createMaskFromColor(QColor(85, 85, 85), Qt::MaskOutColor);

    connect(&redrawTimer, SIGNAL(timeout()), this, SLOT(invalidateUi()));
    redrawTimer.start(16);
}

ControllerWindow::~ControllerWindow() {
    delete ui;
}

/*
void ControllerWindow::setSerialPortWriter(std::shared_ptr<SerialPortWriter> newWriter) {
    if (writer) {
        disconnect(writer.get(), SIGNAL(writeComplete()), this, SLOT(onUSBPacketSent()));
    }
    writer = newWriter;
}
*/

void ControllerWindow::invalidateUi() {
    this->update();
}

void ControllerWindow::drawFilledRect(QPainter &painter, const QRectF &rect) {
    QPen oldPen = painter.pen();

    QPainterPath path;
    path.addRoundedRect(rect, 10, 10);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);
    painter.fillPath(path, Qt::blue);
    painter.drawPath(path);

    painter.setPen(oldPen);
}

void ControllerWindow::drawFilledEllipse(QPainter &painter, const QPointF &center, qreal rx, qreal ry) {
    QPen oldPen = painter.pen();

    QPainterPath path;
    path.addEllipse(center, rx, ry);
    QPen pen(Qt::white, 5);
    painter.setPen(pen);
    painter.fillPath(path, Qt::blue);
    painter.drawPath(path);

    painter.setPen(oldPen);
}

void ControllerWindow::drawFilledPath(QPainter &painter, const std::vector<QPointF> &points) {
    QPen oldPen = painter.pen();

    QPainterPath path(points[0]);
    for(size_t i = 1; i < points.size(); i += 2) {
        QPointF midPoint = points[i];
        QPointF endPoint = points[i+1];
        path.cubicTo(midPoint, midPoint, endPoint);
    }
    QPen pen(Qt::white, 5);
    painter.setPen(pen);
    painter.fillPath(path, Qt::blue);
    painter.drawPath(path);

    painter.setPen(oldPen);
}

void ControllerWindow::renderDpad(QPainter &painter, Dpad_t dpad) {
    QRectF left(475, 575, 90, 90);
    QRectF right(659, 575, 90, 90);
    QRectF up(567, 483, 90, 90);
    QRectF down(567, 667, 90, 90);

    switch (dpad) {
    case DPAD_UP:
    case DPAD_UP_LEFT:
    case DPAD_UP_RIGHT:
        drawFilledRect(painter, up);
        break;
    case DPAD_DOWN:
    case DPAD_DOWN_LEFT:
    case DPAD_DOWN_RIGHT:
        drawFilledRect(painter, down);
        break;
    default:
        break;
    }

    switch (dpad) {
    case DPAD_LEFT:
    case DPAD_UP_LEFT:
    case DPAD_DOWN_LEFT:
        drawFilledRect(painter, left);
        break;
    case DPAD_RIGHT:
    case DPAD_UP_RIGHT:
    case DPAD_DOWN_RIGHT:
        drawFilledRect(painter, right);
        break;
    default:
        break;
    }
}

void ControllerWindow::renderButtons(QPainter &painter, const Button_t buttons) {
    if (buttons & BUTTON_Y)
        drawFilledEllipse(painter, QPointF(1220, 373), 60, 60);
    if (buttons & BUTTON_A)
        drawFilledEllipse(painter, QPointF(1500, 373), 60, 60);
    if (buttons & BUTTON_X)
        drawFilledEllipse(painter, QPointF(1360, 252), 60, 60);
    if (buttons & BUTTON_B)
        drawFilledEllipse(painter, QPointF(1360, 496), 60, 60);

    if (buttons & BUTTON_HOME)
        drawFilledEllipse(painter, QPointF(1014, 373), 40, 40);
    if (buttons & BUTTON_CAPTURE)
        drawFilledRect(painter, QRectF(728, 341, 64, 64));

    if (buttons & BUTTON_PLUS)
        drawFilledEllipse(painter, QPointF(1106, 240), 36, 36);
    if (buttons & BUTTON_MINUS)
        drawFilledEllipse(painter, QPointF(666, 240), 36, 36);

    if (buttons & BUTTON_L) {
        std::vector<QPointF> pointsL;
        pointsL.push_back(QPointF(634, 39));
        pointsL.push_back(QPointF(372, 55));
        pointsL.push_back(QPointF(195, 144));
        pointsL.push_back(QPointF(265, 34));
        pointsL.push_back(QPointF(424, 11));
        pointsL.push_back(QPointF(540, 0));
        pointsL.push_back(QPointF(634, 39));
        drawFilledPath(painter, pointsL);
    }

    if (buttons & BUTTON_R) {
        std::vector<QPointF> pointsR;
        pointsR.push_back(QPointF(1144, 39));
        pointsR.push_back(QPointF(1406, 55));
        pointsR.push_back(QPointF(1583, 144));
        pointsR.push_back(QPointF(1513, 34));
        pointsR.push_back(QPointF(1354, 11));
        pointsR.push_back(QPointF(1238, 0));
        pointsR.push_back(QPointF(1144, 39));
        drawFilledPath(painter, pointsR);
    }

    if (buttons & BUTTON_ZL)
        drawFilledRect(painter, QRectF(602, 943, 120, 120));
    if (buttons & BUTTON_ZR)
        drawFilledRect(painter, QRectF(1055, 943, 120, 120));

    if (buttons & BUTTON_LCLICK)
        drawFilledEllipse(painter, QPointF(395, 373), 80, 80);
    if (buttons & BUTTON_RCLICK)
        drawFilledEllipse(painter, QPointF(1123, 620), 80, 80);
}

void ControllerWindow::renderLeftStick(QPainter &painter, const quint8 lx, const quint8 ly) {
    if (lx == STICK_CENTER && ly == STICK_CENTER) return;

    int cx = 395 - (128 - lx);
    int cy = 373 - (128 - ly);
    drawFilledEllipse(painter, QPointF(cx, cy), 20, 20);
    QPen oldPen = painter.pen();
    QPen pen(Qt::white, 5);
    painter.setPen(pen);
    painter.drawLine(QPointF(395, 373), QPointF(cx, cy));
    painter.setPen(oldPen);
}

void ControllerWindow::renderRightStick(QPainter &painter, const quint8 rx, const quint8 ry) {
    if (rx == STICK_CENTER && ry == STICK_CENTER) return;

    int cx = 1123 - (128 - rx);
    int cy = 620 - (128 - ry);
    drawFilledEllipse(painter, QPointF(cx, cy), 20, 20);
    QPen oldPen = painter.pen();
    QPen pen(Qt::white, 5);
    painter.setPen(pen);
    painter.drawLine(QPointF(1123, 620), QPointF(cx, cy));
    painter.setPen(oldPen);
}

void ControllerWindow::paintEvent(QPaintEvent *) {

    QPainter painter;
    painter.begin(this);
    QFlags<QPainter::RenderHint> hints;
    hints.setFlag(QPainter::Antialiasing);
    hints.setFlag(QPainter::TextAntialiasing);
    hints.setFlag(QPainter::SmoothPixmapTransform);
    painter.setRenderHints(hints);
    painter.fillRect(0, 0, this->width(), this->height(), Qt::green);
    painter.drawPixmap((this->width() - image_scaled.width()) / 2.0, (this->height() - image_scaled.height()) / 2.0, image_scaled);
    painter.setPen(Qt::white);
    painter.drawPixmap((this->width() / 2.0) - (2 * zl_scaled.width()), 0.5 * this->height() + 0.25 * image_scaled.height(), zl_mask);
    painter.drawPixmap((this->width() / 2.0) + zr_scaled.width(), 0.5 * this->height() + 0.25 * image_scaled.height(), zr_mask);
    double translateX = ((this->width() - image_scaled.width()) / 2.0);
    double translateY = ((this->height() - image_scaled.height()) / 2.0);
    painter.translate(translateX, translateY);
    painter.scale(scaleFactor, scaleFactor);
    painter.setPen(Qt::NoPen);

    quint8 lx;
    quint8 ly;
    quint8 rx;
    quint8 ry;
    Dpad_t dpad;
    Button_t button;
    quint8 vendorSpec;
    controller.get()->getState(&lx, &ly, &rx, &ry, &dpad, &button, &vendorSpec);

    renderDpad(painter, dpad);
    renderButtons(painter, button);
    renderLeftStick(painter, lx, ly);
    renderRightStick(painter, rx, ry);

    painter.end();
}

QSize ControllerWindow::minimumSizeHint() const {
    return QSize(886, 616);
    //return QSize(888, 620);
}

QSize ControllerWindow::sizeHint() const {
    return QSize(886, 616);
    //return QSize(888, 620);
}

QSize ControllerWindow::maximumSizeHint() const {
    return QSize(1772, 1232);
    //return QSize(1774, 1238);
}

void ControllerWindow::closeEvent(QCloseEvent *event) {
    emit controllerWindowClosing();
    event->accept();
}
