#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>
#include <ctype.h>

// 클리어 명령어 정의
#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

// 상수 정의
#define MAX_LINE_LENGTH 100
#define MAX_ORDERS 100
#define MAX_ITEMS 20
#define MAX_MENU_ITEMS 21

// 제품 구조체 정의
typedef struct {
    char name[20];
    int qty;
    char size[3];
    char temperature[10];
    int price;
} product;

// 메뉴 항목 구조체 정의
typedef struct {
    char name[20];
    int price_s;
    int price_m;
    int price_l;
} menu_item;

// 주문 구조체 정의
typedef struct {
    char name[20];
    int qty;
    char temperature[2];
    char size[2];
    int total_price;
    int discount;
    int payment;
    int month;
    int day;
    char time[9];
} Order;

// 전역 변수 정의
menu_item menu[MAX_ITEMS];
int coffee_item_count = 0;
int tea_item_count = 0;

// 함수 선언
void show_welcome_screen(WINDOW *win);
void order_program();
void load_menu(const char *filename);
int main_menu();
int menu_selection(int menu_type);
int select_temperature();
int select_size();
int coffee(int qty, char size[], int index);
int tea(int qty, char size[], int index);
int confirm_order(product pd[], int count);
int apply_coupon(product pd[], int count, int discount[]);
int card();
int print_receipt(product pd[], int count, int discount);
int write_csv(char *filename, product pd[], int count, int discount[]);
int select_yes_no(const char *prompt);
void admin_mode();
int read_csv(char* filename, Order orders[], int max_orders);
void get_monthly_data(int month, Order orders[], int *order_count);
void monthly_sales(Order orders[], int order_count, int month);
void sales_by_product(Order orders[], int order_count, char* product_name);
void ordered_selling_products(Order orders[], int order_count);
void ordered_revenue_products(Order orders[], int order_count);
void display_menu_items(WINDOW *win);
int display_orders(Order orders[], int order_count);
int admin_menu(WINDOW *win);
int admin_menu_selection();

int main() {
    initscr(); // curses 모드 시작
    cbreak(); // 입력을 즉시 처리하도록 설정
    noecho(); // 입력된 문자를 화면에 표시하지 않도록 설정
    keypad(stdscr, TRUE); // 키패드 활성화

    show_welcome_screen(stdscr); // 환영 화면 표시

    endwin(); // curses 모드 종료
    return 0;
}

void show_welcome_screen(WINDOW *win) {
    const char *welcome_message = "Welcome to Hwigyeong Station Cafe";
    const char *options[] = {"View Menu", "Administrator Mode"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    int width = 50;
    int height = 10;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *welcome_win = newwin(height, width, starty, startx);
    wborder(welcome_win, '|', '|', '=', '=', '+', '+', '+', '+'); // 테두리 설정
    keypad(welcome_win, TRUE); // 키패드 활성화

    while (1) {
        werase(welcome_win); // 창 내용 지우기
        wborder(welcome_win, '|', '|', '=', '=', '+', '+', '+', '+'); // 테두리 다시 설정

        int message_x = (width - strlen(welcome_message)) / 2;
        int message_y = 1;
        mvwprintw(welcome_win, message_y, message_x, welcome_message);

        for (int i = 0; i < num_options; i++) {
            int option_x = (width - strlen(options[i])) / 2;
            int option_y = 3 + i;
            if (i == choice) {
                wattron(welcome_win, A_REVERSE); // 선택된 옵션 반전
            }
            mvwprintw(welcome_win, option_y, option_x, options[i]);
            wattroff(welcome_win, A_REVERSE); // 선택된 옵션 반전 해제
        }
        wrefresh(welcome_win); // 창 새로 고침

        int ch = wgetch(welcome_win);
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10: // Enter key
                delwin(welcome_win); // 창 삭제
                if (choice == 0) {
                    order_program();
                    return;
                } else if (choice == 1) {
                        admin_mode();
                    return;
                }
        }
    }
}

// 주문 프로그램 시작 함수
void order_program() {
    load_menu("menu.csv"); // 메뉴 로드

    int menu_type, idx = 0;
    product pd[MAX_ITEMS] = { 0 };
    char input = 'Y';
    char confirm = 'N';
    int i = 0;
    int discount[MAX_ITEMS] = { 0 };

    while (1) {
        idx = 0;
        memset(pd, 0, sizeof(pd)); // 제품 배열 초기화
        memset(discount, 0, sizeof(discount)); // 할인 배열 초기화

        while (input == 'Y' || input == 'y') {
            clear();
            menu_type = main_menu(); // 메인 메뉴 표시 및 선택
            if (menu_type == -1) {
                continue;
            }

            int item = menu_selection(menu_type); // 메뉴 항목 선택
            if (item == -1) {
                printw("Invalid selection. Please try again.\n");
                continue;
            }

            strcpy(pd[idx].name, menu[item].name);

            clear();
            int temp_selection = select_temperature(); // 온도 선택
            if (temp_selection == -1) {
                continue;
            }
            if (temp_selection == 0) {
                strcpy(pd[idx].temperature, "I");
            } else if (temp_selection == 1) {
                strcpy(pd[idx].temperature, "H");
            }

            int size_selection = select_size(); // 사이즈 선택
            if (size_selection == -1) {
                continue;
            }
            if (size_selection == 0) {
                strcpy(pd[idx].size, "S");
            } else if (size_selection == 1) {
                strcpy(pd[idx].size, "M");
            } else if (size_selection == 2) {
                strcpy(pd[idx].size, "L");
            }

            while (1) {
                printw("Please enter the quantity: ");
                refresh();
                echo();
                char quantity_input[10];
                getstr(quantity_input);
                noecho();

                if (strlen(quantity_input) == 0) {
                    clear();
                    printw("Invalid input. Please enter a valid quantity.\n");
                    refresh();
                    continue;
                }

                int valid = 1;
                for (int i = 0; i < strlen(quantity_input); i++) {
                    if (!isdigit(quantity_input[i])) {
                        valid = 0;
                        break;
                    }
                }

                if (valid && strlen(quantity_input) > 0) {
                    pd[idx].qty = atoi(quantity_input);
                    if (pd[idx].qty > 0) {
                        break;
                    } else {
                        clear();
                        printw("Invalid input. Please enter a valid quantity.\n");
                        refresh();
                    }
                } else {
                    clear();
                    printw("Invalid input. Please enter a valid quantity.\n");
                    refresh();
                }
            }

            if (menu_type == 1) {
                pd[idx].price = coffee(pd[idx].qty, pd[idx].size, item); // 커피 가격 계산
            } else if (menu_type == 2) {
                pd[idx].price = tea(pd[idx].qty, pd[idx].size, item); // 티 가격 계산
            }

            idx++;
            clear();
            refresh();
            input = select_yes_no("Would you like to place an additional order?") ? 'Y' : 'N'; // 추가 주문 여부 확인
            refresh();
        }

        confirm = 'N';
        confirm = confirm_order(pd, idx); // 주문 확인

        if (confirm == 'N' || confirm == 'n') {
            printw("\n\nYour shopping cart has been reset. Here we go again.\n");
            input = 'Y';
            continue;
        }

        if (confirm == 'Y' || confirm == 'y') {
            char filename[50];

            int discount_total = apply_coupon(pd, idx, discount); // 쿠폰 적용

            if (card()) {
                write_csv(filename, pd, idx, discount); // 주문 기록 저장
                print_receipt(pd, idx, discount_total); // 영수증 출력
                printw("Press any key to exit...");
                getch();
            }
            break;
        }
    }
}

// 메뉴 파일을 로드하는 함수
void load_menu(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    int index = 0;
    int valid_lines = 0;

    while (valid_lines < 21 && fgets(line, sizeof(line), file)) {
        if (strlen(line) <= 1) {
            continue;
        }

        if (index > 0) {
            sscanf(line, "%19[^,],%d,%d,%d", menu[valid_lines].name, &menu[valid_lines].price_s, &menu[valid_lines].price_m, &menu[valid_lines].price_l);
            if (valid_lines < 10) {
                coffee_item_count++;
            } else {
                tea_item_count++;
            }
            valid_lines++;
        }
        index++;
    }

    fclose(file);
}

// 메인 메뉴를 표시하는 함수
int main_menu() {
    const char *options[] = {"Coffee", "Tea"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        int startx = (COLS - 20) / 2;
        int starty = (LINES - num_options) / 2;
        mvprintw(starty - 2, startx, "----------MENU----------");
        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                attron(A_REVERSE); // 선택된 옵션 반전
            }
            mvprintw(starty + i, startx, "%d. %s", i + 1, options[i]);
            if (i == choice) {
                attroff(A_REVERSE); // 선택된 옵션 반전 해제
            }
        }
        mvprintw(starty + num_options, startx, "------------------------");
        mvprintw(starty + num_options + 1, startx, "Choose the menu you want to see: ");
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10:
                return choice + 1; // 선택된 옵션 반환
        }
    }
}

// 메뉴 항목을 선택하는 함수
int menu_selection(int menu_type) {
    int start_idx = (menu_type == 1) ? 0 : coffee_item_count;
    int end_idx = (menu_type == 1) ? coffee_item_count : coffee_item_count + tea_item_count;
    int choice = start_idx;

    while (1) {
        clear();
        int width = 60;  // Increase width to accommodate longer text
        int height = end_idx - start_idx + 6;
        int startx = (COLS - width) / 2;
        int starty = (LINES - height) / 2;

        WINDOW *menu_win = newwin(height, width, starty, startx);
        wborder(menu_win, '|', '|', '-', '-', '+', '+', '+', '+');
        keypad(menu_win, TRUE);

        const char *prompt = "Choose a menu item (ESC to cancel)";
        int prompt_len = strlen(prompt);
        if (prompt_len > width - 4) {
            prompt_len = width - 4;
        }
        int prompt_x = (width - prompt_len) / 2;
        mvwprintw(menu_win, 1, prompt_x, "%.*s", prompt_len, prompt);

        int visible_item_count = 0;
        for (int i = start_idx; i < end_idx; i++) {
            if (strlen(menu[i].name) > 0) {
                if (i == choice) {
                    wattron(menu_win, A_REVERSE); // 선택된 항목 반전
                }
                mvwprintw(menu_win, 3 + visible_item_count, 2, "%d) %-20s S: %4dwon M: %4dwon L: %4dwon", visible_item_count + 1, menu[i].name, menu[i].price_s, menu[i].price_m, menu[i].price_l);
                if (i == choice) {
                    wattroff(menu_win, A_REVERSE); // 선택된 항목 반전 해제
                }
                visible_item_count++;
            }
        }

        wrefresh(menu_win);
        int ch = wgetch(menu_win);
        switch (ch) {
            case KEY_UP:
                if (choice > start_idx) {
                    choice--;
                } else {
                    choice = start_idx + visible_item_count - 1;
                }
                break;
            case KEY_DOWN:
                if (choice < start_idx + visible_item_count - 1) {
                    choice++;
                } else {
                    choice = start_idx;
                }
                break;
            case 10:
                delwin(menu_win);
                clear();
                refresh();
                return choice;

            case 27: // ESC key
                delwin(menu_win);
                clear();
                refresh();
                return -1; // ESC 키를 누르면 -1 반환
        }
    }
}

// 온도를 선택하는 함수
int select_temperature() {
    const char *options[] = {"Ice", "Hot"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        int width = 40;
        int height = 6;
        int startx = (COLS - width) / 2;
        int starty = (LINES - height) / 2;

        WINDOW *temp_win = newwin(height, width, starty, startx);
        wborder(temp_win, '|', '|', '-', '-', '+', '+', '+', '+');
        keypad(temp_win, TRUE);
        mvwprintw(temp_win, 1, (width - strlen("Choose temperature (ESC to cancel)")) / 2, "Choose temperature (ESC to cancel)");

        for (int i = 0; i < num_options; i++) {
            int option_x = (width - strlen(options[i])) / 2;
            if (i == choice) {
                wattron(temp_win, A_REVERSE); // 선택된 옵션 반전
            }
            mvwprintw(temp_win, 3 + i, option_x, "%s", options[i]);
            wattroff(temp_win, A_REVERSE); // 선택된 옵션 반전 해제
        }
        wrefresh(temp_win);

        int ch = wgetch(temp_win);
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10:
                delwin(temp_win);
                clear();
                refresh();
                return choice;
            case 27:
                delwin(temp_win);
                clear();
                refresh();
                return -1;
        }
    }
}

// 사이즈를 선택하는 함수
int select_size() {
    const char *options[] = {"Small", "Medium", "Large"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        int width = 40;
        int height = 8;
        int startx = (COLS - width) / 2;
        int starty = (LINES - height) / 2;

        WINDOW *size_win = newwin(height, width, starty, startx);
        wborder(size_win, '|', '|', '-', '-', '+', '+', '+', '+');
        keypad(size_win, TRUE);
        mvwprintw(size_win, 1, (width - strlen("Choose size(ESC to cancel)")) / 2, "Choose size(ESC to cancel)");

        for (int i = 0; i < num_options; i++) {
            int option_x = (width - strlen(options[i])) / 2;
            if (i == choice) {
                wattron(size_win, A_REVERSE); // 선택된 옵션 반전
            }
            mvwprintw(size_win, 3 + i, option_x, "%s", options[i]);
            wattroff(size_win, A_REVERSE); // 선택된 옵션 반전 해제
        }
        wrefresh(size_win);

        int ch = wgetch(size_win);
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10:
                delwin(size_win);
                clear();
                refresh();
                return choice;
            case 27:
                delwin(size_win);
                clear();
                refresh();
                return -1;
        }
    }
}

// 커피 가격을 계산하는 함수
int coffee(int qty, char size[], int index) {
    return (strcmp(size, "L") == 0) ? menu[index].price_l * qty : (strcmp(size, "M") == 0) ? menu[index].price_m * qty : menu[index].price_s * qty;
}

// 티 가격을 계산하는 함수
int tea(int qty, char size[], int index) {
    return (strcmp(size, "L") == 0) ? menu[index].price_l * qty : (strcmp(size, "M") == 0) ? menu[index].price_m * qty : menu[index].price_s * qty;
}

// 주문을 확인하는 함수
int confirm_order(product pd[], int count) {
    const char *options[] = {"Yes", "No"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        int width = 50;
        int height = count + 10; // 창 높이를 증가시켜 Yes/No 옵션이 창 안에 들어가도록 함
        int startx = (COLS - width) / 2;
        int starty = (LINES - height) / 2;

        WINDOW *confirm_win = newwin(height, width, starty, startx);
        wborder(confirm_win, '|', '|', '-', '-', '+', '+', '+', '+');
        keypad(confirm_win, TRUE);

        const char *prompt = "Is this the right order?";
        int prompt_len = strlen(prompt);
        if (prompt_len > width - 4) {
            prompt_len = width - 4;
        }
        int prompt_x = (width - prompt_len) / 2;

        mvwprintw(confirm_win, 1, prompt_x, "%.*s", prompt_len, prompt);
        mvwprintw(confirm_win, 2, (width - strlen("===============Order List===============")) / 2, "===============Order List===============");
        mvwprintw(confirm_win, 3, (width - strlen("Menu     Quantity Temperature Size Price")) / 2, "Menu     Quantity Temperature Size Price");
        mvwprintw(confirm_win, 4, (width - strlen("========================================")) / 2, "========================================");
        for (int i = 0; i < count; i++) {
            mvwprintw(confirm_win, 5 + i, 2, "%-10s %5d %11s %6s %5d", pd[i].name, pd[i].qty, pd[i].temperature, pd[i].size, pd[i].price);
        }

        for (int i = 0; i < num_options; i++) {
            int option_x = (width - strlen(options[i])) / 2;
            if (i == choice) {
                wattron(confirm_win, A_REVERSE);
            }
            mvwprintw(confirm_win, 7 + count + i, option_x, "%s", options[i]);
            wattroff(confirm_win, A_REVERSE);
        }
        wrefresh(confirm_win);

        int ch = wgetch(confirm_win);
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10:
                delwin(confirm_win);
                return (choice == 0) ? 'Y' : 'N';
        }
    }
}

// 쿠폰을 적용하는 함수
int apply_coupon(product pd[], int count, int discount[]) {
    int total_discount = 0;

    int use_coupon = select_yes_no("Do you want to use the coupon?");
    while (use_coupon) {
        int code;
        clear();
        refresh();

        int width = 50;
        int height = 5;
        int startx = (COLS - width) / 2;
        int starty = (LINES - height) / 2;

        WINDOW *coupon_win = newwin(height, width, starty, startx);
        wborder(coupon_win, '|', '|', '-', '-', '+', '+', '+', '+');
        mvwprintw(coupon_win, 1, (width - strlen("Please enter your coupon number:")) / 2, "Please enter your coupon number:");
        wrefresh(coupon_win);

        echo();
        wscanw(coupon_win, "%d", &code);
        noecho();

        delwin(coupon_win);

        if (code == 12345) {
            for (int i = 0; i < count; i++) {
                discount[i] = 0.5 * pd[i].price;
                total_discount += discount[i];
            }
            clear();
            mvprintw((LINES / 2) - 2, (COLS - strlen("The coupon has been applied.")) / 2, "The coupon has been applied.");
            refresh();
            getch();
            return total_discount;
        } else {
            clear();
            mvprintw((LINES / 2) - 2, (COLS - strlen("Invalid coupon number.")) / 2, "Invalid coupon number.");
            refresh();
            getch();
            use_coupon = select_yes_no("Do you want to use the coupon?");
            if (!use_coupon) {
                break;
            }
        }
    }

    return total_discount;
}

// 카드 결제 여부를 확인하는 함수
int card() {
    clear();
    refresh();
    return select_yes_no("Would you like to pay by credit card?");
}

// 영수증을 출력하는 함수
int print_receipt(product pd[], int count, int discount) {
    int total_price = 0;

    printw("\n----- Receipt -----\n");
    for (int i = 0; i < count; i++) {
        printw("Menu: %s\n", pd[i].name);
        printw("Hot or Ice: %s\n", pd[i].temperature);
        printw("Size: %s\n", pd[i].size);
        printw("Quantity: %d\n", pd[i].qty);
        printw("Price: %dwon\n", pd[i].price);
        printw("------------------\n");
        total_price += pd[i].price;
    }

    printw("Total Price: %dwon\n", total_price);
    printw("Discount amount: %dwon\n", discount);
    printw("Total amount of Payment: %dwon\n", total_price - discount);
    printw("------------------\n");
    return 0;
}

// 주문 내역을 CSV 파일에 저장하는 함수
int write_csv(char *filename, product pd[], int count, int discount[]) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(filename, "order_%d_%02d.csv", tm.tm_year + 1900, tm.tm_mon + 1);

    FILE *file;

    if ((file = fopen(filename, "a")) == NULL) {
        perror("Unable to open file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Name,Qty,Temperature,Size,Total price,Discount, Payment price, month,day,time\n");
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%s,%d,%s,%s,%d,%d,%d,%02d,%02d,%02d:%02d:%02d\n", pd[i].name, pd[i].qty, pd[i].temperature, pd[i].size, pd[i].price, discount[i], pd[i].price - discount[i], tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    fclose(file);
    return 0;
}

// Yes/No 질문을 선택하는 함수
int select_yes_no(const char *prompt) {
    const char *options[] = {"Yes", "No"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        clear();
        int width = 50;
        int height = 8;
        int startx = (COLS - width) / 2;
        int starty = (LINES - height) / 2;

        WINDOW *yes_no_win = newwin(height, width, starty, startx);
        wborder(yes_no_win, '|', '|', '-', '-', '+', '+', '+', '+');
        keypad(yes_no_win, TRUE);

        int prompt_len = strlen(prompt);
        if (prompt_len > width - 4) {
            prompt_len = width - 4;
        }
        int prompt_x = (width - prompt_len) / 2;

        mvwprintw(yes_no_win, 1, prompt_x, "%.*s", prompt_len, prompt);
        wrefresh(yes_no_win);

        for (int i = 0; i < num_options; i++) {
            int option_x = (width - strlen(options[i])) / 2;
            if (i == choice) {
                wattron(yes_no_win, A_REVERSE); // 선택된 옵션 반전
            }
            mvwprintw(yes_no_win, 3 + i, option_x, "%s", options[i]);
            wattroff(yes_no_win, A_REVERSE); // 선택된 옵션 반전 해제
        }
        wrefresh(yes_no_win);

        int ch = wgetch(yes_no_win);
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10:
                delwin(yes_no_win);
                return (choice == 0);
        }
    }
}

// 관리자 모드 함수
void admin_mode() {
    Order orders[MAX_ORDERS];
    int order_count = 0;
    int choice;
    int month;
    char cont = 'Y';

    load_menu("menu.csv"); // 메뉴 로드

    while (1) {
        clear();
        int startx = (COLS - 30) / 2;
        int starty = (LINES - 10) / 2;

        mvprintw(starty, startx, "Enter the month (1-12): ");
        refresh();

        char month_str[10];
        echo();  // 입력값을 화면에 표시
        getstr(month_str);
        noecho();  // 다시 입력값 화면 출력 비활성화

        if (strlen(month_str) == 0 || !isdigit(month_str[0]) || (month = atoi(month_str)) < 1 || month > 12) {
            mvprintw(starty + 2, startx, "Invalid input. Please enter a valid month (1-12).");
            refresh();
            getch();
        } else {
            break;
        }
    }

    get_monthly_data(month, orders, &order_count); // 월별 데이터 가져오기

    while (cont == 'Y' || cont == 'y') {
        choice = admin_menu(stdscr); // 관리자 메뉴 표시

        switch (choice) {
            case 1:
                clear();
                monthly_sales(orders, order_count, month); // 함수 호출 수정
                break;
            case 2: {
                char repeat = 'Y';
                while (repeat == 'Y' || repeat == 'y') {
                    clear();
                    display_menu_items(stdscr); // 메뉴 항목 표시
                    int menu_choice = admin_menu_selection(); // 메뉴 항목 선택
                    if (menu_choice >= 0 && menu_choice < coffee_item_count + tea_item_count) {
                        sales_by_product(orders, order_count, menu[menu_choice].name); // 함수 호출 수정
                    } else {
                        mvprintw((LINES - 10) / 2 + 2, (COLS - 30) / 2, "Invalid input.");
                        refresh();
                    }
                    repeat = select_yes_no("Would you like to check another product's sales? (Y/N): ") ? 'Y' : 'N';
                }
                break;
            }
            case 3:
                clear();
                ordered_selling_products(orders, order_count); // 함수 호출 수정
                break;
            case 4:
                clear();
                ordered_revenue_products(orders, order_count); // 함수 호출 수정
                break;
            case 5:
                clear();
                display_orders(orders, order_count); // 모든 주문 표시
                break;
            default:
                clear();
                mvprintw((LINES - 10) / 2, (COLS - 30) / 2, "Invalid input.");
                refresh();
        }

        cont = select_yes_no("Return to the menu? (Y/N): ") ? 'Y' : 'N';
    }

    mvprintw((LINES - 10) / 2, (COLS - 30) / 2, "Exiting the program.");
    getch();
    endwin();
}

// CSV 파일을 읽어오는 함수
int read_csv(char* filename, Order orders[], int max_orders) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Unable to open file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    int count = 0;

    if (fgets(line, MAX_LINE_LENGTH, file)) {
        while (fgets(line, MAX_LINE_LENGTH, file) && count < 21) {
            char* token = strtok(line, ",");
            if (token != NULL) strcpy(orders[count].name, token);

            token = strtok(NULL, ",");
            if (token != NULL) orders[count].qty = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL) strcpy(orders[count].temperature, token);

            token = strtok(NULL, ",");
            if (token != NULL) strcpy(orders[count].size, token);

            token = strtok(NULL, ",");
            if (token != NULL) orders[count].total_price = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL) orders[count].discount = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL) orders[count].payment = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL) orders[count].month = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL) orders[count].day = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL) strcpy(orders[count].time, token);

            count++;
        }
    }

    fclose(file);
    return count;
}

// 월별 데이터 가져오는 함수
void get_monthly_data(int month, Order orders[], int *order_count) {
    char filename[20];
    sprintf(filename, "order_2024_%02d.csv", month);
    *order_count = read_csv(filename, orders, MAX_ORDERS);
}

// 월별 총 매출을 계산하고 화면에 표시하는 함수
void monthly_sales(Order orders[], int order_count, int month) {
    int total_sales = 0;

    for (int i = 0; i < order_count; i++) {
        total_sales += orders[i].payment;
    }

    clear();
    int width = 50;
    int height = 10;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *sales_win = newwin(height, width, starty, startx);
    wborder(sales_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(sales_win, 1, (width - strlen("Monthly Sales")) / 2, "Monthly Sales");
    mvwprintw(sales_win, 3, 2, "Total sales for month %d: %d won", month, total_sales);
    mvwprintw(sales_win, 5, 2, "Press any key to return to the menu...");
    wrefresh(sales_win);
    wgetch(sales_win);
    delwin(sales_win);
    clear();
    refresh();
}

// 제품별 판매량을 계산하고 화면에 표시하는 함수
void sales_by_product(Order orders[], int order_count, char* product_name) {
    int small = 0, medium = 0, large = 0;

    for (int i = 0; i < order_count; i++) {
        if (strcmp(orders[i].name, product_name) == 0) {
            if (strcmp(orders[i].size, "S") == 0) {
                small += orders[i].qty;
            } else if (strcmp(orders[i].size, "M") == 0) {
                medium += orders[i].qty;
            } else if (strcmp(orders[i].size, "L") == 0) {
                large += orders[i].qty;
            }
        }
    }

    clear();
    int width = 50;
    int height = 10;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *sales_win = newwin(height, width, starty, startx);
    wborder(sales_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(sales_win, 1, (width - strlen(product_name)) / 2, product_name);
    mvwprintw(sales_win, 3, 2, "Small: %d, Medium: %d, Large: %d", small, medium, large);
    mvwprintw(sales_win, 5, 2, "Press any key to continue...");
    wrefresh(sales_win);
    wgetch(sales_win);
    delwin(sales_win);
    clear();
    refresh();
}

// 가장 많이 판매된 제품을 계산하고 화면에 표시하는 함수
void ordered_selling_products(Order orders[], int order_count) {
    typedef struct {
        char name[20];
        int total_qty;
    } ProductSales;

    ProductSales sales[MAX_ORDERS] = { 0 };
    int sales_count = 0;

    for (int i = 0; i < order_count; i++) {
        int found = 0;
        for (int j = 0; j < sales_count; j++) {
            if (strcmp(sales[j].name, orders[i].name) == 0) {
                sales[j].total_qty += orders[i].qty;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(sales[sales_count].name, orders[i].name);
            sales[sales_count].total_qty = orders[i].qty;
            sales_count++;
        }
    }

    // 판매량 기준으로 정렬
    for (int i = 0; i < sales_count - 1; i++) {
        for (int j = i + 1; j < sales_count; j++) {
            if (sales[i].total_qty < sales[j].total_qty) {
                ProductSales temp = sales[i];
                sales[i] = sales[j];
                sales[j] = temp;
            }
        }
    }

    clear();
    int width = 50;
    int height = sales_count + 6;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *sales_win = newwin(height, width, starty, startx);
    wborder(sales_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(sales_win, 1, (width - strlen("Best Selling Products")) / 2, "Best Selling Products");

    for (int i = 0; i < sales_count; i++) {
        mvwprintw(sales_win, 3 + i, 2, "%s: %d", sales[i].name, sales[i].total_qty);
    }

    mvwprintw(sales_win, height - 2, 2, "Press any key to return to the menu...");
    wrefresh(sales_win);
    wgetch(sales_win);
    delwin(sales_win);
    clear();
    refresh();
}

// 가장 높은 수익을 올린 제품을 계산하고 화면에 표시하는 함수
void ordered_revenue_products(Order orders[], int order_count) {
    typedef struct {
        char name[20];
        int total_revenue;
    } ProductRevenue;

    ProductRevenue revenue[MAX_ORDERS] = { 0 };
    int revenue_count = 0;

    for (int i = 0; i < order_count; i++) {
        int found = 0;
        for (int j = 0; j < revenue_count; j++) {
            if (strcmp(revenue[j].name, orders[i].name) == 0) {
                revenue[j].total_revenue += orders[i].payment;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(revenue[revenue_count].name, orders[i].name);
            revenue[revenue_count].total_revenue = orders[i].payment;
            revenue_count++;
        }
    }

    // 수익 기준으로 정렬
    for (int i = 0; i < revenue_count - 1; i++) {
        for (int j = i + 1; j < revenue_count; j++) {
            if (revenue[i].total_revenue < revenue[j].total_revenue) {
                ProductRevenue temp = revenue[i];
                revenue[i] = revenue[j];
                revenue[j] = temp;
            }
        }
    }

    clear();
    int width = 50;
    int height = revenue_count + 6;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *revenue_win = newwin(height, width, starty, startx);
    wborder(revenue_win, '|', '|', '-', '-', '+', '+', '+', '+');
    mvwprintw(revenue_win, 1, (width - strlen("Highest Revenue Products")) / 2, "Highest Revenue Products");

    for (int i = 0; i < revenue_count; i++) {
        mvwprintw(revenue_win, 3 + i, 2, "%s: %d", revenue[i].name, revenue[i].total_revenue);
    }

    mvwprintw(revenue_win, height - 2, 2, "Press any key to return to the menu...");
    wrefresh(revenue_win);
    wgetch(revenue_win);
    delwin(revenue_win);
    clear();
    refresh();
}

// 모든 주문을 표시하는 함수
int display_orders(Order orders[], int order_count) {
    clear();
    int width = COLS; // 창 너비를 터미널 너비로 설정
    int height = LINES; // 창 높이를 터미널 높이로 설정
    int startx = 0;
    int starty = 0;

    WINDOW *orders_win = newwin(height, width, starty, startx);

    box(orders_win, 0, 0); // 테두리 설정
    mvwprintw(orders_win, 1, (width - strlen("All Orders")) / 2, "All Orders");

    // 각 열의 너비 설정
    int name_width = 12;
    int qty_width = 6;
    int temp_width = 6;
    int size_width = 6;
    int total_price_width = 12;
    int discount_width = 10;
    int payment_width = 10;
    int month_width = 8;
    int day_width = 6;
    int time_width = 12;

    mvwprintw(orders_win, 3, 1, "%-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s %-*s",
              name_width, "Name",
              qty_width, "Qty",
              temp_width, "Temp",
              size_width, "Size",
              total_price_width, "TotalPrice",
              discount_width, "Discount",
              payment_width, "Payment",
              month_width, "Month",
              day_width, "Day",
              time_width, "Time");

    for (int i = 0; i < order_count && i < height - 6; i++) { // 높이를 넘어가지 않도록 조건 추가
        mvwprintw(orders_win, 4 + i, 1, "%-*s %-*d %-*s %-*s %-*d %-*d %-*d %-*d %-*d %-*s",
                  name_width, orders[i].name,
                  qty_width, orders[i].qty,
                  temp_width, orders[i].temperature,
                  size_width, orders[i].size,
                  total_price_width, orders[i].total_price,
                  discount_width, orders[i].discount,
                  payment_width, orders[i].payment,
                  month_width, orders[i].month,
                  day_width, orders[i].day,
                  time_width, orders[i].time);
    }

    mvwprintw(orders_win, height - 2, 1, "Press any key to return to the menu...");
    wrefresh(orders_win);
    wgetch(orders_win);
    delwin(orders_win);
    clear();
    refresh();
    return 0;
}

// 메뉴 항목을 표시하는 함수
void display_menu_items(WINDOW *win) {
    int startx = (COLS - 30) / 2;
    int starty = (LINES - 20) / 2;

    wprintw(win, "\n==== Menu List ====\n");
    int visible_index = 0; // 유효한 메뉴 항목의 인덱스

    for (int i = 0; i < MAX_MENU_ITEMS; i++) {
        if (strlen(menu[i].name) > 0) {
            mvwprintw(win, starty + visible_index, startx, "%d. %s\n", visible_index + 1, menu[i].name);
            visible_index++; // 유효한 항목을 출력할 때마다 인덱스 증가
        }
    }
    wrefresh(win);
}

// 관리자 메뉴를 표시하는 함수
int admin_menu(WINDOW *win) {
    const char *options[] = {"Monthly Sales", "Sales by Product", "Best Selling Product", "Highest Revenue Product", "View All Orders"};
    int choice = 0;
    int num_options = sizeof(options) / sizeof(options[0]);

    int width = 50;
    int height = num_options + 6;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;

    WINDOW *admin_win = newwin(height, width, starty, startx);
    wborder(admin_win, '|', '|', '-', '-', '+', '+', '+', '+');
    keypad(admin_win, TRUE);

    while (1) {
        werase(admin_win);
        wborder(admin_win, '|', '|', '-', '-', '+', '+', '+', '+');
        mvwprintw(admin_win, 1, (width - strlen("Administrator Menu")) / 2, "Administrator Menu");

        for (int i = 0; i < num_options; i++) {
            if (i == choice) {
                wattron(admin_win, A_REVERSE);
            }
            mvwprintw(admin_win, 3 + i, 2, "%d. %s", i + 1, options[i]);
            wattroff(admin_win, A_REVERSE);
        }

        wrefresh(admin_win);
        int ch = wgetch(admin_win);
        switch (ch) {
            case KEY_UP:
                if (choice > 0) choice--;
                break;
            case KEY_DOWN:
                if (choice < num_options - 1) choice++;
                break;
            case 10:
                delwin(admin_win);
                return choice + 1;
        }
    }
}

// 관리자 메뉴 항목 선택 함수
int admin_menu_selection() {
    int choice = 0;

    while (1) {
        clear();
        int width = 50;
        int startx = (COLS - width) / 2;
        int starty = (LINES - (coffee_item_count + tea_item_count)) / 2;
        mvprintw(starty - 2, startx, "----------Menu----------");
        int visible_item_count = 0;
        int valid_choices[coffee_item_count + tea_item_count];
        memset(valid_choices, -1, sizeof(valid_choices));
        for (int i = 0; i < coffee_item_count + tea_item_count; i++) {
            if (strlen(menu[i].name) > 0) {
                if (visible_item_count == choice) {
                    attron(A_REVERSE); // 선택된 항목 반전
                    valid_choices[visible_item_count] = i;
                }
                mvprintw(starty + visible_item_count, startx, "%d) %-15s S: %4dwon M: %4dwon L: %4dwon", visible_item_count + 1, menu[i].name, menu[i].price_s, menu[i].price_m, menu[i].price_l);
                if (visible_item_count == choice) {
                    attroff(A_REVERSE); // 선택된 항목 반전 해제
                }
                visible_item_count++;
            }
        }

        refresh();
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (choice > 0) {
                    choice--;
                } else {
                    choice = visible_item_count - 1;
                }
                break;
            case KEY_DOWN:
                if (choice < visible_item_count - 1) {
                    choice++;
                } else {
                    choice = 0;
                }
                break;
            case 10:
                clear();
                refresh();
                return valid_choices[choice];
        }
    }
}
