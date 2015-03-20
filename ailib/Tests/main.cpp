#include "Scheduler.h"
#include "BehaviorTree.h"
#include "Blackboard.h"

#include <stdlib.h>
#include <string>
#include <iostream>

using namespace ailib;

class CanPrintDecorator;

static CanPrintDecorator* mainlock = 0;

class CanPrintDecorator : public Decorator
{
public:
    CanPrintDecorator(Scheduler& scheduler, Behavior* child) :
        Decorator(scheduler, child),
        mGuard(false)
    {
        ;
    }

    void lock()
    {
        mGuard = false;
    }

    void unlock()
    {
        mGuard = true;
    }

    virtual void run()
    {
        if(mGuard)
        {
            scheduleBehavior();
            setStatus(StatusWaiting);
        }
        else
        {
            notifyFailure();
        }
    }

    virtual void terminate()
    {
        Decorator::terminate();
    }

    virtual void onSuccess(Behavior* behavior)
    {
        UNUSED(behavior);
        notifySuccess();
    }

    virtual void onFailure(Behavior* behavior)
    {
        UNUSED(behavior);
        notifyFailure();
    }

private:
    bool mGuard;
};

class PrintBehavior : public Behavior
{
public:
    PrintBehavior(const std::string& msg) :
        mMsg(msg),
        i(100000000)
    {
        ;
    }

    virtual void run()
    {
        static const int64_t start = 100000000;
        int64_t begin = i;
        while(--i > 0)
        {
            if(begin - i == start / 25)
            {
                return;
            }

            if(i == 5000000 && mainlock != 0)
            {
                //mainlock->lock();
            }
        }

        std::cout << mMsg << std::endl;
        notifySuccess();
    }

private:
    std::string mMsg;
    int64_t i;
};

class BehaviorRoot : public BehaviorListener
{
public:
    virtual void onSuccess(Behavior* behavior)
    {
        // Re-run behavior
        behavior->setStatus(StatusRunning);
    }

    virtual void onFailure(Behavior* behavior)
    {
        // Re-run behavior
        behavior->setStatus(StatusRunning);
    }
};

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    Blackboard<btHashString> bb1, bb2;

    bb1.set("Hi1", 0.f);
    bb2.set("Hi1", 0.f);

    if(bb1 == bb2)
    {
        puts("true");
    }
    else
    {
        puts("false");
    }

    Scheduler aiWorld;

    PrintBehavior b1("Hi1");
    CanPrintDecorator c1(aiWorld, &b1);
    PrintBehavior b2("Hi2");
    CanPrintDecorator c2(aiWorld, &b2);

    std::vector<Behavior*> list1;
    list1.push_back(&c1);
    Sequence s(aiWorld, list1);

    std::vector<Behavior*> list2;
    list2.push_back(&c2);
    list2.push_back(&s);

    Parallel p(aiWorld, list2);

    BehaviorRoot root;
    p.setListener(&root);

    aiWorld.enqueue(&p);
    //aiWorld.update(HighResolutionTime::seconds(5000), 0);

    c1.unlock();
    c2.unlock();

    mainlock = &c2;

    //p.terminate();
    //aiWorld.enqueue(&p);
    aiWorld.update(HighResolutionTime::seconds(5000), 0);

    return EXIT_SUCCESS;
}
