#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define fnd_dev "/dev/fnd"

int play_time[4] = {0,0,3,0}; // 초기 시간 30초
//bool isEnd = false; //경기 종료 알림 전역변수
int i, j;

// 카운트다운 함수
int FND_control(int play_time[], int time_sleep){
  //unsigned char FND_DATA_TBL[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90 };
  unsigned char FND_DATA_TBL[]={
        	0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,
        	0x83,0xC6,0xA1,0x86,0x8E,0xC0,0xF9,0xA4,0xB0,0x99,0x89
	};
  //unsigned char FND_DATA_TBL[] = {0,1,2,3,4,5,6,7,8,9};

  
  int fnd_fd = 0;
  unsigned char fnd_num[4];

  //play_time 원소 순서에 맞게 넣기
  fnd_num[0] = FND_DATA_TBL[play_time[0]];
  fnd_num[1] = FND_DATA_TBL[play_time[1]];
  fnd_num[2] = FND_DATA_TBL[play_time[2]];
  fnd_num[3] = FND_DATA_TBL[play_time[3]];

  fnd_fd = open(fnd_dev, O_RDWR);
    if(fnd_fd < 0){printf("fnd error\n");}; // 예외처리

  write(fnd_fd, &fnd_num, sizeof(fnd_num)); // 출력 0030
  sleep(time_sleep); //점등시간 조절

  for (i=2; i>=0; i--) { // 3번째 핀(10의 자릿수)
    for (j=10; j>0; j--) { // 4번째 핀(1의 자릿수)
      // 20, 10 출력
      if ((i==1 || i==0) && j==10) {
        play_time[2] = ++i;
        play_time[3] = 0;
        fnd_num[2] = FND_DATA_TBL[play_time[2]];
        fnd_num[3] = FND_DATA_TBL[play_time[3]];
        write(fnd_fd, &fnd_num, sizeof(fnd_num)); // 출력
        sleep(time_sleep); //점등시간 조절
        i--;
        continue;
      } else {
        if (i==2 && j==10){
          continue;
        }
        // 나머지
        play_time[2] = i;
        play_time[3] = j;
        fnd_num[2] = FND_DATA_TBL[play_time[2]];
        fnd_num[3] = FND_DATA_TBL[play_time[3]];
      }
      write(fnd_fd, &fnd_num, sizeof(fnd_num)); // 출력
      sleep(time_sleep); //점등시간 조절
    }
  }

  close(fnd_fd);
}

int main() {
  //unsigned char FND_DATA_TBL[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90 };
  //unsigned char FND_DATA_TBL[] = {0,1,2,3,4,5,6,7,8,9};
  
  FND_control(play_time, 2);  

  //count_time(play_time, FND_DATA_TBL);
  
  return 0;
}
