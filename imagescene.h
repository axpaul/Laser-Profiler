#ifndef IMAGESCENE_H
#define IMAGESCENE_H

#include <QGraphicsScene>
#include <QObject>
#include <QWidget>
#include <QPixmap>

class ImageScene : public QGraphicsScene
{
public:
    ImageScene(QObject * parent = Q_NULLPTR);
    //~ImageScene();

    void loadImage(bool showGrid, const QPixmap image);

private:
    QPixmap *m_image = nullptr;

};

#endif // IMAGESCENE_H
