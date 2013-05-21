#include <APIs/cexplode.h>
#include <generic.h>
#include <stdio.h>
#include <unistd.h>
#define TEST_STR_AMNT 7

char *testdata[TEST_STR_AMNT]={"1234567890","123 123456789", " 123456789", "123456789 ", " 12 34 56 78 90", " 1 2 3 4 5 6 7 8 9 0 ", "1 2 3 4 5 6 7 8 9"}; 

int main()
{
	CexplodeStrings exp_obj;
	int i,j;
	int cexpRet;
	char *tmp;

	for(i=0;i<TEST_STR_AMNT;i++)
	{
		printf("TestStr%d = \"%s\"\n",i+1,testdata[i]);
		printf("Exploding...\n");
		sleep(4);
		cexpRet=Cexplode(testdata[i]," ",&exp_obj);
		printf("cexpRet = %d\n",cexpRet);
		sleep(1);
		tmp=Cexplode_getNth(1,&exp_obj);
		DPRINT("Cexplode_getNth(1,..) returned \"%s\"",tmp);
		tmp=Cexplode_getnext(&exp_obj);
		DPRINT("Now, Cexplode_getnext returned \"%s\"",tmp);
		tmp=Cexplode_getNth(1,&exp_obj);
		for(j=0;NULL!=tmp;j++)
		{
			printf("Piece %d =\"%s\"\n",j+1,tmp);
			sleep(15);
			tmp=Cexplode_getnext(&exp_obj);
		}
		Cexplode_free(exp_obj);
	}
	return 0;
}




