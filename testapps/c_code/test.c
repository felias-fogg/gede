#include <stdio.h>
#include <stdint.h>

int global_var1 = 1;

	 	//rad1
   	//kalle
	//tab1
		//tab2
			//tab3
//		|tab1_again
//  	|tab2_again
    //kalle
  	//nisse

#define EMPTY ""


void long_func_with_many_arguments(int a_very_long_function_variable_name_xxxxxxx, int another_very_long_variable_name___, int another_another_very_long_variable_name___, int another_another_very_long_variable_name___2)
{
}

void testLargeArray()
{
    struct Element
    {
        uint64_t a;
    };
    static struct Element array[8*1024];
    const int elementCount = sizeof(array)/sizeof(array[0]);
    
    for(int i = 0;i < elementCount;i++)
    {
        array[i].a = i;
//sleep(1);
    }

    array[elementCount-1].a = 0;
    array[elementCount-1].a = 1;
    array[elementCount-1].a = 2;
    array[elementCount-1].a = 3;
    
    
}


typedef enum {CUSTOM_ENUM1, CUSTOM_ENUM2} CustomEnum;
int main(int argc,char *argv[])
{
    
    char sbuffer[1024];
    const char **strList;
    const char *strListData[] = {"hej", "kalle"};
    int i3[] = { 111,222};
    struct
    {
        int a;
        struct
        {
            int b;
        }s2;
    }s;
    int a = 1001;
    char c = 'Z';
    float f1;
    char *varStr;
    enum {ENUM1, ENUM2}varEnum;
    unsigned char d = 0;
    CustomEnum customEnum1;
    int i;

    for(i = 1;i < argc;i++)
    {
        printf("%d:>%s<\n", i, argv[i]);
    }

    global_var1++;
    global_var1++;
    global_var1++;

    sbuffer[0] = '1';
    sbuffer[1] = '2';
    sbuffer[2] = '3';
    for(i = 3;i <= 9;i++)
        sbuffer[i] = '0'+i;

    printf("hello"EMPTY" >");
    printf("<\n");

    varStr = "stri\n\r\t\03ng1";
    varStr = "string2";
    f1 = 0.0;
    s.a = 0;
    s.s2.b = 1;
    varEnum = ENUM1;
    varEnum = ENUM2;
    c = '\'';
    a++;

    strList = strListData;

    switch(s.a)
    {
        case 0:
            {
                printf("Hej!\n");
            };break;
        default:;break;
    }

    testLargeArray();
    
    while(1)
    {
        c++;
        f1 += 0.1;
        func();
        s.a++;
        s.s2.b++;
        s.s2.b++;
        s.a++;
        d++;
    }
    return 0;
}

