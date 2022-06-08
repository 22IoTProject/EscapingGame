#include <asm/ioctls.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define dot "/dev/dot"
#define tact_d "/dev/tactsw"
#define fnd "/dev/fnd"
#define clcd "/dev/clcd"

unsigned char obstacle[8] = 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char player[8] = 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10}; // 2*1 형태의 플레이어
unsigned char matrix[8] = 
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //출력할 매트릭스
unsigned char CNTD_TBL[10] = 
    {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
int play_time[4] = {0,0,0,0}; // 카운트다운 용 배열
unsigned char fnd_num[4]; // fnd 용 배열
int time_show = 30; // 시간 제한 초기화 값
int count_timer = 0; // 시간 유지 시간
int total_score = 0; // 점수
char score_text[16]; // 점수 clcd
bool b = true; // 게임 종료 판단


// 카운트다운 함수, count_timer동안 time_show 유지
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

// 점수 반환 함수
char *score_count(int total_score) {
    char tmp[16] = "Score: ";
    char score[5] = ""; // 점수 string 만들기 위한 배열

    if (total_score > 999){
        strcpy(score, "999+");
    } else{
        sprintf(score, "%d", total_score); // itoa
    }
    strcat(tmp, score); // Score + total_score

    strcpy(score_text, tmp); // score_text = "Score: total_score"

    return score_text;
}

// clcd 출력 함수
void clcd_text(char clcd_text[]) {
    int clcd_d;
    
    clcd_d = open(clcd, O_RDWR);
    if (clcd_d < 0) {
        printf("clcd error\n");
    } // 예외처리

    write(clcd_d, clcd_text, strlen(clcd_text));
    close(clcd_d);
}

// 플레이어+장애물 배열 함수
int setMatrix(char d1[], char d2[], int d) // d1이 장애물
{
    int h;
    bool b = true;
    for (h = 0; h < 8; h++) {
        if ((d2[h] & d1[h]) > 0)
        {
            total_score--;
            if (total_score <0)
            {
                b = false;
            };
            break;
        }
        matrix[h] = d1[h] + d2[h]; //매트릭스 합쳐서 출력할 매트릭스 만든것
    }
    write(d, &matrix, sizeof(matrix));
    return b;
}

int main() {
    struct timeval dotstart, dotend, tactstart, tactend, obstaclestart,obstacleend, obstacle_spst, obstacle_spend, fndstart, fndend;
    int dot_d = 0;
    int tact = 0;
    int fnd_d = 0;
    int clcd_d = 0;
    unsigned char t = 0; // 입력받는 tact 값
    double obstacle_p; // 장애물 위치
    int obstacle_n, i; // 장애물 길이
    int exit = 1; // 재시작 안하면 exit
    char re[32]; // 재시작 문구 출력용
    
    srand(time(NULL)); // 매번 다른 장애물 출현하도록 rand
    
    gettimeofday(&obstaclestart, NULL);
    gettimeofday(&obstacle_spst, NULL);
    gettimeofday(&dotstart, NULL);

    while(exit){ // 재시작을 위한 while문
        // 시작 시 2번 키 입력
        if (tact == 0) {
            tact = open(tact_d, O_RDWR);
        }
        gettimeofday(&tactstart, NULL);

        // 안내 문구 출력
        clcd_text("Start key 2 ... left | - | right ");
        while (1) {
            gettimeofday(&tactend, NULL);
            read(tact, &t, sizeof(t));
            
            if (t == 2) { // 2번 버튼 입력 시 게임 시작 문구 출력
                printf("2\n");
                tact = close(tact);
                clcd_text("Hide Score..    Play Time!");
                break;
            }
        }
        while (b) {
            if (dot_d == 0) // dot matrix에 접근하지 않은 경우만 open
            {
                dot_d = open(dot, O_RDWR);
            }
            gettimeofday(&dotend, NULL);
            gettimeofday(&obstacleend, NULL);
    
            if (!setMatrix(obstacle, player, dot_d)) {
                break;
            }

            // 2초마다 장애물 생성
            if ((obstacleend.tv_usec + 2000000 - obstaclestart.tv_usec > 2000000) 
                && ((obstacleend.tv_sec - obstaclestart.tv_sec) == 2)) 
            {
                obstacle_n = rand() % 2;
                if (obstacle_n == 0) // 장애물 길이 2
                {
                    obstacle_p = rand() % 6; // 장애물 생성 위치 결정
                    obstacle[0] = pow(2.0, obstacle_p) + pow(2.0, obstacle_p + 1.0f); // 해당 위치에 장애물 생성
                } else // 장애물 길이 3
                {
                    obstacle_p = rand() % 5; 
                    obstacle[0] = pow(2.0, obstacle_p) 
                                    + pow(2.0, obstacle_p + 1.0f) 
                                    + pow(2.0, obstacle_p + 2.0f);
                }
                gettimeofday(&obstaclestart, NULL);
            }
            
            gettimeofday(&obstacle_spend, NULL);
            // 0.8초 간격으로 장애물이 내려올 수 있도록
            if ((obstacle_spend.tv_usec - obstacle_spst.tv_usec > 800000) 
                || (obstacle_spend.tv_sec > obstacle_spst.tv_sec 
                    && (obstacle_spend.tv_usec + 1000000 - obstacle_spst.tv_usec > 800000))
            ) {
                if (!setMatrix(obstacle, player, dot_d)) {
                    break;
                } else { 
                    for (i = 6; i > -1; i--) // 장애물 떨어지도록
                    {
                        obstacle[i + 1] = obstacle[i];
                    }
                }
                if (obstacle[0] > 0) // 장애물 초기화
                {
                    obstacle[0] = 0;
                }
                gettimeofday(&obstacle_spst, NULL);
            }
            
    
        // dot matrix, tack switch, fnd를 0.2초마다 번갈아가면서 접근
            if ((dotend.tv_usec - dotstart.tv_usec > 200000) || (dotend.tv_sec > dotstart.tv_sec && (dotend.tv_usec + 1000000 - dotstart.tv_usec > 200000))) {
                dot_d = close(dot_d);
                
                if (tact == 0) {
                    tact = open(tact_d, O_RDWR);
                }
                gettimeofday(&tactstart, NULL);
        
                while (1) {
                    gettimeofday(&tactend, NULL);
                    read(tact, &t, sizeof(t)); 

                    if (t==1 || t==4 || t==7 || t==10){ // 왼쪽으로 플레이어 이동
                        if (player[6] != 0x80)
                        {
                            player[6] = player[6] << 1;
                            player[7] = player[7] << 1; 
                            total_score++;
                        }
                        break;
                    } else if (t==3 || t==6 || t==9 || t==12){ // 오른쪽으로 플레이어 이동
                        if (player[6] != 0x01) 
                        {
                            player[6] = player[6] >> 1;
                            player[7] = player[7] >> 1;
                            total_score++;
                        }
                        break;
                    }
        
                // 0.2초 경과 or tact switch 입력이 있는 경우 tact switch에 접근 해제
                    if ((tactend.tv_usec - tactstart.tv_usec > 200000) ||(tactend.tv_sec > tactstart.tv_sec &&(tactend.tv_usec + 1000000 - tactstart.tv_usec > 200000)) ||t) {
                        tact = close(tact);
                        break;               
                    }
                }
                gettimeofday(&dotstart, NULL);
    
                // 제한 시간 출력
                if (fnd_d == 0) {
                    fnd_d = open(fnd, O_RDWR);
                }
                gettimeofday(&fndstart, NULL);
                
                while (1) {
                    gettimeofday(&fndend, NULL);
                    fnd_control(fnd_d);
        
                    // 0.1초 경과시 fnd에 접근 해제
                    if ((fndend.tv_usec - fndstart.tv_usec > 100000) || (fndend.tv_sec > fndstart.tv_sec && (fndend.tv_usec + 1000000 - fndstart.tv_usec > 100000))) {
                        fnd_d = close(fnd_d);
                        break;
                    }
                }
            }
        }
        
        // 게임 종료 시 score 출력하고 restart 물어보기
        if (!b) {
            if (tact == 0) {
                tact = open(tact_d, O_RDWR);
            }
            gettimeofday(&tactstart, NULL);

            strcpy(re, "Restart key 5 ..");
            strcat(re, score_count(total_score)); // "restart + score"
            clcd_text(re);
            
            while (1) {
                gettimeofday(&tactend, NULL);
                read(tact, &t, sizeof(t));
                
                if (t == 5) { // 재시작 및 변수 초기화
                    printf("RE\n");
                    time_show = 30;
                    total_score = 0;
                    b = true;
                    tact = close(tact);
                    break;
                }
                else if (t == 8){ // 게임 종료
                    clcd_text("Exit Game");
                    printf("Game Over\n");
                    exit = 0;
                    tact = close(tact);
                    break;
                }
            }
        }
    }
}
