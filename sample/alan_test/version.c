#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "event2/event.h"

int main(void)
{
	printf("cur ver: %s\n", event_get_version());
	printf("cur ver na: %08x\n", event_get_version_number());
	
	return 0;
}