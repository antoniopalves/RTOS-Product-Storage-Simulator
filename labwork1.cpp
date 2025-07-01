#include <iostream>
#include<conio.h>
#include<stdlib.h>
#include<windows.h>    // for the Sleep function
using namespace std;

extern "C" {
#include <interface.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
}

typedef struct {
	int id;
	int entryDate;
	int exp; //expiration
	int putTake; //1-put product   0- take product
	int full;
} storeRequest;

typedef struct {
	int x;
	int z;
} TPosition;

typedef struct {
	int dir;  //0-esquerda/baixo 1-direita/cima 
	int pos;
} calibration;

struct {
	storeRequest position[3][3] = { NULL };
	int counter;
}storage;


//System Time
SYSTEMTIME st;
int STOP = 0;
int EMPTY = 0;
int VAR;
int CALIBX = 0;
int CALIBZ = 0;


//mailboxes
xQueueHandle mbx_x;  //for goto_x
xQueueHandle mbx_z;  //for goto_z
xQueueHandle mbx_xz; //for goto_xz
xQueueHandle mbx_input;
xQueueHandle mbx_req;
xQueueHandle mbx_swchInput;
xQueueHandle mbx_stopRes;
xQueueHandle mbx_calX;
xQueueHandle mbx_calZ;


//semahores
xSemaphoreHandle sem_x_done;
xSemaphoreHandle sem_z_done;
xSemaphoreHandle sem_xz_done;
xSemaphoreHandle sem_put_request;
xSemaphoreHandle sem_put_done;
xSemaphoreHandle sem_take_request;
xSemaphoreHandle sem_take_done;
xSemaphoreHandle sem_empty_request;
xSemaphoreHandle sem_led_request;

TaskHandle_t xHandler_x = NULL;
TaskHandle_t xHandler_z = NULL;
TaskHandle_t xHandler_xz = NULL;
TaskHandle_t xHandler_put = NULL;
TaskHandle_t xHandler_take = NULL;
TaskHandle_t xHandler_services = NULL;
TaskHandle_t xHandler_receive = NULL;
TaskHandle_t xHandler_storage = NULL;
TaskHandle_t xHandler_empty = NULL;

//********************************* PROTOTYPES *********************************
int getBitValue(uInt8 value, uInt8 bit_n);
void setBitValue(uInt8* variable, int n_bit, int new_value_bit);
void moveXLeft();
void moveXRight();
void moveZUp();
void moveZDown();
void moveYIn();
void moveYOut();
void stopX();
void stopY();
void stopZ();
int getXPos();
int getYPos();
int getZPos();
void gotoX(int x_dest);
void gotoY(int y_dest);
void gotoZ(int z_dest);
void goUp();
void goDown();
void initialize();
int getExp(int time);
int getSeconds(SYSTEMTIME time);
int verifyID(int id);
TPosition freePos();
TPosition getPos(int id);
void show_menu();
void listProducts();
int getExpTime(storeRequest req, int time);
void listExp();

//********************************* PORTS *********************************

int getBitValue(uInt8 value, uInt8 bit_n)
// given a byte value, returns the value of its bit n
{
	return(value & (1 << bit_n));
}

void setBitValue(uInt8* variable, int n_bit, int new_value_bit)
// given a byte value, set the n bit to value
{
	uInt8  mask_on = (uInt8)(1 << n_bit);
	uInt8  mask_off = ~mask_on;
	if (new_value_bit)*variable |= mask_on;
	else                *variable &= mask_off;
}

//********************************* MOVEMENT *********************************

void moveXLeft()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //  read port 2
	setBitValue(&p, 6, 1);     //  set bit 6 to high level
	setBitValue(&p, 7, 0);      //set bit 7 to low level
	writeDigitalU8(2, p); //  update port 2
	taskEXIT_CRITICAL();

}

void moveXRight()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //read port 2
	setBitValue(&p, 6, 0);    //  set bit 6 to  low level
	setBitValue(&p, 7, 1);      //set bit 7 to high level
	writeDigitalU8(2, p); //update port 2
	taskEXIT_CRITICAL();
}

void moveYIn()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //  read port 2
	setBitValue(&p, 5, 1);     //  set bit 5 to high level
	setBitValue(&p, 4, 0);      //set bit 4 to low level
	writeDigitalU8(2, p); //  update port 2
	taskEXIT_CRITICAL();
}

void moveYOut()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //read port 2
	setBitValue(&p, 5, 0);    //  set bit 5 to  low level
	setBitValue(&p, 4, 1);      //set bit 4 to high level
	writeDigitalU8(2, p); //update port 2
	taskEXIT_CRITICAL();
}

void moveZUp()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //read port 2
	setBitValue(&p, 2, 0);    //  set bit 2 to  low level
	setBitValue(&p, 3, 1);      //set bit 3 to high level
	writeDigitalU8(2, p); //update port 2
	taskEXIT_CRITICAL();
}

void moveZDown()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //  read port 2
	setBitValue(&p, 2, 1);     //  set bit 2 to high level
	setBitValue(&p, 3, 0);      //set bit 3 to low level
	writeDigitalU8(2, p); //  update port 2
	taskEXIT_CRITICAL();
}

//********************************* STOP *********************************

void stopX()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //read port 2
	setBitValue(&p, 6, 0);   //  set bit 6 to  low level
	setBitValue(&p, 7, 0);   //set bit 7 to low level
	writeDigitalU8(2, p); //update port 2
	taskEXIT_CRITICAL();
}

void stopY()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //read port 2
	setBitValue(&p, 5, 0);   //  set bit 5 to  low level
	setBitValue(&p, 4, 0);   //set bit 4 to low level
	writeDigitalU8(2, p); //update port 2
	taskEXIT_CRITICAL();
}

void stopZ()
{
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(2); //read port 2
	setBitValue(&p, 2, 0);   //  set bit 2 to  low level
	setBitValue(&p, 3, 0);   //set bit 3 to low level
	writeDigitalU8(2, p); //update port 2
	taskEXIT_CRITICAL();
}

//********************************* POSITIONS *********************************

int getXPos() {
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(0);
	taskEXIT_CRITICAL();
	if (!getBitValue(p, 2))
		return 1;
	if (!getBitValue(p, 1))
		return 2;
	if (!getBitValue(p, 0))
		return 3;
	return(-1);
}

int getYPos() {
	taskENTER_CRITICAL();
	uInt8 p = readDigitalU8(0);
	taskEXIT_CRITICAL();
	if (!getBitValue(p, 5)) //posiçao de fora return 1
		return 1;
	if (!getBitValue(p, 4)) //posiçao do meio return 2
		return 2;
	if (!getBitValue(p, 3)) //posiçao de dentro return 3
		return 3;
	return(-1);
}

int getZPos() {
	taskENTER_CRITICAL();
	uInt8 p0 = readDigitalU8(0);
	uInt8 p1 = readDigitalU8(1);
	taskEXIT_CRITICAL();
	if (!getBitValue(p1, 3))
		return 1;
	if (!getBitValue(p1, 2))
		return 4; //1,5
	if (!getBitValue(p1, 1))
		return 2;
	if (!getBitValue(p1, 0))
		return 5; //2,5
	if (!getBitValue(p0, 7))
		return 3;
	if (!getBitValue(p0, 6))
		return 6; // 3,5
	return(-1);
}

//********************************* GOTO *********************************

void gotoX(int x_dest) {
	int x_act = getXPos();
	if (x_act != x_dest) {
		if (x_act < x_dest)
			moveXRight();
		else if (x_act > x_dest)
			moveXLeft();
		while (getXPos() != x_dest) {
			Sleep(1);
		}
		stopX();
	}
}

void gotoY(int y_dest)
{
	int y_act = getYPos();
	if (y_act != y_dest) {
		if (y_act < y_dest)
			moveYIn();
		else if (y_act > y_dest)
			moveYOut();
		while (getYPos() != y_dest) {
			Sleep(1);
		}
		stopY();
	}
}

void gotoZ(int z_dest) {
	int z_act = getZPos();
	if (z_act != z_dest) {
		if (z_act < z_dest)
			moveZUp();
		else if (z_act > z_dest)
			moveZDown();
		while (getZPos() != z_dest) {
			Sleep(1);
		}
		stopZ();
	}
}

void goUp() {
	int z_act = getZPos();
	moveZUp();
	while (getZPos() != (z_act + 3)) {
		Sleep(1);
	}
	stopZ();
}

void goDown() {
	int z_act = getZPos();
	moveZDown();
	while (getZPos() != (z_act - 3)) {
		Sleep(1);
	}
	stopZ();
}

//********************************* STORAGE FUNCTIONS *********************************

void initialize() {          //inicializar o vetor 
	int i, j;
	storeRequest init;
	storage.counter = 0;
	init.full = 0;
	init.id = -1;
	init.exp = -1;
	init.entryDate = -1;
	init.putTake = -1;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			storage.position[i][j] = init;
		}
	}
}

void show_menu() {
	printf("\n\n*********************** Calibration ***********************");
	printf("\n\n  Movements: w, a, s, d (stops at nearest sensor)");
	printf("\n\n************************* Comands *************************");
	printf("\n\n (1) - Store a product");
	printf("\n\n (2) - Retrieve a product");
	printf("\n\n (3) - List of stored products and expiration time");
	printf("\n\n (4) - Search for product ID");
	printf("\n\n (5) - List of expired products");
	printf("\n\n***********************************************************");
	printf("\n\nEnter Option: ");
}

int getInput() {
	int key, input=0;

	while(true) {
		xQueueReceive(mbx_input, &key, portMAX_DELAY);
		if (key == 13)
			return input;
		if (key < '0' || key > '9')
			return 0;
		key -= '0';
		input = (input * 10) + key;
	}
}

int getSeconds(SYSTEMTIME time) {
	int secs;
	//GetLocalTime(&time);
	secs = (time.wMinute * 60) + time.wSecond;
	return secs;
}

int verifyID(int id) {
	int i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (storage.position[i][j].id == id)
				return 0;
		}
	}
	return 1;
}

TPosition freePos() {
	int i, j;
	TPosition pos;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (!storage.position[i][j].full) {
				pos.x = i + 1;
				pos.z = j + 1;
				return pos;
			}
		}
	}
}

TPosition getPos(int id) {
	int i, j;
	TPosition pos;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (storage.position[i][j].id == id) {
				pos.x = i + 1;
				pos.z = j + 1;
				return pos;
			}
		}
	}
}

int getExp(int time) {
	int i, j;
	storeRequest product;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (storage.position[i][j].full) {
				product = storage.position[i][j];
				if (time - product.entryDate > product.exp) {
					return product.id;
				}
			}
		}
	}
	return 0;
}

int getExpTime(storeRequest req, int time) {
	int exp;

	exp = req.exp - (time - req.entryDate);
	return exp;
}

void listProducts() {
	int i, j, id, tSec, exp;
	SYSTEMTIME t;

	GetLocalTime(&t);
	tSec = getSeconds(t);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (storage.position[i][j].full) {
				id = storage.position[i][j].id;
				exp = storage.position[i][j].exp - (tSec - storage.position[i][j].entryDate);
				printf("\n %d           %d", id, exp);
			}
		}
	}
}

void listExp() {
	int i, j, exp, tSec;
	storeRequest product;
	SYSTEMTIME t;

	GetLocalTime(&t);
	tSec = getSeconds(t);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (storage.position[i][j].full) {
				product = storage.position[i][j];
				exp = getExpTime(product, tSec);
				if (exp <= 0)
					printf("\n  %d                  (%d,%d)", product.id, i + 1, j + 1);
			}
		}
	}
}

//********************************* TASKS *********************************

void receive_instructions_task(void* ignore) {
	int c = 0;
	while (true) {
		c = _getwch();  // this function is new.
		putchar(c);
		xQueueSend(mbx_input, &c, portMAX_DELAY);
	}
}

void task_storage_services(void* param) {
	SYSTEMTIME time;
	storeRequest req;
	TPosition pos;
	calibration cal;
	int id, exp, cmd;

	while (true) {
		show_menu();
		// get selected option from 
		xQueueReceive(mbx_input, &cmd, portMAX_DELAY);

		switch (cmd) {
		case 'a':
			cal.pos = getXPos();
			if (cal.pos != 1) {
				cal.dir = 0;
				xQueueSend(mbx_calX, &cal, portMAX_DELAY);
			}
			break;
		case 'd':
			cal.pos = getXPos();
			if (cal.pos != 3) {
				cal.dir = 1;
				xQueueSend(mbx_calX, &cal, portMAX_DELAY);
			}
			break;
		case 'w':
			cal.pos = getZPos();
			if (cal.pos != 6) {
				cal.dir = 1;
				xQueueSend(mbx_calZ, &cal, portMAX_DELAY);
			}
			break;
		case 's':
			cal.pos = getZPos();
			if (cal.pos != 1) {
				cal.dir = 0;
				xQueueSend(mbx_calZ, &cal, portMAX_DELAY);
			}
			break;
		/*case ' ':
			stopX();
			stopZ();
			break;*/
		case '1':
			req.putTake = 1;
			cout << "\n Product ID: ";
			req.id = getInput();
			if (req.id) {
				cout << "\n Expiration: ";
				req.exp = getInput();
				if (req.exp) {
					GetLocalTime(&time);
					req.entryDate = getSeconds(time);
					xQueueSend(mbx_req, &req, portMAX_DELAY);
				}
				else
					printf("\n\n Invalid Expiration");
			}
			else
				printf("\n\n Invalid ID");
			break;
		case '2':			
			cout << "\nProduct ID: ";
			req.id = getInput();
			if (req.id) {
				req.putTake = 0;
				xQueueSend(mbx_req, &req, portMAX_DELAY);
			}
			else
				printf("\n\n Invalid ID");
			break;
		case '3':
			printf("\n\n**** Product List ****");
			printf("\n ID        Exp(days)");
			listProducts();
			printf("\n**********************");
			break;
		case '4':
			printf("\n\n Product ID:");			
			id = getInput();
			if (id) {
				if (!verifyID(id)) {
					GetLocalTime(&time);
					pos = getPos(id);
					exp = getExpTime(storage.position[pos.x - 1][pos.z - 1], getSeconds(time));
					printf("\n\nPosition(x,y)    Exp. (days)");
					printf("\n    (%d,%d)            %d", pos.x, pos.z, exp);
				}
				else
					printf("\n\n Product not found");
			}
			else
				printf("\n\n Invalid ID");
			break;
		case '5':
			printf("\n\n**** List of expired products ****");
			printf("\n  ID              Position(x,z)");			
			listExp();
			printf("\n**********************************");
			break;
		case 't': // hidden option
			taskENTER_CRITICAL();
			writeDigitalU8(2, 0); //stop all motors;
			vTaskEndScheduler(); // terminates application
			taskEXIT_CRITICAL();
			break;
		}
	}
}

void calibrateX_task(void* param) {
	calibration cal;
	int pos;

	while (true) {
		xQueueReceive(mbx_calX, &cal, portMAX_DELAY);
		if (cal.dir == 1)
			moveXRight();
		else {
			moveXLeft();
		}
		while (true) {
			pos = getXPos();
			if (pos != -1) {
				if (cal.pos == -1)
					break;
				if (cal.pos != -1 && pos != cal.pos)
					break;
			}
		}
		stopX();
		CALIBX = 1;
	}
}

void calibrateZ_task(void* param) {
	calibration cal;
	int pos;

	while (true) {
		xQueueReceive(mbx_calZ, &cal, portMAX_DELAY);
		if (cal.dir == 1)
			moveZUp();
		else
			moveZDown();
		while (true) {
			pos = getZPos();
			if (pos != -1) {
				if (cal.pos == -1)
					break;
				if (cal.pos != -1 && pos != cal.pos)
					break;
			}
		}
		stopZ();
		CALIBZ = 1;
	}
}

void storageHandler_task(void* param) {
	storeRequest req;
	TPosition pos;

	while (true) {
		xQueueReceive(mbx_req, &req, portMAX_DELAY);
		if (req.putTake) {
			if (verifyID(req.id)) {
				if (storage.counter < 9) {
					storage.counter++;
					pos = freePos();
					req.full = 1;
					storage.position[pos.x - 1][pos.z - 1] = req;
					xQueueSend(mbx_xz, &pos, portMAX_DELAY);
					xSemaphoreTake(sem_xz_done, portMAX_DELAY);
					xSemaphoreGive(sem_put_request);
					xSemaphoreTake(sem_put_done, portMAX_DELAY);
				}
				else
					printf("\nStorage is full");
			}
			else
				printf("\nProdut ID alredy registered");
		}
		else {
			if (!verifyID(req.id)) {
				pos = getPos(req.id);
				xQueueSend(mbx_xz, &pos, portMAX_DELAY);
				xSemaphoreTake(sem_xz_done, portMAX_DELAY);
				xSemaphoreGive(sem_take_request);
				xSemaphoreTake(sem_take_done, portMAX_DELAY);

				storage.counter--;
				req.id = -1;
				req.exp = -1;
				req.entryDate = -1;
				req.full = 0;
				storage.position[pos.x - 1][pos.z - 1] = req;
			}
			else
				cout << "\nInvalid ID";
		}
	}
}

void goto_xz_task(void* param) {
	TPosition pos;

	while (true) {
		xQueueReceive(mbx_xz, &pos, portMAX_DELAY);

		xQueueSend(mbx_x, &pos.x, portMAX_DELAY);
		xQueueSend(mbx_z, &pos.z, portMAX_DELAY);

		//wait for goto_x completion, synchronization
		xSemaphoreTake(sem_x_done, portMAX_DELAY);

		//wait for goto_z completion, synchronization           
		xSemaphoreTake(sem_z_done, portMAX_DELAY);

		xSemaphoreGive(sem_xz_done);
	}
}

void goto_x_task(void* param) {
	while (true) {
		int x;
		xQueueReceive(mbx_x, &x, portMAX_DELAY);
		gotoX(x);
		xSemaphoreGive(sem_x_done);
	}
}

void goto_z_task(void* param) {
	while (true) {
		int z;
		xQueueReceive(mbx_z, &z, portMAX_DELAY);
		gotoZ(z);
		xSemaphoreGive(sem_z_done);
	}
}

void putPartInCell_task(void* param) {
	while (true) {
		xSemaphoreTake(sem_put_request, portMAX_DELAY);
		goUp();
		gotoY(3);
		goDown();
		gotoY(2);
		xSemaphoreGive(sem_put_done);
	}
}

void takePartFromCell_task(void* param) {
	while (true) {
		xSemaphoreTake(sem_take_request, portMAX_DELAY);
		gotoY(3);
		goUp();
		gotoY(2);
		goDown();
		xSemaphoreGive(sem_take_done);
	}
}

void receiveButton_task(void* param) {

	uInt8 p;
	SYSTEMTIME t0, t1;
	int s1, s2;
	int state;
	int counter = 0;
	int swch;

	while (true) {
		taskENTER_CRITICAL();
		p = readDigitalU8(1);
		s1 = getBitValue(p, 5);
		s2 = getBitValue(p, 6);
		taskEXIT_CRITICAL();

		state = 0;
		if (s1) {

			GetLocalTime(&t0);
			if (s2) {
				state = 3;
			}
			else {
				state = 1;
				counter = 0;				
				while (s1) {
					if (counter < 5) {
						GetLocalTime(&t1);
						counter = t1.wSecond - t0.wSecond;
					}
					else {
						state = 4;
						break;
					}
					taskENTER_CRITICAL();
					s1 = getBitValue(readDigitalU8(1), 5);
					taskEXIT_CRITICAL();
				}

			}
		}
		if (s2 && !s1) {
			state = 2;
		}
		xQueueSend(mbx_swchInput, &state, portMAX_DELAY);
	}
}

void swichManager_task(void* param) {
	int state;
	int stopRes;
	while (true) {
		xQueueReceive(mbx_swchInput, &state, portMAX_DELAY);

		switch (state) {
		case 1:
			if (STOP) {
				stopRes = 2;
				xQueueSend(mbx_stopRes, &stopRes, portMAX_DELAY);
			} break;
		case 2:
			if (STOP) {
				stopRes = 3;
				xQueueSend(mbx_stopRes, &stopRes, portMAX_DELAY);
			} break;
		case 3:
			stopRes = 1;
			xQueueSend(mbx_stopRes, &stopRes, portMAX_DELAY);
			break;
		case 4:
			VAR = 1;
			xSemaphoreGive(sem_empty_request);
			break;
		}
	}
}

void emptyExp_task(void* param) {

	SYSTEMTIME t;
	storeRequest req;
	int time;
	int id;
	int lastId = 0;

	while (true) {
		xSemaphoreTake(sem_empty_request, portMAX_DELAY);
		GetLocalTime(&t);
		time = getSeconds(t);
		id = 1;
		while (VAR) {
			id = getExp(time);
			if (id) {
				if (id != lastId) {
					EMPTY = 1;
					xSemaphoreGive(sem_led_request);
					req.putTake = 0;
					req.id = id;
					xQueueSend(mbx_req, &req, portMAX_DELAY);
					lastId = id;
				}
				else {
					id = -1;
				}
			}
			else {
				EMPTY = 0;
				VAR = 0; //para sair da task
			}
		}
	}
}

void stopRes_task(void* param) {
	uInt8 port2;
	int stopRes;
	int wasExp = 0;
	while (true) {
		xQueueReceive(mbx_stopRes, &stopRes, portMAX_DELAY);
		switch (stopRes) {
		case 1:
			if (!STOP) {
				taskENTER_CRITICAL();
				port2 = readDigitalU8(2);
				taskEXIT_CRITICAL();
				stopX();
				stopZ();
				stopY();
				if (EMPTY == 1) {
					EMPTY = 0;
					wasExp = 1;
				}
				STOP = 1;
				xSemaphoreGive(sem_led_request);
				vTaskSuspend(xHandler_xz);
				vTaskSuspend(xHandler_x);
				vTaskSuspend(xHandler_z);
				vTaskSuspend(xHandler_services);
				vTaskSuspend(xHandler_put);
				vTaskSuspend(xHandler_take);
				vTaskSuspend(xHandler_receive);
				vTaskSuspend(xHandler_storage);
				vTaskSuspend(xHandler_empty);
				printf("\n\n**********************************");
				printf("\n*      EMERGENCY STOP            *");
				printf("\n* Switch1-Resume   Switch2-Reset *");
				printf("\n**********************************");
			}
			break;
		case 2:
			STOP = 0;
			if (wasExp == 1)
				EMPTY = 1;
			taskENTER_CRITICAL();
			writeDigitalU8(2, port2);
			taskEXIT_CRITICAL();
			vTaskResume(xHandler_xz);
			vTaskResume(xHandler_x);
			vTaskResume(xHandler_z);
			vTaskResume(xHandler_services);
			vTaskResume(xHandler_put);
			vTaskResume(xHandler_take);
			vTaskResume(xHandler_receive);
			vTaskResume(xHandler_storage);
			vTaskResume(xHandler_empty);
			show_menu();
			break;
		case 3:
			STOP = 0;
			gotoY(2);
			vTaskResume(xHandler_receive);
			vTaskResume(xHandler_services);
			initialize();
			CALIBX = 0;
			CALIBZ = 0;
			printf("\n\n********* STORAGE RESET ************");
			printf("\n* Please calibrate before shutdown *");
			printf("\n************************************");
			while (!CALIBX || !CALIBZ) {

			}
			taskENTER_CRITICAL();
			writeDigitalU8(2, 0); //stop all motors;
			vTaskEndScheduler(); // terminates application
			taskEXIT_CRITICAL();
			break;
		}
	}
}

void ledBlink_task(void* param) {
	uInt8 p;
	TickType_t delay;

	while (true) {
		xSemaphoreTake(sem_led_request, portMAX_DELAY);
		if (EMPTY) {
			delay = 500 / portTICK_PERIOD_MS;
			while (EMPTY) {
				taskENTER_CRITICAL();
				p = readDigitalU8(2);
				setBitValue(&p, 0, 1);
				writeDigitalU8(2, p);
				taskEXIT_CRITICAL();
				vTaskDelay(delay);
				taskENTER_CRITICAL();
				p = readDigitalU8(2);
				setBitValue(&p, 0, 0);
				writeDigitalU8(2, p);
				taskEXIT_CRITICAL();
				vTaskDelay(delay);
			}
		}
		if (STOP) {
			delay = 250 / portTICK_PERIOD_MS;
			while (STOP) {
				taskENTER_CRITICAL();
				p = readDigitalU8(2);
				setBitValue(&p, 0, 1);
				setBitValue(&p, 1, 1);
				writeDigitalU8(2, p);
				taskEXIT_CRITICAL();
				vTaskDelay(delay);
				taskENTER_CRITICAL();
				p = readDigitalU8(2);
				setBitValue(&p, 0, 0);
				setBitValue(&p, 1, 0);
				writeDigitalU8(2, p);
				taskEXIT_CRITICAL();
				vTaskDelay(delay);
			}
		}
	}
}

//*************************************************************************

int main(int argc, char** argv) {
	printf("\nwaiting for hardware simulator...");
	createDigitalInput(0);
	createDigitalInput(1);
	createDigitalOutput(2);
	writeDigitalU8(2, 0);
	printf("\ngot access to simulator...");

	GetLocalTime(&st); //system time
	initialize();

	sem_x_done = xSemaphoreCreateCounting(1000, 0);// try modeling adequate    
	sem_z_done = xSemaphoreCreateCounting(1000, 0);// capacity for the semaphores 
	sem_xz_done = xSemaphoreCreateCounting(1000, 0);
	sem_put_request = xSemaphoreCreateCounting(1000, 0);
	sem_put_done = xSemaphoreCreateCounting(1000, 0);
	sem_take_request = xSemaphoreCreateCounting(1000, 0);
	sem_take_done = xSemaphoreCreateCounting(1000, 0);
	sem_empty_request = xSemaphoreCreateCounting(1000, 0);
	sem_led_request = xSemaphoreCreateCounting(1000, 0);

	mbx_req = xQueueCreate(10, sizeof(storeRequest));
	mbx_x = xQueueCreate(10, sizeof(int));
	mbx_z = xQueueCreate(10, sizeof(int));
	mbx_xz = xQueueCreate(10, sizeof(TPosition));
	mbx_input = xQueueCreate(10, sizeof(int));
	mbx_swchInput = xQueueCreate(10, sizeof(int));
	mbx_stopRes = xQueueCreate(10, sizeof(int));
	mbx_calX = xQueueCreate(10, sizeof(calibration));
	mbx_calZ = xQueueCreate(10, sizeof(calibration));

	xTaskCreate(goto_x_task, "goto_x_task", 100, NULL, 0, &xHandler_x);
	xTaskCreate(goto_z_task, "goto_z_task", 100, NULL, 0, &xHandler_z);
	xTaskCreate(goto_xz_task, "goto_xz_task", 100, NULL, 0, &xHandler_xz);
	xTaskCreate(task_storage_services, "task_storage_services", 100, NULL, 0, &xHandler_services);
	xTaskCreate(receive_instructions_task, "receive_instructions_task", 100, NULL, 0, &xHandler_receive);
	xTaskCreate(putPartInCell_task, "putPartInCell_task", 100, NULL, 0, &xHandler_put);
	xTaskCreate(storageHandler_task, "storageHandler_task", 100, NULL, 0, &xHandler_storage);
	xTaskCreate(takePartFromCell_task, "takePartFromCell_task", 100, NULL, 0, &xHandler_take);
	xTaskCreate(receiveButton_task, "receiveButton_task", 100, NULL, 0, NULL);
	xTaskCreate(swichManager_task, "swichManager_task", 100, NULL, 0, NULL);
	xTaskCreate(emptyExp_task, "emptyExp_task", 100, NULL, 0, &xHandler_empty);
	xTaskCreate(ledBlink_task, "ledBlink_task", 100, NULL, 0, NULL);
	xTaskCreate(stopRes_task, "stopRes_task", 100, NULL, 0, NULL);
	xTaskCreate(calibrateX_task, "calibrateX_task", 100, NULL, 0, NULL);
	xTaskCreate(calibrateZ_task, "calibrateZ_task", 100, NULL, 0, NULL);

	vTaskStartScheduler();
}