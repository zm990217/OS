#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<memory.h>
#include<pthread.h>
#include<unistd.h>

#define NORTH 0
#define WEST  1
#define SOUTH 2
#define EAST  3
#define MAX 100
//四种方向
char NWSE[4][6] = { "North","West","South","East" };
//车辆结构体
typedef struct car {
	int dire;//车辆的来源方向
	int numb;//车辆的编号
}*Car;
//队列数据结构
typedef struct queue {
	int Capacity;//队列最大容量
	int Size;	 //队列大小
	int Front;	 //队列的首元素
	int Rear;    //队列的尾元素
	int* Array;  //队列中所有元素所在的数组
}*Queue;
//初始化队列
Queue queue_init(int cap)
{
	Queue q = (Queue)malloc(sizeof(struct queue));//动态分配空间
	q->Capacity = cap;//设置最大容量
	q->Front = 1;     //空队列首元素下标设为1
	q->Rear = 0;      //空队列尾元素下标设为0
	q->Size = 0;      //空队列初始大小设为0
	q->Array = (int*)malloc(sizeof(int) * cap); //按照最大容量申请数组空间
	return q;         //返回队列指针
}
//回收队列
void queue_free(Queue q)
{
	free(q->Array); //回收队列元素数组
	free(q);        //回收队列指针
}
//入队
void enqueue(int Elem, Queue q)
{
	//如果队列已满
	if (q->Size >= q->Capacity) {
		printf("The queue is full!\n");
		return;
	}
	else {
		//尾元素下标循环右移
		q->Rear = (q->Rear + 1) % (q->Capacity);
		q->Array[q->Rear] = Elem;
		//队列大小加一
		q->Size++;
	}
}
//出队
int dequeue(Queue q)
{
	//如果队列是空的
	if (q->Size <= 0) {
		printf("The queue is empty!\n");
		return -1;
	}
	else {
		int Elem;
		//查找当前首元素下标处的元素值
		Elem = q->Array[q->Front];
		//首元素下标循环右移
		q->Front = (q->Front + 1) % (q->Capacity);
		//容量减少
		q->Size--;
		//返回该元素
		return Elem;
	}
}
//查看队列首元素
int front(Queue q)
{
	//如果队列是空的
	if (q->Size <= 0) {
		printf("The queue is empty!\n");
		return -1;
	}
	//返回首元素值
	else return q->Array[q->Front];
}
int carNum = 0;  //车辆编号
int carInCross;  //道路中是否有车辆
int length;      //输入的字符串长度
Queue waitCarQueue[4];   //车辆等待队列
pthread_cond_t first[4]; //信号量：该方向的车辆下一个通过路口
pthread_mutex_t waitQMut[4];//等待队列互斥锁
pthread_mutex_t printLock;//输出互斥锁

pthread_mutex_t cross; //道路互斥锁
pthread_t deadLockCheck; //死锁检测线程

int waiting[4]; //某方向上是否有车辆在等待通过

pthread_cond_t leaveCarQueue[4]; //信号量：该方向上有车辆离开
pthread_cond_t leaveCross; //信号量：有车辆离开路口
pthread_mutex_t waitingLock[4];//等待车辆标记数组互斥锁

//死锁检测
void* checkDeadLock()
{
	while (1)
	{
		//在检测是对道路添加互斥锁
		pthread_mutex_lock(&cross);
		while (carInCross) //如果路口中有其余车辆
		{
			pthread_cond_wait(&leaveCross, &cross);//等待离开路口信号量（防止反复死锁）
		}
		//如果不是每个路口都有车辆在等待，则不是死锁
		if (!(waiting[NORTH] & waiting[WEST] & waiting[SOUTH] & waiting[EAST]))
		{
			//解锁道路
			pthread_mutex_unlock(&cross);
			//继续死锁检测
			continue;
		}
		else {
			//打印死锁信息
			pthread_mutex_lock(&printLock);
			printf("DEADLOCK: car jam detected, signalling NORTH to go.\n");
			pthread_mutex_unlock(&printLock);

			//释放信号量，通知NORTH方向上的车辆先行
			pthread_mutex_lock(&waitQMut[WEST]);
			pthread_cond_signal(&first[NORTH]);
			pthread_mutex_unlock(&waitQMut[WEST]);

			//释放信号量，通知有车辆离开路口
			pthread_cond_signal(&leaveCross);
			//解锁
			pthread_mutex_unlock(&cross);
		}

	}
}
//车辆线程
void* carFrom(void* info)
{
	Car thisCar = (Car)info;   //保存车辆信息
	int dire = thisCar->dire;  //车辆方向
	int numb = thisCar->numb;  //车辆编号
	char* theDirection;        //方向（字符串形式）
	theDirection = NWSE[dire];

	//对车辆所在方向的等待队列添加互斥锁
	pthread_mutex_lock(&waitQMut[dire]);
	//车辆加入等待队列
	enqueue(numb, waitCarQueue[dire]);

	//如果该车辆不是队列的首元素
	while (front(waitCarQueue[dire]) != numb) {
		//等待有车辆离开
		pthread_cond_wait(&leaveCarQueue[dire], &waitQMut[dire]);
	}
	//打印车辆到来信息
	pthread_mutex_lock(&printLock);
	printf("Car %d from %s arrives at crossing.\n", numb, theDirection);
	pthread_mutex_unlock(&printLock);
	//解锁等待队列
	pthread_mutex_unlock(&waitQMut[dire]);
	
	//对该方向右边的等待队列添加互斥锁
	pthread_mutex_lock(&waitQMut[(dire + 1) % 4]);
	//如果该车辆右边方向有车辆，则不可直接通过，需等待
	if (waitCarQueue[(dire + 1) % 4]->Size != 0)
	{
		//修改等待信息
		pthread_mutex_lock(&waitingLock[dire]);
		waiting[dire] = 1;
		pthread_mutex_unlock(&waitingLock[dire]);
		//等待first信号通知通过路口
		pthread_cond_wait(&first[dire], &waitQMut[(dire + 1) % 4]);
		//此时已收到通过路口的信号，修改等待变量
		pthread_mutex_lock(&waitingLock[dire]);
		waiting[dire] = 0;
		pthread_mutex_unlock(&waitingLock[dire]);
	}
	//解锁右边方向的等待队列
	pthread_mutex_unlock(&waitQMut[(dire + 1) % 4]);

	//车辆即将通过路口，对路口添加互斥锁
	pthread_mutex_lock(&cross);
	//如果路口上有其他车辆
	while (carInCross) {
		//等待离开信号
		pthread_cond_wait(&leaveCross, &cross);
	}
	//设置路口中有车辆
	carInCross = 1;
	//打印通过路口信息
	pthread_mutex_lock(&printLock);
	printf("Car %d from %s leaving crossing.\n", numb, theDirection);
	pthread_mutex_unlock(&printLock);
	//释放信号量，通知左侧方向的车辆可以离开
	pthread_mutex_lock(&waitQMut[dire]);
	pthread_cond_signal(&first[(dire + 3) % 4]);
	pthread_mutex_unlock(&waitQMut[dire]);

	//对当前车辆方向等待队列添加互斥锁
	pthread_mutex_lock(&waitQMut[dire]);
	//车辆离开等待队列
	dequeue(waitCarQueue[dire]);
	//释放信号量，该方向有车辆离开路口（下一车辆可以准备就绪）
	pthread_cond_signal(&leaveCarQueue[dire]);
	//解锁
	pthread_mutex_unlock(&waitQMut[dire]);

	//设置路口中没有车辆
	carInCross = 0;
	//释放信号量，有车辆离开路口
	pthread_cond_signal(&leaveCross);
	//解锁路口
	pthread_mutex_unlock(&cross);
}

//主函数
int main(int argc, char* argv[])
{
	int i;
	int error;
	char input[MAX + 1]; //输入字符串
	pthread_t car[MAX];  //车辆线程

	//初始化每个方向的各个变量
	for (i = 0; i < 4; ++i)
	{
		pthread_cond_init(&first[i], NULL);           //初始化first信号量
		pthread_cond_init(&leaveCarQueue[i], NULL);   //初始化离开等待队列信号量
		pthread_mutex_init(&waitingLock[i], NULL);    //初始化等待信息互斥锁
		pthread_mutex_init(&waitQMut[i], NULL);       //初始化等待队列互斥锁
		waitCarQueue[i] = queue_init(MAX);            //初始化等待队列
		waiting[i] = 0;                               //初始化等待信息
	}

	pthread_mutex_init(&cross, NULL);                 //初始化道路互斥锁
	pthread_mutex_init(&printLock, NULL);             //初始化打印互斥锁
	pthread_cond_init(&leaveCross, NULL);             //初始化离开路口信号量
	carInCross = 0;                                   //初始化路口上是否有车辆

	//创建死锁检测线程
	error = pthread_create(&deadLockCheck, NULL, checkDeadLock, NULL);
	//若线程创建失败，返回错误信息
	if (error != 0)printf("Creat deadLockCheck thread failed! %s\n", strerror(error));

	strcpy(input, argv[1]);        //获取输入字符串
	length = strlen(input);        //获取字符串长度

	//对每个输出进行遍历
	for (i = 0; i < length; i++)
	{
		//车辆信息
		Car thisCar = (Car)malloc(sizeof(struct car));
		//确定车辆的来源方向
		switch (input[i])
		{
		case 'n':
			thisCar->dire = NORTH;
			break;
		case 'e':
			thisCar->dire = EAST;
			break;
		case 's':
			thisCar->dire = SOUTH;
			break;
		case 'w':
			thisCar->dire = WEST;
			break;
		default:
			break;
		}
		//确定车辆编号
		thisCar->numb = ++carNum;
		//创建车辆线程
		error = pthread_create(&car[i], NULL, carFrom, thisCar);
		//若线程创建失败，返回错误信息
		if (error != 0)printf("Can't create car %d! %s\n", thisCar->numb, strerror(error));
	}
	//等待线程完成
	for (i = 0; i < length; i++) {
		pthread_join(car[i], NULL);
	}
	//清空队列
	for (i = 0; i < 4; i++) {
		queue_free(waitCarQueue[i]);
	}

	return 0;
}
