#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>

pthread_mutex_t customer_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t waiting_customers;
sem_t awaits_shaving;

int NuOfCustomers = 0;
#define CUSTOMERS 6
#define BARBERSHOP_CAPACITY 4

pthread_t customer[CUSTOMERS];
pthread_t barber;

void* customer_body(void* vptr)
{
   int customer_id = (int) vptr;
  while(1)
  {
     
      pthread_mutex_lock(&customer_mutex);
       if ( NuOfCustomers < BARBERSHOP_CAPACITY )       
       {
        NuOfCustomers++;
        sem_post(&waiting_customers);
        printf(" number of custoners %d customer id %d \n", NuOfCustomers, customer_id);
        pthread_mutex_unlock(&customer_mutex);
        sem_wait(&awaits_shaving);
       }
       else  pthread_mutex_unlock(&customer_mutex);

      sleep(customer_id);
  }
 
return NULL;
   
}

void* barber_body(void* vptr)
{

   while(1)
   {
      sem_wait(&waiting_customers);
      pthread_mutex_lock(&customer_mutex);
        NuOfCustomers--;
        printf(" shaved one - number of custoners is now %d \n", NuOfCustomers);
        sem_post(&awaits_shaving);
      pthread_mutex_unlock(&customer_mutex);
    
      sleep(2);


   }
return NULL;

}

int main(void)
{   
   int i;

   sem_init(&waiting_customers,0,0);
   sem_init(&awaits_shaving,0,0);
   
   pthread_create(&barber, NULL, &barber_body, NULL);
   for(i=0;i < CUSTOMERS; i++)
    {
       pthread_create( &customer[i], NULL , &customer_body, (void*) (i+1) );
    } 

   pthread_join(barber, NULL);
   for(i=0;i < CUSTOMERS; i++)
    {
       pthread_join( customer[i], NULL);
    } 

return 0;

   

}