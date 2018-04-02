#include <map>
#include <list>

using namespace std;

typedef pair<int, int> TIME_INFO;
typedef pair<TIME_INFO, TIME_INFO> TASK_DURATION;
typedef struct{
    TASK_DURATION duration;
    int enable;
    int taskId;
    int day;
} TASK_DURATION_INFO;

typedef list<TASK_DURATION_INFO> TIME_DURATION_LIST;
typedef map<string, TIME_DURATION_LIST> TIMER_TASK_INFO;

static void (*func)(int);
typedef void (*task_handler)(int);
typedef map<string, task_handler> TASK_TO_DO;
typedef map<string, int> TASK_GLOBAL_SW;

class timertask
{
    private:
      TIMER_TASK_INFO taskInfo;
      TASK_TO_DO taskToDo;
      TASK_GLOBAL_SW taskGlobalSw;
      int addToUci();
      int timeCompare(TIME_INFO t1, TIME_INFO t2);

    public:
      int AddTask(int day, TASK_DURATION_INFO &taskDurationInfo, string taskType, task_handler handler);
      int DelTask(int taskId, string taskType);
      int EditTask(int taskId, string taskType, TASK_DURATION_INFO &taskDurationInfo, task_handler handler);
      int SetTaskSw(int taskId, string taskType, int enable);
      int SetTaskGlobalSw(string taskType, int enable);
      void taskRun();
      void printTask();

};