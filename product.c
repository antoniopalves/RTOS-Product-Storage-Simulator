#include<stdio.h>
#include<stdlib.h>

#include"storage.h"

struct goods {
	int id;
	int entryDate;      // for our lab work, assume one second as one day
	int expiration;     // for sake of demo in this lab, set it as a period (e. g. a value of 10 means it is best for 10 ais (modeled as seconds)
	int x;              // you can use your imagination, provided you keep it simple (to see and use). 
	int z;
};

void newProduct(int id, int entryDate, int expiration, int x, int y) {
	
}