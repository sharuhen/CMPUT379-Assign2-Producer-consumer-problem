#include "header.h"

using namespace std;

pthread_mutex_t mutexBuffer,mutexask,mutexreceive;

sem_t semEmpty;
sem_t semFull;

int fileid;
int sleepcount;
int workcount;
int completecount;
int askcount;
int receivecount;

ofstream myfile;
auto t_start = std::chrono::high_resolution_clock::now();

queue<int> buffer;
//int count=0;
bool isDone = false;

void* producer(void* args) {
  string prodText;

  while(getline(cin,prodText)) {
    if(prodText[0]=='T') {
      // Sleep(prodText[1]);

      sem_wait(&semEmpty);
      pthread_mutex_lock(&mutexBuffer);
      //Add to the buffer
      auto t_end = std::chrono::high_resolution_clock::now();
      double elapsed_time_ms = std::chrono::duration<double>(t_end-t_start).count();

      double rounded=(int)(elapsed_time_ms*1000.0)/1000.0;

      myfile<<rounded<<setprecision(3)<<"\tComplete\t"<< prodText[1] - '0'<<endl;
      completecount++;      
      buffer.push(prodText[1] - '0');
      pthread_mutex_unlock(&mutexBuffer);
      sem_post(&semFull);
      workcount++;
    } else {
        Sleep(prodText[1] - '0');
        auto t_end = std::chrono::high_resolution_clock::now();   
        double elapsed_time_ms = std::chrono::duration<double>(t_end-t_start).count();
        double rounded=(int)(elapsed_time_ms*1000.0)/1000.0;
        myfile<<rounded<<setprecision(3)<<"\t\t Sleep\t"<< prodText[1] - '0'<<endl;
        sleepcount++;
      }
  }
  pthread_mutex_lock(&mutexBuffer);
  isDone=true;
  sem_post(&semFull);
  pthread_mutex_unlock(&mutexBuffer);

  return 0;
}

void* consumer(void* args) {

  while(1) {
    //Remove from the buffer
    askcount++;

    // auto askend = std::chrono::high_resolution_clock::now();
    // double elapsedend = std::chrono::duration<double>(askend-t_start).count();
    // double askendrounded=(int)(elapsedend*1000.0)/1000.0;
    pthread_mutex_lock(&mutexask);
    auto endask = std::chrono::high_resolution_clock::now();
    double elapsedask = std::chrono::duration<double>(endask-t_start).count();

    double roundedask=(int)(elapsedask*1000.0)/1000.0;
    myfile<<roundedask<<"\t\t Ask\t"<<endl;
    pthread_mutex_unlock(&mutexask);

    sem_wait(&semFull);
    pthread_mutex_lock(&mutexBuffer);
    if(isDone && buffer.size() == 0){
      pthread_mutex_unlock(&mutexBuffer);
      sem_post(&semFull);
      return 0;
    }

    int y=buffer.front();
    buffer.pop();

    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_time_ms = std::chrono::duration<double>(t_end-t_start).count();

    double rounded=(int)(elapsed_time_ms*1000.0)/1000.0;
    pthread_mutex_lock(&mutexreceive);
    myfile<<rounded<<setprecision(3)<<"\t\t Receive\t"<<y<<endl; 
    pthread_mutex_unlock(&mutexreceive);
    receivecount++;
    Trans(y);
    pthread_mutex_unlock(&mutexBuffer);
    sem_post(&semEmpty);
  }
  return 0;
}


int main(int argc, char *argv[])
{
  int noofthreads=stoi(argv[1]); //Number of threads
  //queue as asked in the assignment

  pthread_t threads[noofthreads+1];
  pthread_mutex_init(&mutexBuffer,NULL);
  sem_init(&semEmpty,0,2*noofthreads);
  sem_init(&semFull,0,0);

  

  if(argc==3) {
    int fileid=stoi(argv[2]); // file id from the compiler
    
    string filename="prodcon."+to_string(fileid)+".log";
    myfile.open(filename.c_str());

    
  } else if (argc==2) {

    string x="prodcon";
    string y=".log";
    string filename=x+y;
    myfile.open(filename.c_str());
  }

  int i;
    for(i=0;i<=noofthreads;i++) {
      if(i==0) {
        if(pthread_create(&threads[i],NULL,&producer,NULL)!=0) {
          perror("Producer Thread creation failed!");
      }
      } else {
        if(pthread_create(&threads[i],NULL,&consumer,NULL)!=0) {
          perror("Consumer Thread creation failed!");
        }
      }
    }
    
    for(i=0;i<=noofthreads;i++) {
      if(pthread_join(threads[i],NULL)!=0) {
        perror("Thread joining failed!");
      }
    }

    myfile<<"Summary:"<<endl;
    myfile<<"\tWork\t\t"<<workcount<<endl;
    myfile<<"\tAsk\t\t\t"<<askcount<<endl;
    myfile<<"\tReceive\t\t"<<receivecount<<endl;
    myfile<<"\tComplete\t"<<completecount<<endl;
    myfile<<"\tSleep\t\t"<<sleepcount<<endl;

    auto t_end = std::chrono::high_resolution_clock::now();
    double elapsed_time_ms = std::chrono::duration<double>(t_end-t_start).count();
    double rounded=(int)(elapsed_time_ms*1000.0)/1000.0;
    myfile<<"Transactions per second: "<<workcount/rounded<<endl;


    myfile.close();



  sem_destroy(&semEmpty);
  sem_destroy(&semFull);
  pthread_mutex_destroy(&mutexBuffer);

  
  return 0;
}
