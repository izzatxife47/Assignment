/*----------------------------------------------------------------------------
	
	Designers Guide to the Cortex-M Family
	CMSIS RTOS Mutex Example

*----------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "cmsis_os.h"
#include "uart.h"

void x_Thread1 (void const *argument);
void x_Thread2 (void const *argument);
void x_Thread3 (void const *argument);
void x_Thread4 (void const *argument);
void x_Thread5 (void const *argument);
osThreadDef(x_Thread1, osPriorityNormal, 1, 0);
osThreadDef(x_Thread2, osPriorityNormal, 1, 0);
osThreadDef(x_Thread3, osPriorityNormal, 1, 0);
osThreadDef(x_Thread4, osPriorityNormal, 1, 0);
osThreadDef(x_Thread5, osPriorityNormal, 1, 0);

osThreadId T_x1;
osThreadId T_x2;
osThreadId T_x3;
osThreadId T_x4;
osThreadId T_x5;

osMessageQId Q_LED;
osMessageQDef (Q_LED,0x16,unsigned char);
osEvent  result;

osMutexId x_mutex;
osMutexDef(x_mutex);
osSemaphoreId Item_semaphore;                   // Semaphore ID
osSemaphoreDef(Item_semaphore);                 // Semaphore definition
osSemaphoreId Space_semaphore;                  // Semaphore ID
osSemaphoreDef(Space_semaphore);                // Semaphore definition
osSemaphoreId secondtake_semaphore;             // Semaphore ID
osSemaphoreDef(secondtake_semaphore);           // Semaphore definition
osSemaphoreId firsttake_semaphore;              // Semaphore ID
osSemaphoreDef(firsttake_semaphore);            // Semaphore definition

long int x=0;
long int i=0;
long int j=0;
long int k=0;
long int z=0;

const unsigned int N = 5;
unsigned char buffer[N];
unsigned int insertPtr = 0;
unsigned int readPtr = 0;

unsigned char buff_0;
unsigned char buff_1;
unsigned char buff_2;
unsigned char buff_3;
unsigned char buff_4;

void put(unsigned char an_item){
	osSemaphoreWait(Space_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	buffer[insertPtr] = an_item;
	insertPtr = (insertPtr + 1) % N;	// Write Pointer
	buff_0 = buffer[0];			// First slot buffer
	buff_1 = buffer[1];			// Second slot buffer
	buff_2 = buffer[2];			// Third slot buffer
	buff_3 = buffer[3];			// Forth slot buffer
	buff_4 = buffer[4];			// Fifth slot buffer
	osMutexRelease(x_mutex);
	osSemaphoreRelease(Item_semaphore);
}

unsigned char get(){
	unsigned int rr = 0xff;
	osSemaphoreWait(Item_semaphore, osWaitForever);
	osMutexWait(x_mutex, osWaitForever);
	rr = buffer[readPtr];
	readPtr = (readPtr + 1) % N;		// Read Pointer	
	buff_0 = buffer[0];			// First slot buffer
	buff_1 = buffer[1];			// Second slot buffer
	buff_2 = buffer[2];			// Third slot buffer
	buff_3 = buffer[3];			// Forth slot buffer
	buff_4 = buffer[4];			// Fifth slot buffer	
	osMutexRelease(x_mutex);
	osSemaphoreRelease(Space_semaphore);
	return rr;
}


void x_Thread1 (void const *argument) 
{
	//producer
	unsigned char item = 0x41;		// Data in Character
	for(;;){
		put(item++);
	}
}	

void x_Thread2 (void const *argument) 
{
	// Consumer
	unsigned int data = 0x00;
	for(;;){
		osSemaphoreWait(firsttake_semaphore, osWaitForever);	
		data = get();
		osMutexWait(x_mutex, osWaitForever);
		osMessagePut(Q_LED,data,osWaitForever);          //Place a value in the message queue
		osMutexRelease(x_mutex);
		osSemaphoreRelease(secondtake_semaphore);
	}
}

void x_Thread3 (void const *argument) 
{
	// Consumer
	unsigned int c2data = 0x00;
	for(;;){
		osSemaphoreWait(secondtake_semaphore, osWaitForever);
		c2data = get();
		osMutexWait(x_mutex, osWaitForever);
		osMessagePut(Q_LED,c2data,osWaitForever);        //Place a value in the message queue
		osMutexRelease(x_mutex);
		osSemaphoreRelease(firsttake_semaphore);
	}
}

void x_Thread4 (void const *argument) 
{
	// Consumer
	unsigned int c3data = 0x00;
	for(; z<loopcount; z++){
		c3data = get();
		osMessagePut(Q_LED,c3data,osWaitForever);        //Place a value in the message queue
	}
}
void x_Thread5(void const *argument)
{
	// Viewer
	osMessagePut(Q_LED,0x20,osWaitForever);
	for(;;){
		result = 	osMessageGet(Q_LED,osWaitForever);		//wait for a message to arrive
		SendChar(result.value.v);
	}
}

int main (void) 
{
	osKernelInitialize ();                    				// Initialize CMSIS-RTOS
	USART1_Init();
	Item_semaphore = osSemaphoreCreate(osSemaphore(Item_semaphore), 0);
	Space_semaphore = osSemaphoreCreate(osSemaphore(Space_semaphore), N);
	secondtake_semaphore = osSemaphoreCreate(osSemaphore(secondtake_semaphore), N);
	firsttake_semaphore = osSemaphoreCreate(osSemaphore(firsttake_semaphore), 0);
	x_mutex = osMutexCreate(osMutex(x_mutex));	
	
	Q_LED = osMessageCreate(osMessageQ(Q_LED),NULL);	// Create the message queue
	
	T_x1 = osThreadCreate(osThread(x_Thread1), NULL);	// Create Producer 
	T_x2 = osThreadCreate(osThread(x_Thread2), NULL);	// Create Consumer
	T_x3 = osThreadCreate(osThread(x_Thread3), NULL);	// Create Consumer
	T_x4 = osThreadCreate(osThread(x_Thread4), NULL);	// Create Viewer to view the data
	T_x5 = osThreadCreate(osThread(x_Thread4), NULL);
	
	osKernelStart ();                         				// Start thread execution 
}



