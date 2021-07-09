#include "imagescene.h"

ImageScene::ImageScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0,0,620,410);

    for(int i = 0; i < 10; i++)
    {
        addLine(0, i*41, 620, i *41, QPen(Qt::red));
        addLine(i*62, 0, i *62, 416, QPen(Qt::red));
    }

}

void ImageScene::loadImage(const QPixmap image){

    if(!(m_image == nullptr))
        delete m_image;

    this->clear();
    m_image = new QPixmap(image);
    *m_image = m_image->scaled(620,410,Qt::IgnoreAspectRatio,Qt::FastTransformation);
    addPixmap(*m_image);

    for(int i = 0; i < 10; i++)
    {
        addLine(0, i*41, 620, i *41, QPen(Qt::red));
        addLine(i*62, 0, i *62, 416, QPen(Qt::red));
    }
}
