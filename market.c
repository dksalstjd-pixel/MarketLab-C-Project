#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "market.h"
#ifdef _WIN32
#include <windows.h>
#endif

/* 뉴스, 거래, 차트, 저장 기능 */
/* 뉴스별 종목 영향: effect 순서는 stocks의 순서와 같음 */
typedef struct {
    const char *titles[3];
    int effect[STOCK_COUNT];
    int good, icon;
} News;

static const News NEWS[NEWS_COUNT] = {
    /* 0~12: 실제 보도에서 소재를 가져온 사건 (가격 변동은 게임 규칙) */
    {{"엔비디아 관련 해외 정보기관 의혹 조사", "엔비디아, 외국 정보기관 접근 의혹 파장", "엔비디아 지배구조 관련 조사 보도"}, {-6,0,-1,-2,-2,0,0}, 0,0},
    {{"엔비디아 H20 대중국 수출 제한 강화", "미국, AI 칩 수출 허가 요건 강화", "H20 수출 규제에 반도체 업계 긴장"}, {-5,0,0,-2,-3,0,0}, 0,0},
    {{"SK하이닉스, HBM4 공급 비중 확대 관측", "엔비디아 HBM4 공급 관련 전망", "SK하이닉스 HBM4 공급 확대 보도"}, {2,0,0,0,6,0,0}, 1,0},
    {{"삼성전자 HBM4 생산 확대 계획 보도", "삼성전자, 차세대 HBM 공급 준비", "HBM4 양산 계획에 시장 관심"}, {2,0,0,5,1,0,0}, 1,0},
    {{"테슬라, 오스틴 로보택시 운행 개시", "테슬라 첫 유료 로보택시 시범 서비스", "테슬라 로보택시 시험 운행 시작"}, {0,5,0,0,0,0,0}, 1,1},
    {{"테슬라 로보택시 주행 문제 영상 보도", "로보택시 시범 운행 안전 논란", "테슬라 자율주행 안전성 우려 보도"}, {0,-5,0,0,0,0,0}, 0,1},
    {{"현대차그룹 미국 투자 계획 발표", "현대차그룹, 미국 생산 투자 확대", "현대차그룹 미국 210억달러 투자 발표"}, {0,0,0,0,0,5,0}, 1,1},
    {{"현대차, 미국 관세에 수익성 부담", "미국 자동차 관세 영향 우려 확대", "현대차, 관세 영향으로 이익률 목표 조정"}, {0,0,0,0,0,-5,0}, 0,1},
    {{"애플, 아이폰17 생산 확대 요청 보도", "아이폰17 초기 수요에 생산 확대 관측", "애플 신형 아이폰 공급 확대 소식"}, {0,0,5,1,0,0,0}, 1,2},
    {{"애플 아이폰17 중국 판매 호조 보도", "중국 아이폰 판매 성장세 주목", "아이폰17 중국 초기 판매 증가 조사"}, {0,0,4,0,0,0,0}, 1,2},
    {{"두산에너빌리티, 뉴스케일 SMR 부품 계약", "두산에너빌리티 SMR 핵심 부품 수주", "두산에너빌리티 원전 모듈 협력 확대"}, {0,0,0,0,0,0,6}, 1,3},
    {{"SK하이닉스, AI 메모리 수급 부족 전망", "SK하이닉스 미국 HBM 생산계획 발표", "HBM 수요 확대에 설비 투자 주목"}, {1,0,0,1,5,0,0}, 1,0},
    {{"대중국 AI 칩 규제 추가 강화 보도", "반도체 수출 통제 확대에 시장 주목", "중국 AI 칩 생산 제한 조치 강화"}, {-3,0,-1,-3,-2,0,0}, 0,0},
    /* 13~19: 금리, 시장 심리 등 일반적인 경제 상황 */
    {{"금리 인상 우려 확대", "금리 인상 가능성에 성장주 긴장", "금융시장 긴축 우려"}, {-3,-4,-3,-3,-3,-2,-2}, 0,4},
    {{"증시 투자심리 회복 기대", "시장 전반에 매수세 확대 전망", "글로벌 증시 반등 기대"}, {3,3,3,3,3,2,2}, 1,4},
    {{"글로벌 경기 둔화 우려", "세계 경제 성장 둔화 경계", "수요 둔화에 기업 실적 경계"}, {-4,-4,-3,-4,-4,-4,-2}, 0,4},
    {{"국제 물류비 안정 기대", "글로벌 운송망 개선 전망", "공급망 회복 기대감"}, {0,0,1,0,0,2,2}, 1,4},
    {{"국제 물류 지연 가능성에 긴장", "운송 지연 우려 확대", "공급망 불안에 제조업계 경계"}, {0,0,-2,0,0,-3,-2}, 0,4},
    {{"미국 기술주 실적 기대 확대", "AI 기업 성장 전망 부각", "기술주 투자심리 개선"}, {4,2,3,2,3,0,0}, 1,0},
    {{"기술주 차익 실현 움직임", "성장주 과열 우려 확산", "기술기업 주가 조정 가능성"}, {-4,-2,-3,-2,-3,0,0}, 0,0}
};

/* 각 뉴스의 분야와 기사 본문. 사건 20개와 같은 순서로 대응한다. */
typedef struct {
    const char *section;   /* 예: 산업 / 반도체 */
    const char *lead;      /* 기사 첫 문단 */
    const char *outlook;   /* 기사 뒷부분의 시장 해설 */
} Story;

/* 실제 보도에 기반한 사건도 게임에서 '다음 날' 무작위 순서로 재생된다.
 * 실제 보도 날짜와 게임 진행 날짜는 관련이 없다.
 * 출처명만 간단히 표기하고 상세 주소는 README에 정리한다.
 */
typedef struct {
    int real;               /* 1=실제 보도 참고, 0=가상 사건 */
    const char *reported;   /* 과거 보도 날짜 */
    const char *source;     /* 참고한 기사/공시 출처 */
} NewsBasis;

static const NewsBasis BASIS[NEWS_COUNT] = {
    {1,"2026-11-19","로이터 (당국 조사·의혹)"},
    {1,"2025-04-15","엔비디아 공시 / 로이터"},
    {1,"2026-01-28","연합뉴스 (공급 비중 관측)"},
    {1,"2026-01-26","로이터 (생산 계획 보도)"},
    {1,"2025-06-22","로이터 (시험 서비스)"},
    {1,"2025-06-25","로이터 (주행 문제 보도)"},
    {1,"2025-03-24","연합뉴스 (투자 발표)"},
    {1,"2025-09","로이터 (관세 및 수익성)"},
    {1,"2025-09-19","로이터 (생산 확대 요청 보도)"},
    {1,"2025-11","로이터 (판매 조사 보도)"},
    {1,"2026-10-29","연합뉴스 / 뉴스케일"},
    {1,"2026-08-27","로이터 (전망 및 계획)"},
    {1,"2026-11-19","로이터 (규제 관련 보도)"},
    {0,"-","시장 전망"},
    {0,"-","시장 전망"},
    {0,"-","시장 전망"},
    {0,"-","시장 전망"},
    {0,"-","시장 전망"},
    {0,"-","시장 전망"},
    {0,"-","시장 전망"}
};

static const Story STORIES[NEWS_COUNT] = {
    {"기업 / 엔비디아", "엔비디아와 관계된 인물을 둘러싼 외국 정보기관 연계 의혹과 관련 조사 움직임이 보도됐다.", "의혹의 사실관계가 확인되기 전까지 관련 기업을 둘러싼 불확실성은 이어질 전망이다."},
    {"정책 / AI 반도체", "미국 정부가 엔비디아의 H20 등 일부 AI 반도체의 중국 수출에 추가 허가 요건을 부과했다.", "중국 시장 판매 제약이 반도체 기업의 매출과 공급 전략에 미칠 영향이 관심사다."},
    {"산업 / HBM", "엔비디아 차세대 AI 플랫폼용 HBM4 수요에서 SK하이닉스의 공급 비중이 커질 수 있다는 업계 관측이 나왔다.", "고성능 메모리 수요와 생산 능력이 주요 반도체 기업의 경쟁력을 가를 변수로 꼽힌다."},
    {"산업 / 반도체", "삼성전자가 차세대 고대역폭 메모리 HBM4 생산 확대와 공급 준비에 나섰다는 보도가 나왔다.", "차세대 제품 공급 일정과 실제 고객사 확보 여부에 시장의 관심이 쏠린다."},
    {"기술 / 자율주행", "테슬라가 미국 오스틴에서 제한된 범위의 유료 로보택시 서비스를 시작했다.", "자율주행 서비스의 확대 속도와 운영 안정성이 향후 사업 성과를 좌우할 전망이다."},
    {"기술 / 자동차", "테슬라 로보택시 시범 운행 과정에서 교통 관련 문제가 담긴 영상이 보도됐다.", "안전성 논란이 확산되면서 자율주행 기술의 신뢰성과 규제 대응이 주요 변수로 떠올랐다."},
    {"산업 / 자동차", "현대차그룹이 미국 내 생산과 미래 사업을 위한 210억 달러 규모의 투자 계획을 발표했다.", "현지 생산 확대가 공급망과 관세 부담에 어떤 영향을 줄지 업계가 주목하고 있다."},
    {"정책 / 자동차", "현대차의 미국 관세 부담과 수익성 전망을 둘러싼 우려가 커졌다는 보도가 나왔다.", "관세 수준과 현지 생산 전략이 자동차 업체의 이익률을 좌우할 주요 변수로 꼽힌다."},
    {"소비 / IT", "애플의 아이폰17 초기 주문 수요와 관련해 일부 공급업체에 생산 확대를 요청했다는 보도가 나왔다.", "신제품 판매 흐름이 애플뿐 아니라 부품 공급업체의 실적에도 영향을 미칠 전망이다."},
    {"소비 / 스마트폰", "아이폰17 출시 이후 중국 시장에서 아이폰 판매가 증가했다는 시장조사 결과가 전해졌다.", "중국 시장의 회복세가 지속될지와 경쟁 업체의 대응에 관심이 모이고 있다."},
    {"에너지 / SMR", "두산에너빌리티가 미국 뉴스케일파워와 소형모듈원전 관련 주요 부품 공급 계약을 체결했다.", "SMR 사업의 발주 확대와 실제 납품 일정이 향후 실적의 핵심 관전 요소다."},
    {"산업 / 메모리", "SK하이닉스의 미국 HBM 생산 계획과 중장기 AI 메모리 수급 전망이 주목받고 있다.", "수요 증가 전망 속에서 생산능력 확충과 공급 안정성이 주요 쟁점으로 떠올랐다."},
    {"정책 / 반도체", "미국의 대중국 AI 칩 수출 및 생산 규제가 강화될 수 있다는 소식에 반도체 업계가 촉각을 세우고 있다.", "관련 기업들은 규제 적용 범위와 고객사별 공급 계획을 점검하는 분위기다."},
    {"경제 / 금리", "금리 인상 가능성이 부각되면서 시장에서 성장주에 대한 경계심이 커지고 있다.", "자금 조달 비용이 높아질 경우 성장 기업의 가치 평가에 부담이 될 수 있다."},
    {"증시 / 시장", "글로벌 주식시장에 매수세가 유입될 가능성이 제기되고 있다.", "투자심리가 개선되더라도 업종별 실적 전망에 따라 주가 흐름은 달라질 수 있다."},
    {"경제 / 경기", "세계 경제의 성장 둔화 우려가 기업들의 투자와 소비 전망에 부담으로 작용하고 있다.", "경기에 민감한 업종을 중심으로 매출 전망과 비용 부담을 다시 살펴보는 분위기다."},
    {"물류 / 공급망", "국제 물류비 안정과 운송망 회복 가능성이 제조업계의 관심사로 떠올랐다.", "운송 비용이 안정되면 수출과 부품 조달에 의존하는 기업들의 부담이 줄어들 수 있다."},
    {"물류 / 공급망", "국제 운송 지연 가능성으로 주요 제조업체의 공급 일정에 대한 우려가 커지고 있다.", "원자재와 부품 조달에 차질이 생기면 생산 비용과 출하 일정에 영향을 줄 수 있다."},
    {"시장 / 기술주", "대형 기술기업들의 실적 개선 기대가 높아지면서 AI와 클라우드 관련 업종이 관심을 받고 있다.", "시장에서는 매출 성장세가 이어질지와 설비투자 부담이 얼마나 커질지를 주시하고 있다."},
    {"시장 / 기술주", "최근 기술주 상승 이후 차익 실현 매물이 늘어날 수 있다는 경계감이 확산되고 있다.", "상승폭이 컸던 종목을 중심으로 단기 가격 조정 가능성이 제기된다."}
};

/* 뉴스 영향이 큰 종목 두 개를 세로로 표시 */
static void show_related(const Game *g) {
    int i, first = -1, second = -1;
    int firstImpact = 0, secondImpact = 0;

    if (g->news < 0) return;

    for (i = 0; i < STOCK_COUNT; i++) {
        int impact = abs(NEWS[g->news].effect[i]);
        if (impact > firstImpact) {
            second = first;
            secondImpact = firstImpact;
            first = i;
            firstImpact = impact;
        } else if (impact > secondImpact) {
            second = i;
            secondImpact = impact;
        }
    }

    puts("  [관련 종목]");
    if (first >= 0) {
        printf("  %d. %s  ", first + 1, g->stocks[first].name);
        color(g->stocks[first].change > 0 ? 12 :
              g->stocks[first].change < 0 ? 9 : 7);
        printf("%s %+.1f%%\n", g->stocks[first].change > 0 ? "▲" :
                                  g->stocks[first].change < 0 ? "▼" : "-",
                                  g->stocks[first].change);
        color(7);
    }
    if (second >= 0) {
        printf("  %d. %s  ", second + 1, g->stocks[second].name);
        color(g->stocks[second].change > 0 ? 12 :
              g->stocks[second].change < 0 ? 9 : 7);
        printf("%s %+.1f%%\n", g->stocks[second].change > 0 ? "▲" :
                                  g->stocks[second].change < 0 ? "▼" : "-",
                                  g->stocks[second].change);
        color(7);
    }
}

static void news_picture(int icon); /* 아래에 구현된 그림 함수 선언 */

/* UTF-8 한글을 포함한 문장을 콘솔 너비에 맞춰 줄바꿈 */
static void print_news_text(const char *text) {
    const char *p = text;
    int column = 0;
    printf("  ");
    while (*p) {
        const char *end = p;
        int width = 0;
        while (*end && *end != ' ') {
            unsigned char c = (unsigned char)*end;
            int bytes = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
            width += bytes >= 3 ? 2 : 1;
            end += bytes;
        }
        if (column > 0 && column + 1 + width > 54) {
            printf("\n  ");
            column = 0;
        }
        if (column > 0) { putchar(' '); column++; }
        printf("%.*s", (int)(end - p), p);
        column += width;
        p = end;
        while (*p == ' ') p++;
    }
    putchar('\n');
}

/*
 * 뉴스 전문 화면: 기사와 내 보유 주식의 당일 변화를 함께 표시한다.
 * 실제 기사와는 별개로 가격 변화는 next_day()에서 계산한다.
 */
void show_news_article(const Game *g) {
    const News *n;
    const Story *story;
    const NewsBasis *basis;
    long long dailyProfit = 0;
    int i, haveStock = 0;

    if (g->news < 0) {
        puts("\n첫 거래일에는 아직 뉴스가 없습니다.");
        return;
    }

    n = &NEWS[g->news];
    story = &STORIES[g->news];
    basis = &BASIS[g->news];

    color(11);
    puts("\n==========================================================");
    printf("  MARKET LAB NEWS                       DAY %02d\n", g->day);
    puts("==========================================================");
    color(7);

    printf("  %s", story->section);
    if (basis->real) printf("  |  %s  %s", basis->reported, basis->source);
    putchar('\n');
    puts("----------------------------------------------------------");

    color(n->good ? 12 : 9);
    printf("  [속보] %s\n", n->titles[g->title]);
    color(7);

    puts("\n  [기사 내용]");
    print_news_text(story->lead);
    puts("");
    print_news_text(story->outlook);

    puts("----------------------------------------------------------");
    show_related(g);

    /* 뉴스를 본 뒤 내 종목이 얼마나 움직였는지 확인할 수 있다. */
    if (g->day > 1) {
        for (i = 0; i < STOCK_COUNT; i++) {
            const Stock *s = &g->stocks[i];
            if (s->owned > 0) {
                haveStock = 1;
                dailyProfit += (long long)
                    (s->price - s->history[g->day - 1]) * s->owned;
            }
        }

        if (haveStock) {
            puts("----------------------------------------------------------");
            printf("  [보유 수량 기준 전일 대비 가격차] ");
            color(dailyProfit > 0 ? 12 : dailyProfit < 0 ? 9 : 7);
            printf("%+lld원\n", dailyProfit);
            color(7);
        }
    }
    puts("==========================================================");
}

/* Windows 콘솔 색상: 상승 빨강(12), 하락 파랑(9), 보유 초록(10) */
void color(int c) {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)c);
#else
    printf("\033[%dm", c == 12 ? 31 : c == 9 ? 34 : c == 10 ? 32 : 37);
#endif
}
static void money(long long n) {
    char s[32];
    int i, len;
    snprintf(s, sizeof(s), "%lld", n);
    len = (int)strlen(s);
    for (i = 0; i < len; i++) {
        putchar(s[i]);
        if (i < len - 1 && (len - i - 1) % 3 == 0) putchar(',');
    }
}
static void news_picture(int icon) {
    static const char *art[5][3] = {
        {"  +--------------+", "  |   AI CHIP    |", "  +--------------+"},
        {"     _________",     "  __/  AUTO   \\__", "  O             O"},
        {"    +---------+",   "    |  PHONE  |", "    +---------+"},
        {"       /|",        "      /_|  POWER", "        |"},
        {"    .----------.", "   (   MARKET   )", "    '----------'"}
    };
    int i;
    for (i = 0; i < 3; i++) puts(art[icon][i]);
}

/* 3. 초기화와 자산 계산 */
void init_game(Game *g) {
    const char *names[STOCK_COUNT] = {"엔비디아", "테슬라", "애플", "삼성전자", "SK하이닉스", "현대자동차", "두산에너빌리티"};
    const char *markets[STOCK_COUNT] = {"미국", "미국", "미국", "한국", "한국", "한국", "한국"};
    const long prices[STOCK_COUNT] = {260000,340000,280000,85000,220000,240000,65000};
    int i;
    memset(g, 0, sizeof(*g));
    g->day = 1;
    g->news = -1;
    g->nextNews = rand() % NEWS_COUNT; /* 내일 사용할 사건은 미리 선정 */
    g->cash = START_MONEY;
    for (i = 0; i < STOCK_COUNT; i++) {
        snprintf(g->stocks[i].name, sizeof(g->stocks[i].name), "%s", names[i]);
        snprintf(g->stocks[i].market, sizeof(g->stocks[i].market), "%s", markets[i]);
        g->stocks[i].price = g->stocks[i].history[1] = prices[i];
    }
}
long long total_assets(const Game *g) {
    long long sum = g->cash;
    int i;
    for (i = 0; i < STOCK_COUNT; i++) sum += (long long)g->stocks[i].price * g->stocks[i].owned;
    return sum;
}

/* 4. 뉴스, 투자 미션, 시장 전망: 다음 날 진행 */
static int held_types(const Game *g) {
    int i, count = 0;
    for (i = 0; i < STOCK_COUNT; i++)
        if (g->stocks[i].owned > 0) count++;
    return count;
}

/* 10일, 20일, 30일에 미션 성공 여부를 한 번 평가한다. */
static void check_missions(Game *g) {
    long long total = total_assets(g);
    if (g->day == 10) {
        g->mission[0] = (total >= 1020000LL && held_types(g) >= 1) ? 1 : -1;
        if (g->mission[0] == 1) g->score += 25;
    } else if (g->day == 20) {
        g->mission[1] = (total >= 1080000LL && held_types(g) >= 2) ? 1 : -1;
        if (g->mission[1] == 1) g->score += 35;
    } else if (g->day == MAX_DAY) {
        g->mission[2] = (total >= TARGET_MONEY) ? 1 : -1;
        if (g->mission[2] == 1) g->score += 100;
    }
}

/* 시장 전망을 살지 결정하는 선택 요소: 비용 1만원, 총 3회. */
void show_forecast(Game *g) {
    if (g->day >= MAX_DAY) { puts("게임이 이미 종료되었습니다."); return; }
    if (g->forecastShown) {
        puts("[전망 카드] 오늘 구매한 리포트는 이미 확인했습니다.");
    } else {
        if (g->forecastsUsed >= MAX_FORECASTS) {
            puts("전망 카드는 게임 전체에서 3번까지 구매할 수 있습니다."); return;
        }
        if (g->cash < FORECAST_COST) {
            puts("현금이 부족해 시장 전망 카드를 구매할 수 없습니다."); return;
        }
        g->cash -= FORECAST_COST;
        g->forecastsUsed++;
        g->forecastShown = 1;
        /* 약 75%는 실제 다음 뉴스 방향, 약 25%는 반대 방향 */
        g->forecastGood = NEWS[g->nextNews].good;
        if (rand() % 4 == 0) g->forecastGood = !g->forecastGood;
    }
    puts("\n================== 애널리스트 시장 전망 ==================");
    printf(" [내일 관심 업종] %s\n", STORIES[g->nextNews].section);
    color(g->forecastGood ? 12 : 9);
    printf(" [예상 분위기]   %s\n", g->forecastGood ? "긍정적" : "부정적");
    color(7);
    printf(" 남은 이용 횟수: %d회 / 예측 정확도 약 75%%\n", MAX_FORECASTS - g->forecastsUsed);
    puts("=======================================================");
}

/* 10일, 20일, 30일 단계별 목표와 최종 등급 */
void show_missions(const Game *g) {
    const char *names[3] = {
        "10일: 총자산 102만원 + 1종목 이상 보유 (25점)",
        "20일: 총자산 108만원 + 2종목 이상 보유 (35점)",
        "30일: 총자산 130만원 달성 (100점)"
    };
    int i;
    puts("\n========== 투자 챌린지 미션 ========== ");
    for (i = 0; i < 3; i++) {
        printf(" %d. %s  [%s]\n", i+1, names[i],
            g->mission[i] == 1 ? "성공" : g->mission[i] == -1 ? "실패" : "도전 중");
    }
    printf(" 현재 점수: %d / 160점 | 하루 거래: %d/%d회\n", g->score, g->movesToday, MAX_DAILY_TRADES);
    if (g->day == MAX_DAY) {
        long long total = total_assets(g);
        const char *rank = total >= TARGET_MONEY && g->score >= 100 ? "S" :
            total >= TARGET_MONEY ? "A" : total >= START_MONEY ? "B" : "C";
        printf(" [최종 투자 등급] %s 등급 / 최종 총자산 %lld원\n", rank, total);
    }
}

void next_day(Game *g) {
    int id, i;
    if (g->day >= MAX_DAY) return;
    id = g->nextNews; /* 미리 정해둔 실제 또는 가상 뉴스 */
    g->news = id;
    g->title = rand() % 3;
    g->body = rand() % 3;
    g->note = rand() % 3;
    g->day++;
    g->movesToday = 0; /* 날짜 변경 시 하루 거래 횟수 초기화 */
    g->forecastShown = 0;
    do { g->nextNews = rand() % NEWS_COUNT; } while (g->nextNews == g->news);
    for (i = 0; i < STOCK_COUNT; i++) {
        Stock *s = &g->stocks[i]; /* 구조체 포인터 */
        long old = s->price;
        int rate = (rand() % 5 - 2) + NEWS[id].effect[i];
        long next = (long)((long long)old * (100 + rate) / 100);
        if (next < 100) next = 100;
        s->price = next;
        s->change = 100.0 * (next - old) / old;
        s->history[g->day] = next;
    }
    check_missions(g);
}

/* 5. 연결 리스트: 거래 내역을 새 노드로 기록 */
static int add_trade(Game *g, int index, int count, int buy) {
    TradeNode *node;
    if (g->tradeCount >= MAX_TRADES) return 0;
    node = (TradeNode *)malloc(sizeof(*node));
    if (!node) return 0;
    node->data.day = g->day;
    node->data.index = index;
    node->data.count = count;
    node->data.buy = buy;
    node->data.price = g->stocks[index].price;
    node->next = g->head;   /* 가장 최근 거래가 맨 앞 */
    g->head = node;
    g->tradeCount++;
    return 1;
}
void free_trades(Game *g) {
    TradeNode *p = g->head;
    while (p) {
        TradeNode *next = p->next;
        free(p);
        p = next;
    }
    g->head = NULL;
    g->tradeCount = 0;
}
int trade_stock(Game *g, int index, int count, int buy) {
    Stock *s;
    long long cost;
    if (g->day >= MAX_DAY || g->movesToday >= MAX_DAILY_TRADES ||
        index < 0 || index >= STOCK_COUNT || count <= 0 || count > 10000) return 0;
    s = &g->stocks[index];
    cost = (long long)s->price * count;
    if ((buy && cost > g->cash) || (!buy && count > s->owned)) return 0;
    if (!add_trade(g, index, count, buy)) return 0;
    g->movesToday++;
    if (buy) {
        s->avgPrice = (s->avgPrice * s->owned + (double)cost) / (s->owned + count);
        s->owned += count;
        g->cash -= cost;
    } else {
        g->realized += (s->price - s->avgPrice) * count;
        s->owned -= count;
        g->cash += cost;
        if (s->owned == 0) s->avgPrice = 0;
    }
    return 1;
}

/* 6. 메인 시세 화면: 상승=빨강, 하락=파랑, 보유=초록 */
void show_market(const Game *g) {
    int i;
    long long total = total_assets(g);
    color(11);
    puts("================================================================");
    puts("           MARKET LAB  |  글로벌 주식 투자 챌린지");
    puts("================================================================");
    color(7);
    printf(" %02d/%02d일  |  현금 ", g->day, MAX_DAY);
    money(g->cash);
    printf("원  |  총자산 "); money(total);
    printf("원  |  수익률 %+.1f%%\n", 100.0 * (total - START_MONEY) / START_MONEY);
    printf(" 목표 총자산: "); money(TARGET_MONEY); puts("원");
    color(10);
    printf(" [INVEST CHALLENGE] 누적 %d점 / 160점  |  오늘 거래 %d/%d회\n",
        g->score, g->movesToday, MAX_DAILY_TRADES);
    color(7);
    if (g->day <= 10) puts(" [진행 미션] 10일차 총자산 102만원 + 1종목 이상 보유");
    else if (g->day <= 20) puts(" [진행 미션] 20일차 총자산 108만원 + 2종목 이상 보유");
    else puts(" [진행 미션] 30일차 총자산 130만원 달성");
    puts("----------------------------------------------------------------");
    if (g->news < 0) {
        puts(" [MARKET LAB NEWS]  시장 개장  |  첫 거래일");
        puts("  오늘은 새로운 뉴스가 없습니다.");
    } else {
        const News *n = &NEWS[g->news];
        const Story *story = &STORIES[g->news];
        printf(" [MARKET LAB NEWS] %02d일차 | %s | %s\n", g->day, story->section,
               BASIS[g->news].real ? "경제 뉴스" : "시장 전망");
        color(n->good ? 12 : 9);
        printf("  [속보] %s\n", n->titles[g->title]);
        color(7);
        news_picture(n->icon);
        /* 메인 화면은 속보만 보여주고, 기사 전문은 9번에서 확인한다. */
        puts("  [9] 기사 읽기 / 관련 종목 확인");
    }
    puts("----------------------------------------------------------------");
    for (i = 0; i < STOCK_COUNT; i++) {
        const Stock *s = &g->stocks[i];
        long diff = g->day == 1 ? 0 : s->price - s->history[g->day - 1];
        printf(" %d. %s [%s]  ", i + 1, s->name, s->market);
        color(diff > 0 ? 12 : diff < 0 ? 9 : 7);
        printf("%s ", diff > 0 ? "▲" : diff < 0 ? "▼" : "-");
        money(s->price);
        printf("원 (%+ld원 / %+.1f%%)", diff, s->change);
        color(7);
        printf("  |  ");
        color(s->owned ? 10 : 8);
        printf("보유 %d주\n", s->owned);
        color(7);
    }
    puts("----------------------------------------------------------------");
    puts(" [1]매수 [2]매도 [3]다음 날 [4]차트 [5]자산 현황");
    puts(" [6]거래 내역 [7]저장 [8]불러오기 [9]뉴스 기사");
    puts(" [10]시장 전망 카드 (1만원, 최대 3회) [11]미션/등급 [0]종료");
    if (g->day == MAX_DAY) {
        color(total >= TARGET_MONEY ? 10 : 14);
        printf(" [30일 종료] %s / 최종 자산 ", total >= TARGET_MONEY ? "목표 달성" : "목표 미달성");
        money(total); puts("원"); color(7);
        show_missions(g);
    }
    puts("================================================================");
}
void show_portfolio(const Game *g) {
    int i;
    printf("\n[내 자산] 현금 "); money(g->cash); printf("원 | 실현손익 %+.0f원\n", g->realized);
    for (i = 0; i < STOCK_COUNT; i++) {
        const Stock *s = &g->stocks[i];
        double profit = (s->price - s->avgPrice) * s->owned;
        if (!s->owned) continue;
        printf(" %s | %d주 | 평균 %.0f원 | 평가손익 ", s->name, s->owned, s->avgPrice);
        color(profit > 0 ? 12 : profit < 0 ? 9 : 7);
        printf("%+.0f원\n", profit); color(7);
    }
    printf("총자산: "); money(total_assets(g)); puts("원");
}
/* 2차원 배열로 최근 16일간의 주가 선 그래프를 그린다.
 * 실제 OHLC 봉차트가 아닌 종가 기준 단순 선형 차트다.
 */
void show_chart(const Game *g, int index) {
    enum { HEIGHT = 11, MAX_POINTS = 16, STEP = 3, WIDTH = (MAX_POINTS - 1) * STEP + 1 };
    char canvas[HEIGHT][WIDTH];
    int segmentColor[WIDTH];
    int first, count, x, y, i, j, r;
    long low, high;
    const Stock *s;

    if (index < 0 || index >= STOCK_COUNT) {
        puts("잘못된 종목 번호입니다.");
        return;
    }
    s = &g->stocks[index];
    count = g->day < MAX_POINTS ? g->day : MAX_POINTS;
    first = g->day - count + 1;
    low = high = s->history[first];
    for (i = first + 1; i <= g->day; i++) {
        if (s->history[i] < low) low = s->history[i];
        if (s->history[i] > high) high = s->history[i];
    }
    memset(canvas, ' ', sizeof(canvas));
    for (x = 0; x < WIDTH; x++) segmentColor[x] = 7;

    /* 날짜별 종가를 0~10행의 세로 좌표로 변환 */
    for (i = 0; i < count; i++) {
        int row;
        long price = s->history[first + i];
        x = i * STEP;
        row = (high == low) ? HEIGHT / 2
            : (int)((long long)(high - price) * (HEIGHT - 1) / (high - low));
        canvas[row][x] = '*';
    }

    /* 두 날짜의 점을 '/', '\\', '-'로 이어 선 그래프 생성 */
    for (i = 0; i < count - 1; i++) {
        long p1 = s->history[first + i];
        long p2 = s->history[first + i + 1];
        int y1 = (high == low) ? HEIGHT / 2
            : (int)((long long)(high - p1) * (HEIGHT - 1) / (high - low));
        int y2 = (high == low) ? HEIGHT / 2
            : (int)((long long)(high - p2) * (HEIGHT - 1) / (high - low));
        int lineColor = p2 > p1 ? 12 : p2 < p1 ? 9 : 7;
        char ch = y2 < y1 ? '/' : y2 > y1 ? '\\' : '-';
        for (j = 1; j <= STEP; j++) {
            x = i * STEP + j;
            y = y1 + (y2 - y1) * j / STEP;
            segmentColor[x] = lineColor;
            if (canvas[y][x] == ' ') canvas[y][x] = ch;
        }
        canvas[y2][(i + 1) * STEP] = '*';
    }

    printf("\n[%s] 최근 %d거래일 종가 선 그래프 (단위: 원)\n", s->name, count);
    printf(" 최고가 %ld원 / 최저가 %ld원\n", high, low);
    for (r = 0; r < HEIGHT; r++) {
        long label = high - (long)((long long)(high - low) * r / (HEIGHT - 1));
        printf(" %8ld |", label);
        for (x = 0; x < count * STEP - STEP + 1; x++) {
            color(canvas[r][x] != ' ' ? segmentColor[x] : 7);
            putchar(canvas[r][x]);
        }
        color(7);
        putchar('\n');
    }
    printf("          +");
    for (x = 0; x < count * STEP - STEP + 1; x++) putchar('-');
    printf("\n           ");
    for (i = 0; i < count; i++) printf("%02d ", first + i);
    printf("\n");
    puts(" * = 하루의 종가 / 빨강 = 상승 / 파랑 = 하락");
}
void show_trades(const Game *g) {
    const TradeNode *p = g->head;
    int count = 0;
    puts("\n[최근 거래 내역 / 연결 리스트]");
    if (!p) puts("아직 거래가 없습니다.");
    while (p && count++ < 15) {
        const Trade *t = &p->data;
        printf(" %02d일 | %s | %s | %d주 x %ld원\n", t->day,
               t->buy ? "매수" : "매도", g->stocks[t->index].name, t->count, t->price);
        p = p->next;
    }
}

/* 7. 파일 저장: 포인터 주소는 저장하지 않고 데이터만 기록 */
typedef struct {
    char magic[8];
    int day, news, title, body, note, tradeCount;
    long long cash;
    double realized;
    Stock stocks[STOCK_COUNT];
    int movesToday, nextNews, forecastsUsed, forecastShown, forecastGood;
    int score, mission[3];
} SaveData;

int save_game(const Game *g) {
    FILE *fp = fopen(SAVE_FILE, "wb");
    SaveData data = {0};
    const TradeNode *p;
    int ok;
    if (!fp) return 0;
    memcpy(data.magic, "MLABv07", 8);
    data.day = g->day; data.news = g->news; data.title = g->title;
    data.body = g->body; data.note = g->note; data.tradeCount = g->tradeCount;
    data.cash = g->cash; data.realized = g->realized;
    memcpy(data.stocks, g->stocks, sizeof(data.stocks));
    data.movesToday = g->movesToday;
    data.nextNews = g->nextNews;
    data.forecastsUsed = g->forecastsUsed;
    data.forecastShown = g->forecastShown;
    data.forecastGood = g->forecastGood;
    data.score = g->score;
    memcpy(data.mission, g->mission, sizeof(data.mission));
    ok = fwrite(&data, sizeof(data), 1, fp) == 1;
    for (p = g->head; p && ok; p = p->next)
        ok = fwrite(&p->data, sizeof(Trade), 1, fp) == 1;
    if (fclose(fp) != 0) ok = 0;
    return ok;
}
int load_game(Game *g) {
    FILE *fp = fopen(SAVE_FILE, "rb");
    SaveData data;
    Game temp = {0};
    TradeNode *tail = NULL;
    int i, ok;
    if (!fp) return 0;
    ok = fread(&data, sizeof(data), 1, fp) == 1 &&
         memcmp(data.magic, "MLABv07", 8) == 0;
    if (ok) {
        ok = data.day >= 1 && data.day <= MAX_DAY && data.cash >= 0 &&
             data.news >= -1 && data.news < NEWS_COUNT &&
             data.title >= 0 && data.title < 3 &&
             data.body >= 0 && data.body < 3 &&
             data.note >= 0 && data.note < 3 &&
             data.tradeCount >= 0 && data.tradeCount <= MAX_TRADES &&
             data.movesToday >= 0 && data.movesToday <= MAX_DAILY_TRADES &&
             data.nextNews >= 0 && data.nextNews < NEWS_COUNT &&
             data.forecastsUsed >= 0 && data.forecastsUsed <= MAX_FORECASTS &&
             data.forecastShown >= 0 && data.forecastShown <= 1 &&
             data.forecastGood >= 0 && data.forecastGood <= 1 &&
             data.score >= 0 && data.score <= 160;
        for (i = 0; i < 3 && ok; i++)
            if (data.mission[i] < -1 || data.mission[i] > 1) ok = 0;
    }
    for (i = 0; ok && i < STOCK_COUNT; i++)
        if (data.stocks[i].price < 100 || data.stocks[i].owned < 0 ||
            data.stocks[i].avgPrice < 0) ok = 0;
    for (i = 0; ok && i < data.tradeCount; i++) {
        Trade record;
        TradeNode *node;
        if (fread(&record, sizeof(record), 1, fp) != 1 ||
            record.day < 1 || record.day > data.day ||
            record.index < 0 || record.index >= STOCK_COUNT ||
            record.count <= 0 || record.price < 100 ||
            (record.buy != 0 && record.buy != 1)) { ok = 0; break; }
        node = (TradeNode *)malloc(sizeof(*node));
        if (!node) { ok = 0; break; }
        node->data = record; node->next = NULL;
        if (tail) tail->next = node;
        else temp.head = node;
        tail = node;
        temp.tradeCount++;
    }
    fclose(fp);
    if (ok) {
        free_trades(g);  /* 검증을 통과했을 때만 기존 목록 삭제 */
        g->day = data.day; g->news = data.news; g->title = data.title;
        g->body = data.body; g->note = data.note; g->cash = data.cash;
        g->realized = data.realized; g->tradeCount = temp.tradeCount;
        g->movesToday = data.movesToday;
        g->nextNews = data.nextNews;
        g->forecastsUsed = data.forecastsUsed;
        g->forecastShown = data.forecastShown;
        g->forecastGood = data.forecastGood;
        g->score = data.score;
        memcpy(g->mission, data.mission, sizeof(g->mission));
        memcpy(g->stocks, data.stocks, sizeof(g->stocks));
        g->head = temp.head;
    } else free_trades(&temp);
    return ok;
}

