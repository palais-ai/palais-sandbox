#ifndef GENETICPROTOTYPE_H
#define GENETICPROTOTYPE_H

#include <QObject>
#include <QScriptEngine>
#include <QScriptable>
#include "Genetic.h"

void Genetic_register_prototype(QScriptEngine& engine);
QScriptValue Genetic_prototype_ctor(QScriptContext *context, QScriptEngine *engine);

class GeneticPrototype : public QObject, public QScriptable, public ailib::GeneticListener
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue fitness READ getFitnessFunction WRITE setFitnessFunction)
    Q_PROPERTY(QScriptValue mutation READ getMutationFunction WRITE setMutationFunction)
    Q_PROPERTY(QScriptValue crossover READ getCrossoverFunction WRITE setCrossoverFunction)
    Q_PROPERTY(QScriptValue generator READ getGeneratorFunction WRITE setGeneratorFunction)
    Q_PROPERTY(QScriptValue onGeneration READ getOnGenerationFunction WRITE setOnGenerationFunction)
    Q_PROPERTY(int populationSize READ getPopulationSize WRITE setPopulationSize)
public:
    GeneticPrototype();

    virtual void onGeneration(uint32_t generation, ailib::real_type fitness);

    void setFitnessFunction(QScriptValue fun);
    QScriptValue getFitnessFunction() const;
    void setCrossoverFunction(QScriptValue fun);
    QScriptValue getCrossoverFunction() const;
    void setMutationFunction(QScriptValue fun);
    QScriptValue getMutationFunction() const;
    void setGeneratorFunction(QScriptValue fun);
    QScriptValue getGeneratorFunction() const;
    void setOnGenerationFunction(QScriptValue fun);
    QScriptValue getOnGenerationFunction() const;
    void setPopulationSize(int size);
    int getPopulationSize() const;

    Q_INVOKABLE QScriptValue optimise(int numGenerations,
                                      float pElitism,
                                      float pCrossover,
                                      float pMutation);
private:
    QScriptValue mFitness, mCrossover, mMutation, mGenerator, mOnGeneration;
    int mPopulationSize;
};

#endif // GENETICPROTOTYPE_H
