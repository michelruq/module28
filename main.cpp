#include "queue"
#include "list"
#include <mutex>
#include <thread>
#include <random>
#include <memory>
#include <string>
#include <iostream>

using namespace std;


// структура детали
typedef struct Part {
   int part_id;
   float volume;
   typedef shared_ptr<struct Part> PartPtr; // умный указатель
} Part;


static bool done = false; 
queue<Part::PartPtr> shared_queue;
mutex lock_queue;
mutex lock_cout;

void locked_output(const std::string &str) {
   // захват мьютекса для защищенного вывода
   lock_guard<mutex> raii(lock_cout);
   cout << str << endl;
}

void threadAwork(Part::PartPtr& part) {
   // делаем вычитание из объема и имитируем задержку
   part->volume -= 2;
   std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 6000));

   locked_output("threadAwork finished with part " + to_string(part->part_id));
}

void threadBwork(Part::PartPtr& part) {
   // делаем вычитание из объема и имитируем задержку
   part->volume -= 1;
   std::this_thread::sleep_for(std::chrono::milliseconds(500 + rand() % 6000));

   locked_output("threadBwork finished with part " + to_string(part->part_id));
}

void threadA(list<Part::PartPtr>& input) {
   srand(7777777);
   size_t size = input.size();
   for(size_t i=0; i<size; i++) {
       // обрабатываем деталь
       threadAwork(*input.begin());
       // кладём в очередь
       {
          lock_guard<mutex> raii_obj(lock_queue);
          shared_queue.push(Part::PartPtr(*input.begin()));
          input.remove(*input.begin());
          locked_output("Part was added to queue");
       }
   }
   done = true;
}

void threadB() {
   srand(100000);
   while(true) {
       Part::PartPtr part_for_work;
       {
           // блокируем мьютекс
           lock_queue.lock();
           if(shared_queue.empty()) {
               // если нет элементов, то освобождаем мьютекс
               // и делаем задержку в 1 секунду перед
               // следующий просыпанием
               lock_queue.unlock();
               if(done) break; // условие конца работы потока
               locked_output("threadB useless check, queue is empty. Going to bed");
               this_thread::sleep_for(chrono::milliseconds(1000));
               continue;
           } else {
               // забираем элемент из очереди
               part_for_work = shared_queue.front();
               shared_queue.pop();
               // не мешаем потоку А, освободим мьютекс
               lock_queue.unlock();
               locked_output("Part was removed from queue");
           }
       }
       // работаем над деталью
       threadBwork(part_for_work);
   }
}

int main(int argc, char *argv[])
{
   list<Part::PartPtr> spare_parts;
   for(int i=0; i<5; i++) {
      
       spare_parts.push_back(Part::PartPtr(new Part{ i + 1, 10.0 }));   
   }
   // запуск потоков
   thread ta(threadA, std::ref(spare_parts));
   thread tb(threadB);
   // ждем завершения
   ta.join();
   tb.join();

   locked_output("done");
   return 0;
}