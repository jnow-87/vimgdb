#include <math.h>


int strlen(int val, int base){
	unsigned int len;


	len = 1;
	while(1){
		if(val < pow(base, len))
			return len;
		len++;
	}
}
