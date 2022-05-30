// 수정 이전2

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <asm/ioctls.h>

#define tact_d "/dev/tactsw"

int main()
{
    struct timeval dotst, dotend, tactst, tactend;
    int tact;
    unsigned char c;
  
    if((tact = open(tact_d,O_RDWR)) < 0)
    {
        perror("open");
        exit(1);
    }

    gettimeofday(&tactst, NULL);
    while(1)
    {
        read(tact,&c,sizeof(c));
      
        switch(c)
        {
            //왼쪽일 경우 왼쪽으로, 오른쪽일 경우 오른쪽으로 이동
            case 1: 
              printf("1번 버튼\n");
              break;
            case 2: 
              printf("2번 버튼\n");
              break;
            case 3: 
              printf("3번 버튼\n");
              break;
            case 4: 
              printf("4번 버튼\n");
              break;
            case 5: 
              printf("5번 버튼\n");
              break;
            case 6: 
              printf("6번 버튼\n");
              break;
            case 7: 
              printf("7번 버튼\n");
              break;
            case 8: 
              printf("8번 버튼\n");
              break;
            case 9: 
              printf("9번 버튼\n");
              break;
            case 10: 
              printf("10번 버튼\n");
              break;
            case 11: 
              printf("11번 버튼\n");
              break;
            case 12: 
              printf("12번 버튼\n");
              break;
        }
    }
    close(tact);
    return 0;
}
