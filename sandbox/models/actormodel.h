#ifndef ACTORMODEL_H
#define ACTORMODEL_H

#include <QString>

class ActorModel
{
public:
    ActorModel(const QString& name = "", bool isVisible = false);

    const QString& getName() const;
    bool isVisible() const;
    bool isSelected() const;
    void setSelected(bool selected);
private:
    QString mName;
    bool mIsVisible;
    bool mIsSelected;
};

#endif // ACTORMODEL_H
