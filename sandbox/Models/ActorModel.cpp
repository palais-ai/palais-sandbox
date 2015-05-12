#include "ActorModel.h"

ActorModel::ActorModel(const QString& name,
                       bool isVisible) :
    mName(name),
    mIsVisible(isVisible),
    mIsSelected(false)
{

}

const QString& ActorModel::getName() const
{
    return mName;
}

bool ActorModel::isVisible() const
{
    return mIsVisible;
}

void ActorModel::setVisible(bool visible)
{
    mIsVisible = visible;
}

bool ActorModel::isSelected() const
{
    return mIsSelected;
}

void ActorModel::setSelected(bool selected)
{
    mIsSelected = selected;
}
