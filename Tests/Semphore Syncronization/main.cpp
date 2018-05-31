#include <iostream>
#include <unistd.h>
#include<semaphore.h>
#include<pthread.h>

#include "mythread.h"

using namespace std;



int counter = 0;
sem_t countersem;

class TTYPE1 : public MyThreadClass
{
public:
    TTYPE1()
    {

    }

protected:
    void InternalThreadEntry()
    {
        while(1)
        {
            sem_wait(&countersem);
            counter++;
            cout << "increasing counter with 1, counter is currently " << counter << endl;
            usleep(100000);
        }
    }

private:

};

class TTYPE2 : public MyThreadClass
{
public:
    TTYPE2()
    {

    }

protected:
    void InternalThreadEntry()
    {
        while(1)
        {
            sem_wait(&countersem);
            counter += 10;
            cout << "increasing counter with 10, counter is currently " << counter << endl;
            usleep(100000);
        }
    }

private:

};

int main(void)
{
	sem_init(&countersem, 0, 0);
	TTYPE1* t1 = new TTYPE1();
	TTYPE2* t2 = new TTYPE2();
	t1->StartInternalThread();
	t2->StartInternalThread();

	while(1)
	{
		sem_post(&countersem);
		sleep(1);
	}


	t1->WaitForInternalThreadToExit();
	t2->WaitForInternalThreadToExit();
    return 0;
}


