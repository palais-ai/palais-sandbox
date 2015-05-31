#include "GeneticPrototype.h"
#include <QDebug>
#include "Bindings/JavascriptBindings.h"

Q_DECLARE_METATYPE(GeneticPrototype*)

void Genetic_register_prototype(QScriptEngine& engine)
{
    const int typeId = qRegisterMetaType<GeneticPrototype*>("GeneticPrototype*");
    QScriptValue prototype = engine.newQObject((GeneticPrototype*)0);
    engine.setDefaultPrototype(typeId, prototype);

    engine.globalObject().setProperty("Genetic",
                                      engine.newFunction(Genetic_prototype_ctor));
}

QScriptValue Genetic_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    return engine->newQObject(new GeneticPrototype, QScriptEngine::ScriptOwnership);;
}

GeneticPrototype::GeneticPrototype() :
    mPopulationSize(0)
{
    ;
}

void GeneticPrototype::onGeneration(uint32_t generation, ailib::real_type fitness)
{
    QScriptValue fn = getGeneratorFunction();
    if(fn.isFunction())
    {
        fn.call(QScriptValue(), QScriptValueList() << generation << fitness);
        JavaScriptBindings::checkScriptEngineException(*engine(),
                                                       "Genetic.onGeneration");
    }
}

void GeneticPrototype::setFitnessFunction(QScriptValue fun)
{
    mFitness = fun;
}

QScriptValue GeneticPrototype::getFitnessFunction() const
{
    return mFitness;
}

void GeneticPrototype::setCrossoverFunction(QScriptValue fun)
{
    mCrossover = fun;
}

QScriptValue GeneticPrototype::getCrossoverFunction() const
{
    return mCrossover;
}

void GeneticPrototype::setMutationFunction(QScriptValue fun)
{
    mMutation = fun;
}

QScriptValue GeneticPrototype::getMutationFunction() const
{
    return mMutation;
}

void GeneticPrototype::setGeneratorFunction(QScriptValue fun)
{
    mGenerator = fun;
}

QScriptValue GeneticPrototype::getGeneratorFunction() const
{
    return mGenerator;
}

void GeneticPrototype::setOnGenerationFunction(QScriptValue fun)
{
    mOnGeneration = fun;
}

QScriptValue GeneticPrototype::getOnGenerationFunction() const
{
    return mOnGeneration;
}

void GeneticPrototype::setPopulationSize(int size)
{
    mPopulationSize = size;
}

int GeneticPrototype::getPopulationSize() const
{
    return mPopulationSize;
}

GeneticPrototype* gCurrentGeneticPrototype = NULL;
static ailib::real_type fitness(const QScriptValue& c)
{
    if(gCurrentGeneticPrototype)
    {
        QScriptValue fn = gCurrentGeneticPrototype->getFitnessFunction();
        if(fn.isFunction())
        {
            QScriptValue val = fn.call(QScriptValue(),
                           QScriptValueList() << c);
            JavaScriptBindings::checkScriptEngineException(*gCurrentGeneticPrototype->engine(),
                                                           "Genetic.crossover");
            return val.toNumber();
        }
        else
        {
            qWarning() << "GeneticPrototype.fitness: 'fitness' property must be a function.";
        }
    }

    return 0;
}

static QScriptValue crossover(const QScriptValue& l, const QScriptValue& r)
{
    if(gCurrentGeneticPrototype)
    {
        QScriptValue fn = gCurrentGeneticPrototype->getCrossoverFunction();
        if(fn.isFunction())
        {
            QScriptValue val = fn.call(QScriptValue(),
                           QScriptValueList() << l << r);
            JavaScriptBindings::checkScriptEngineException(*gCurrentGeneticPrototype->engine(),
                                                           "Genetic.crossover");
            return val;
        }
        else
        {
            qWarning() << "GeneticPrototype.crossover: 'crossover' property must be a function.";
        }
    }

    return l;
}

static QScriptValue mutate(const QScriptValue& c)
{
    if(gCurrentGeneticPrototype)
    {
        QScriptValue fn = gCurrentGeneticPrototype->getMutationFunction();
        if(fn.isFunction())
        {
            QScriptValue val = fn.call(QScriptValue(),
                                       QScriptValueList() << c);
            JavaScriptBindings::checkScriptEngineException(*gCurrentGeneticPrototype->engine(),
                                                           "Genetic.mutate");
            return val;
        }
        else
        {
            qWarning() << "GeneticPrototype.mutate: 'mutation' property must be a function.";
        }
    }

    return c;
}

static QScriptValue generate(uint32_t idx)
{
    if(gCurrentGeneticPrototype)
    {
        QScriptValue fn = gCurrentGeneticPrototype->getGeneratorFunction();
        if(fn.isFunction())
        {
            QScriptValue val = fn.call(QScriptValue(),
                           QScriptValueList() << idx);
            JavaScriptBindings::checkScriptEngineException(*gCurrentGeneticPrototype->engine(),
                                                           "Genetic.generate");
            return val;
        }
        else
        {
            qWarning() << "GeneticPrototype.generate: 'generator' property must be a function.";
        }
    }

    return idx;
}

struct GeneticHash
{
    uint64_t operator()(QScriptValue state) const
    {
        return 0;
    }
};

QScriptValue GeneticPrototype::optimise(int numGenerations,
                                        float pElitism,
                                        float pCrossover,
                                        float pMutation)
{
    if(mPopulationSize <= 0)
    {
        qWarning() << "GeneticPrototype.optimise: Population size was smaller or equa to 0. "
                      "Doing nothing.";
        return engine()->undefinedValue();
    }

    gCurrentGeneticPrototype = this;
    ailib::Genetic<QScriptValue, GeneticHash> ga(&fitness, &crossover, &mutate, &generate, mPopulationSize);
    ga.setListener(this);
    ga.generatePopulation();
    QScriptValue result = ga.optimise(numGenerations, pElitism, pCrossover, pMutation);

    gCurrentGeneticPrototype = NULL;
    return result;
}
