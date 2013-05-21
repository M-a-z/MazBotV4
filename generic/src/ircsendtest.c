#include "irchelpers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    char teststr1[]="123456789_12345678";
    char teststr2[]="123456789_123456789";
    char teststr3[]="123456789_123456789_";
    char teststr4[]="123456789_123456789_1234";
    char teststr5[]="123456789_123456789_123456789_123456789_123456789";
    char teststr6[]="123456789_123456789_123456789_123456789_123456789_123456789_123456789_12345";
    char *test_res_str1;
    char *test_res_str2;
    char *test_res_str3;
    char *test_res_str4;
    char *test_res_str5;
    char *test_res_str6;
    size_t sends1;
    size_t sends2;
    size_t sends3;
    size_t sends4;
    size_t sends5;
    size_t sends6;

    printf("Calling: prepare_for_sending(%p,\"%%s\",%s);\n",&sends1,&(teststr1[0]));
    test_res_str1=prepare_for_sending(&sends1,"%s",&(teststr1[0]));
    printf("Calling: prepare_for_sending(%p,\"%%s\",%s);\n",&sends2,&(teststr2[0]));
    test_res_str2=prepare_for_sending(&sends2,"%s",&(teststr2[0]));
    printf("Calling: prepare_for_sending(%p,\"%%s\",%s);\n",&sends3,&(teststr3[0]));
    test_res_str3=prepare_for_sending(&sends3,"%s",&(teststr3[0]));
    printf("Calling: prepare_for_sending(%p,\"%%s\",%s);\n",&sends4,&(teststr4[0]));
    test_res_str4=prepare_for_sending(&sends4,"%s",&(teststr4[0]));
    printf("Calling: prepare_for_sending(%p,\"%%s\",%s);\n",&sends5,&(teststr5[0]));
    test_res_str5=prepare_for_sending(&sends5,"%s",&(teststr5[0]));
    printf("Calling: prepare_for_sending(%p,\"%%s\",%s);\n",&sends6,&(teststr6[0]));
    test_res_str6=prepare_for_sending(&sends6,"%s",&(teststr6[0]));

    if(NULL==test_res_str1 || NULL==test_res_str2 ||NULL==test_res_str3 ||NULL==test_res_str4 ||NULL==test_res_str5 ||NULL==test_res_str6 )
    {
        printf("NULL ptr returned by prepare_for_sending()!\n");
        return -1;
    }

    printf("test1 :\norig=\"%s\" \nresult=\"%s\"\nsendsiz=%u,strlen=%u,lastmark=%u\n\n",teststr1,test_res_str1,sends1,(unsigned int)strlen(test_res_str1),(unsigned int)test_res_str1[sends1]);
    printf("test2 :\norig=\"%s\" \nresult=\"%s\"\nsendsiz=%u,strlen=%u,lastmark=%u\n\n",teststr2,test_res_str2,sends2,(unsigned int)strlen(test_res_str2),(unsigned int)test_res_str2[sends2]);
    printf("test3 :\norig=\"%s\" \nresult=\"%s\"\nsendsiz=%u,strlen=%u,lastmark=%u\n\n",teststr3,test_res_str3,sends3,(unsigned int)strlen(test_res_str3),(unsigned int)test_res_str3[sends3]);
    printf("test4 :\norig=\"%s\" \nresult=\"%s\"\nsendsiz=%u,strlen=%u,lastmark=%u\n\n",teststr4,test_res_str4,sends4,(unsigned int)strlen(test_res_str4),(unsigned int)test_res_str4[sends4]);
    printf("test5 :\norig=\"%s\" \nresult=\"%s\"\nsendsiz=%u,strlen=%u,lastmark=%u\n\n",teststr5,test_res_str5,sends5,(unsigned int)strlen(test_res_str5),(unsigned int)test_res_str5[sends5]);
    printf("test6 :\norig=\"%s\" \nresult=\"%s\"\nsendsiz=%u,strlen=%u,lastmark=%u\n\n",teststr6,test_res_str6,sends6,(unsigned int)strlen(test_res_str6),(unsigned int)test_res_str6[sends6]);
    free(test_res_str1);
    free(test_res_str2);
    free(test_res_str3);
    free(test_res_str4);
    free(test_res_str5);
    free(test_res_str6);
    return 0;
}
