#include <helpers.h>
#include <generic.h>
#include <unistd.h>

#define TRIMTYPES 4
#define TRIMCHAR ' '

const char *trimtypes[]={"ltrim","rtrim","lrtrim","trimall"};
int (*testF[])(char *,char) = { &mbot_ltrim,&mbot_rtrim,&mbot_lrtrim,&mbot_trimall };
int main(void)
{
    int i;

    for(i=0;i<TRIMTYPES;i++)
    {
        int tmp;
        char foo1[]="  trim me trim me mem tri mem tri  ";
        char foo2[]="trim me  trim me mem tri mem tri  ";
        char foo3[]="  trim me trim   me mem tri mem tri";
        char foo4[]="trim    me trim me mem tri mem tri";

        DPRINT("Testing %s function, trimchar='%c'",trimtypes[i]);

        sleep(1);
        printf("Original String=\"%s\"\n",foo1);
        tmp=testF[i](foo1,TRIMCHAR);
        printf("Retval=%d, trimmed str=\"%s\"\n",tmp,foo1);
        printf("Original String=\"%s\"\n",foo2);
        tmp=testF[i](foo2,TRIMCHAR);
        printf("Retval=%d, trimmed str=\"%s\"\n",tmp,foo2);
        printf("Original String=\"%s\"\n",foo3);
        tmp=testF[i](foo3,TRIMCHAR);
        printf("Retval=%d, trimmed str=\"%s\"\n",tmp,foo3);
        printf("Original String=\"%s\"\n",foo4);
        tmp=testF[i](foo4,TRIMCHAR);
        printf("Retval=%d, trimmed str=\"%s\"\n",tmp,foo4);
    }
    return 0;
}
