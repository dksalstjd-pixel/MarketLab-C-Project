#ifndef MARKET_H
#define MARKET_H

/* 마켓랩에서 함께 사용하는 설정값과 구조체 */
#define STOCK_COUNT 7
#define NEWS_COUNT 20
#define MAX_DAY 30
#define START_MONEY 1000000LL
#define TARGET_MONEY 1300000LL
#define MAX_TRADES 200
#define SAVE_FILE "marketlab_save_v7.dat"
#define MAX_DAILY_TRADES 3
#define FORECAST_COST 10000LL
#define MAX_FORECASTS 3

/* 한 회사의 현재 가격, 보유 수량, 과거 주가 */
typedef struct {
    char name[32], market[8];
    long price, history[MAX_DAY + 1];
    int owned;
    double avgPrice, change;
} Stock;

/* 매매 한 건: 실제 거래 내역 데이터 */
typedef struct {
    int day, index, count, buy; /* buy: 1=매수, 0=매도 */
    long price;
} Trade;

/* 연결 리스트 노드: 거래가 생길 때마다 동적 생성 */
typedef struct TradeNode {
    Trade data;
    struct TradeNode *next;
} TradeNode;

typedef struct {
    int day, news, title, body, note, tradeCount;
    long long cash;
    double realized;
    /* 게임 요소: 하루 거래 제한, 시장 전망, 미션 점수 */
    int movesToday, nextNews;
    int forecastsUsed, forecastShown, forecastGood;
    int score, mission[3]; /* -1=실패, 0=미진행, 1=성공 */
    Stock stocks[STOCK_COUNT];
    TradeNode *head; /* 최신 거래부터 연결 */
} Game;

/* market.c에 구현된 함수: main.c에서도 호출하므로 선언한다. */
void init_game(Game *g);
long long total_assets(const Game *g);
void next_day(Game *g);
int trade_stock(Game *g, int index, int count, int buy);
void free_trades(Game *g);
void show_market(const Game *g);
void show_portfolio(const Game *g);
void show_chart(const Game *g, int index);
void show_trades(const Game *g);
void show_forecast(Game *g);      /* 유료 시장 전망 카드 */
void show_missions(const Game *g); /* 미션 진행 현황 */
void show_news_article(const Game *g); /* 게임 내 뉴스 전문 */
int save_game(const Game *g);
int load_game(Game *g);
void color(int c);

#endif /* MARKET_H */
