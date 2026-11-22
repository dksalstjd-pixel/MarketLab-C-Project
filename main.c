#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "market.h"
#ifdef _WIN32
#include <windows.h>
#endif

/* 메뉴와 입력만 담당하고, 실제 주식 계산은 market.c에서 처리한다. */
static void clear_screen(void) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO b;
    DWORD written;
    COORD p = {0, 0};
    if (GetConsoleScreenBufferInfo(h, &b)) {
        DWORD n = (DWORD)b.dwSize.X * b.dwSize.Y;
        FillConsoleOutputCharacterA(h, ' ', n, p, &written);
        FillConsoleOutputAttribute(h, b.wAttributes, n, p, &written);
        SetConsoleCursorPosition(h, p);
    }
#else
    printf("\033[2J\033[H");
#endif
}
static int input_int(const char *msg) {
    char line[64], extra;
    int value;
    printf("%s", msg);
    if (!fgets(line, sizeof(line), stdin)) return -999;
    return sscanf(line, " %d %c", &value, &extra) == 1 ? value : -999;
}
static void pause_screen(void) {
    char line[8];
    printf("\nEnter를 누르면 돌아갑니다...");
    fgets(line, sizeof(line), stdin);
}
/* main(): 실행 및 사용자 메뉴 선택 */
int main(void) {
    Game game;
    int menu, index, count;
    srand((unsigned)time(NULL));
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    SetConsoleTitleW(L"MARKET LAB - 글로벌 주식 투자 게임");
#endif
    init_game(&game);
    for (;;) {
        clear_screen();
        show_market(&game);
        menu = input_int("메뉴 선택 > ");
        if (menu == 0) break;
        if (menu == 1 || menu == 2) {
            index = input_int("종목 번호(1~7) > ") - 1;
            count = input_int("거래 수량 > ");
            puts(trade_stock(&game, index, count, menu == 1)
                 ? "거래 완료!" : "거래 실패: 종목, 잔액 또는 오늘 거래 횟수(3회)를 확인하세요.");
        } else if (menu == 3) {
            if (game.day < MAX_DAY) { next_day(&game); puts("새 뉴스가 도착했습니다."); }
            else puts("이미 30일이 종료됐습니다.");
        } else if (menu == 4) {
            index = input_int("차트를 볼 종목(1~7) > ") - 1;
            show_chart(&game, index);
        } else if (menu == 5) show_portfolio(&game);
        else if (menu == 6) show_trades(&game);
        else if (menu == 7) puts(save_game(&game) ? "저장 완료!" : "저장 실패!");
        else if (menu == 8) puts(load_game(&game) ? "불러오기 완료!" : "저장 파일이 없거나 잘못됐습니다.");
        else if (menu == 9) show_news_article(&game);
        else if (menu == 10) show_forecast(&game);
        else if (menu == 11) show_missions(&game);
        else puts("0~11 중 하나를 입력하세요.");
        pause_screen();
    }
    free_trades(&game); /* 종료 전에 malloc 공간 반환 */
    color(7);
    puts("게임을 종료합니다.");
    return 0;
}
