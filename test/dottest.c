#include <asm/ioctls.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define clcd "/dev/clcd"
#define led "/dev/led"
#define dot "/dev/dot"
#define fnd_dev "/dev/fnd"

unsigned char rps[1][8] = {
    // 도트 매트릭스 화면
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0}, // 2*2 캐릭터

};

void DOT_control(int rps_col, int time_sleep) {
  int dot_d;

  dot_d = open(dot, O_RDWR);
  if (dot_d < 0) {
    printf("dot Error\n");
  } // 예외처리

  write(dot_d, &rps[rps_col], sizeof(rps)); // 출력
  sleep(time_sleep);                        // 몇초동안 점등할지

  close(dot_d);
}

int main() {
	DOT_control(0, 10);
	return 0;
}
