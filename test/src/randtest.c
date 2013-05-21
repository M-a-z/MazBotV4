#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int main(void)
{
	unsigned long long int amntofnmbrs[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned long long int *tmp=&(amntofnmbrs[10]);
	int i=0;
	srand(time(NULL));
	for(i=0;i<1000000000;i++)
	{
		int dice1;
		int dice2;
		dice1=(rand()%10);
		dice2=(rand()%10);
	//	printf("%d-%d=\t%d\n",dice1,dice2,dice1-dice2);
		tmp[(dice1-dice2)]++;
	}
	printf("Summary of results\n");
	for(i=-9;i<10;i++)
	{
		printf("amnt of %d: %lld\n",i,tmp[i]);
	}
	return 0;
}
