// timer, matrix ok

#include <asm/ioctls.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define dot "/dev/dot"
#define tact_d "/dev/tactsw"
#define fnd "/dev/fnd"
#define clcd "/dev/clcd"

unsigned char obstacle[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char player[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0};
unsigned char matrix[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //출력할 매트릭스
unsigned char CNTD_TBL[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
                                0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E,
                                0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x89};
// bool GameOver = true; // 게임 종료 판단
bool b = true;
int play_time[4] = {0,0,0,0};
unsigned char fnd_num[4];
//char score_text[10] = "Score: "; // 점수 clcd
int time_show = 30;              // 시간 제한 초기화 값
int count_timer = 0;             // 1s동안 보이게 만들기 위한 값

// 카운트다운 0.2s*5 = 1s동안 time_show 유지
void fnd_control(int fnd_d) {
    int tenth = time_show / 10;

    if (time_show == 0) {
        b = false;
    } else if (count_timer != 200000 && time_show > 0) {
        play_time[2] = tenth;
        play_time[3] = time_show - tenth * 10;

        count_timer++;
    } else if (count_timer == 200000 && time_show > 0) {
        play_time[2] = tenth;
        play_time[3] = time_show - tenth * 10;

        time_show--;
        count_timer = 1;
    }
    else { } //pass
    fnd_num[0] = CNTD_TBL[play_time[0]];
    fnd_num[1] = CNTD_TBL[play_time[1]];
    fnd_num[2] = CNTD_TBL[play_time[2]];
    fnd_num[3] = CNTD_TBL[play_time[3]];
    write(fnd_d, &fnd_num, sizeof(fnd_num));
}

// 점수 출력용
char *score_count(int total_score, char score_text[]) {
    char score[3]; // 점수용 string

    sprintf(score, "%d", total_score);
    strcat(score_text, score);
    
    return score_text;
}

// clcd 출력
void clcd_text(char clcd_text[]) {
    int clcd_d;
    
    clcd_d = open(clcd, O_RDWR);
    if (clcd_d < 0) {
        printf("clcd error\n");
    } // 예외처리

    write(clcd_d, clcd_text, strlen(clcd_text));
    close(clcd_d);
}

//최종배열을 만드는 함수
int setMatrix(char d1[], char d2[], int d) // d1이 장애물
{
    int h;
    int b = 1;
    for (h = 0; h < 8; h++) {
        if ((d2[h] & d1[h]) > 0) //비트연산자로 충돌하면 끝
        {
            b = false;
            break;
        }
        matrix[h] = d1[h] + d2[h]; //매트릭스 합쳐서 출력할 매트릭스 만든것
    }
    write(d, &matrix, sizeof(matrix));
    return b;
}

int main() {
    struct timeval dotstart, dotend, tactstart, tactend, obstaclestart,obstacleend, obstacle_spst, obstacle_spend, timest, fndstart, fndend, clcdstart, clcdend;
    int dot_d = 0;
    int tact = 0;
    int fnd_d = 0;
    int clcd_d = 0;
    unsigned char t = 0;
    //bool b = true; // Gameover
    int k;
    double obstacle_p; //장애물 위치
    int obstacle_n, i; //장애물 길이
    int total_score = 0;
    char score_text[10] = "Score: ";
    
    k = 10;
    srand(time(NULL)); //난수 생성
    
    gettimeofday(&obstaclestart, NULL);
    gettimeofday(&obstacle_spst, NULL);
    gettimeofday(&dotstart, NULL);
    gettimeofday(&timest, NULL);
    gettimeofday(&fndstart, NULL);
    // gettimeofday(&clcdstart, NULL);

    /*if (tact == 0) {
        tact = open(tact_d, O_RDWR);
    }
    gettimeofday(&tactstart, NULL);

    while (1) {
        clcd_text("Start key 2 ... left | - | right ");
        
        gettimeofday(&tactend, NULL);
        read(tact, &t, sizeof(t));
        
        if (t == 2) {
            printf("2");
            break;
            tact = close(tact);
        }
    }*/
    while (b) {
        //clcd_text(score_count(total_score, score_text)); // score print

        if (dot_d == 0) // dot matrix에 접근하지 않은 경우만 open
        {
            dot_d = open(dot, O_RDWR);
        }
        gettimeofday(&dotend, NULL);
        gettimeofday(&obstacleend, NULL);

        if (!setMatrix(obstacle, player, dot_d)) {
            break;
        }

        if ((obstacleend.tv_usec + 2000000 - obstaclestart.tv_usec > 2000000) && ((obstacleend.tv_sec - obstaclestart.tv_sec) == 2)) {
            obstacle_n = rand() % 1;
            if (obstacle_n == 0) //블록길이 1
            {
                obstacle_p = rand() % 7; //블록 생성 위치 결정
                obstacle[0] = pow(2.0, obstacle_p) + pow(2.0, obstacle_p + 1.0f); //정해진 위치에 블록 생성
            } else                                       //블록길이 3
            {
                obstacle_p = rand() % 5;
                obstacle[0] = pow(2.0, obstacle_p) + pow(2.0, obstacle_p + 1.0f) + pow(2.0, obstacle_p + 2.0f);
            }
            gettimeofday(&obstaclestart, NULL);
        }
        gettimeofday(&obstacle_spend, NULL);

        if ((obstacle_spend.tv_usec - obstacle_spst.tv_usec > 800000) || (obstacle_spend.tv_sec > obstacle_spst.tv_sec && (obstacle_spend.tv_usec + 1000000 - obstacle_spst.tv_usec > 800000))) {
            if (!setMatrix(obstacle, player, dot_d)) {
                break;
            }
            for (i = 6; i > -1; i--) //떨어지는 모습 구현
            {
                obstacle[i + 1] = obstacle[i];
            }
            if (obstacle[0] > 0) //블록 생성 전 삭제
            {
                obstacle[0] = 0;
            }
            gettimeofday(&obstacle_spst, NULL);
        }

    // dot matrix와 tack switch를 0.2초마다 번갈아가면서 접근
        if ((dotend.tv_usec - dotstart.tv_usec > 200000) || (dotend.tv_sec > dotstart.tv_sec && (dotend.tv_usec + 1000000 - dotstart.tv_usec > 200000))) {
            dot_d = close(dot_d);
            
            if (tact == 0) {
                tact = open(tact_d, O_RDWR);
    
            }
            gettimeofday(&tactstart, NULL);
    
            while (1) {
                gettimeofday(&tactend, NULL);
                read(tact, &t, sizeof(t)); // tact switch에 0.2초간 접근해있는 동안 입력받음
    
                switch (t) {
                    case 4:
                        if (player[6] != 0x40) // 4번 버튼 입력시 왼쪽으로 플레이어 이동
                        {
                            player[6] = player[6] << 1;
                            player[7] = player[7] << 1;
                        }
                        break;
                    case 6:
                        if (player[6] != 0x02) // 6번 버튼 입력시 오른쪽으로 플레이어 이동
                        {
                            player[6] = player[6] >> 1;
                            player[7] = player[7] >> 1;
                        }
                        break;
                }
    
            // 0.2초 경과 or tact switch 입력이 있는 경우 tact switch에 접근 해제
                if ((tactend.tv_usec - tactstart.tv_usec > 200000) ||(tactend.tv_sec > tactstart.tv_sec &&(tactend.tv_usec + 1000000 - tactstart.tv_usec > 200000)) ||t) {
                    tact = close(tact);
                    break;
                }
            }
    
            if (fnd_d == 0) {
                fnd_d = open(fnd, O_RDWR);
            }
            gettimeofday(&fndstart, NULL);
            while (1) {
                gettimeofday(&fndend, NULL);
                fnd_control(fnd_d);
                //fnd_control(fnd_d, fnd_num);
    
                // 0.2초 경과시 fnd에 접근 해제
                if ((fndend.tv_usec - fndstart.tv_usec > 200000) || (fndend.tv_sec > fndstart.tv_sec && (fndend.tv_usec + 1000000 - fndstart.tv_usec > 200000))) {
                    fnd_d = close(fnd_d);
                    break;
                }
            }
            gettimeofday(&dotstart, NULL);
        }
    }

    /*if (tact == 0) {
        tact = open(tact_d, O_RDWR);
    }
    gettimeofday(&tactstart, NULL);

    while (1) {
        clcd_text("Restart key 5 ..");
        
        gettimeofday(&tactend, NULL);
        read(tact, &t, sizeof(t));
        
        if (t == 5) {
            break;
            tact = close(tact);
            b = true;
        }
    }*/
}

