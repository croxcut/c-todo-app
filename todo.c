/**
 * todo.c - Fancy Terminal Todo App with CSV Export :D*
 *
 * todo app? made simply? WTH nooo :D*
 * write it on C to make you question your existence XD*
 * it's a one off project might not update this useless project 
 * 
 * Build: make  |  Run: todo.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

// keys :D*
#define KEY_UP      72
#define KEY_DOWN    80
#define KEY_LEFT    75
#define KEY_RIGHT   77
#define KEY_ENTER   13
#define KEY_ESC     27
#define KEY_BACK    8
#define KEY_DELETE  83
#define KEY_F1      59
#define KEY_F2      60
#define KEY_F5      63

// colours :D*
#define C_BLACK       0
#define C_BLUE        1
#define C_GREEN       2
#define C_CYAN        3
#define C_RED         4
#define C_MAGENTA     5
#define C_YELLOW      6
#define C_WHITE       7
#define C_BBLACK      8
#define C_BBLUE       9
#define C_BGREEN     10
#define C_BCYAN      11
#define C_BRED       12
#define C_BMAGENTA   13
#define C_BYELLOW    14
#define C_BWHITE     15

// limits :D*
#define MAX_TODOS     128
#define MAX_TITLE      80
#define MAX_TAG        20
#define MAX_DATE       12
#define SAVE_FILE  "todos.dat"
#define CSV_FILE   "todos.csv"

// priority labels :D*
#define PRI_LOW    0
#define PRI_MED    1
#define PRI_HIGH   2

typedef struct {
    char  title[MAX_TITLE];
    char  tag[MAX_TAG];
    char  date[MAX_DATE];   // YYYY-MM-DD :D*
    int   priority;
    int   done;
} Todo;

// globals :D*
static Todo  todos[MAX_TODOS];
static int   todo_count  = 0;
static int   selected    = 0;
static int   scroll_off  = 0;
static int   view_mode   = 0;  // 0=all 1=active 2=done :D*

static HANDLE hOut;
static int    con_w = 80, con_h = 30;

/* ══════════════════════════════════════════════════════
 *  CONSOLE HELPERS
 * ══════════════════════════════════════════════════════ */

static void get_console_size(void) {
    CONSOLE_SCREEN_BUFFER_INFO ci;
    if (GetConsoleScreenBufferInfo(hOut, &ci)) {
        con_w = ci.srWindow.Right  - ci.srWindow.Left + 1;
        con_h = ci.srWindow.Bottom - ci.srWindow.Top  + 1;
    }
}

static void gotoxy(int x, int y) {
    COORD p = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hOut, p);
}

static void color(int fg, int bg) {
    SetConsoleTextAttribute(hOut, (WORD)((bg << 4) | fg));
}

static void hide_cursor(void) {
    CONSOLE_CURSOR_INFO ci = {1, FALSE};
    SetConsoleCursorInfo(hOut, &ci);
}

static void show_cursor(void) {
    CONSOLE_CURSOR_INFO ci = {1, TRUE};
    SetConsoleCursorInfo(hOut, &ci);
}

static void clrscr(void) { system("cls"); }

// print a string padded/clipped to width w :D*
static void putfw(const char *s, int w) {
    int len = (int)strlen(s);
    for (int i = 0; i < w; i++) {
        putchar(i < len ? s[i] : ' ');
    }
}

/* ══════════════════════════════════════════════════════
 *  BOX-DRAWING (ASCII art windows)
 * ══════════════════════════════════════════════════════ */

// double-line box :D*
static void draw_dbox(int x, int y, int w, int h, int fg, int bg) {
    color(fg, bg);
    // top :D*
    gotoxy(x, y);
    putchar('\xC9');
    for (int i = 1; i < w-1; i++) putchar('\xCD');
    putchar('\xBB');
    // sides :D*
    for (int r = 1; r < h-1; r++) {
        gotoxy(x,     y+r); putchar('\xBA');
        gotoxy(x+w-1, y+r); putchar('\xBA');
    }
    // bottom :D*
    gotoxy(x, y+h-1);
    putchar('\xC8');
    for (int i = 1; i < w-1; i++) putchar('\xCD');
    putchar('\xBC');
}

// single-line box :D*
static void draw_sbox(int x, int y, int w, int h, int fg, int bg) {
    color(fg, bg);
    gotoxy(x, y);
    putchar('\xDA');
    for (int i = 1; i < w-1; i++) putchar('\xC4');
    putchar('\xBF');
    for (int r = 1; r < h-1; r++) {
        gotoxy(x,     y+r); putchar('\xB3');
        gotoxy(x+w-1, y+r); putchar('\xB3');
    }
    gotoxy(x, y+h-1);
    putchar('\xC0');
    for (int i = 1; i < w-1; i++) putchar('\xC4');
    putchar('\xD9');
}

// fill interior of a box with spaces :D*
static void fill_box(int x, int y, int w, int h, int fg, int bg) {
    color(fg, bg);
    for (int r = 1; r < h-1; r++) {
        gotoxy(x+1, y+r);
        for (int c = 0; c < w-2; c++) putchar(' ');
    }
}

// title bar inside a box :D*
static void box_title(int x, int y, int w, const char *title, int fg, int bg) {
    int tlen = (int)strlen(title);
    int tx   = x + (w - tlen) / 2;
    gotoxy(tx, y);
    color(fg, bg);
    printf(" %s ", title);
}

// horizontal separator, double-line style, connects to double box :D*
static void hsep(int x, int y, int w, int fg, int bg) {
    color(fg, bg);
    gotoxy(x, y);
    putchar('\xCC');
    for (int i = 1; i < w-1; i++) putchar('\xCD');
    putchar('\xB9');
}

/* ══════════════════════════════════════════════════════
 *  DATA HELPERS
 * ══════════════════════════════════════════════════════ */

static const char *pri_label(int p) {
    switch (p) {
        case PRI_LOW:  return "LOW ";
        case PRI_MED:  return "MED ";
        case PRI_HIGH: return "HIGH";
    }
    return "??? ";
}

static int pri_fg(int p) {
    switch (p) {
        case PRI_LOW:  return C_BGREEN;
        case PRI_MED:  return C_BYELLOW;
        case PRI_HIGH: return C_BRED;
    }
    return C_WHITE;
}

static void today_str(char *buf) {
    time_t t  = time(NULL);
    struct tm *tm = localtime(&t);
    sprintf(buf, "%04d-%02d-%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
}

/* ══════════════════════════════════════════════════════
 *  PERSISTENCE
 * ══════════════════════════════════════════════════════ */

static void save_todos(void) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) return;
    fwrite(&todo_count, sizeof(int), 1, f);
    fwrite(todos, sizeof(Todo), todo_count, f);
    fclose(f);
}

static void load_todos(void) {
    FILE *f = fopen(SAVE_FILE, "rb");
    if (!f) return;
    fread(&todo_count, sizeof(int), 1, f);
    if (todo_count > MAX_TODOS) todo_count = MAX_TODOS;
    fread(todos, sizeof(Todo), todo_count, f);
    fclose(f);
}

static void export_csv(void) {
    FILE *f = fopen(CSV_FILE, "w");
    if (!f) return;
    fprintf(f, "Title,Tag,Date,Priority,Status\n");
    for (int i = 0; i < todo_count; i++) {
        fprintf(f, "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
            todos[i].title,
            todos[i].tag,
            todos[i].date,
            pri_label(todos[i].priority),
            todos[i].done ? "Done" : "Active");
    }
    fclose(f);
}

/* ══════════════════════════════════════════════════════
 *  TEXT INPUT WIDGET  (inline, returns 1 on OK, 0 on ESC)
 * ══════════════════════════════════════════════════════ */

static int textinput(int x, int y, int w, char *buf, int maxlen, const char *prompt, int pfg, int pbg) {
    int pos = (int)strlen(buf);

    while (1) {
        // redraw field :D*
        gotoxy(x, y);
        color(pfg, pbg);
        printf("%-*.*s", w, w, buf);
        gotoxy(x + pos, y);
        show_cursor();

        int ch = _getch();
        if (ch == KEY_ENTER) { hide_cursor(); return 1; }
        if (ch == KEY_ESC)   { hide_cursor(); return 0; }
        if (ch == KEY_BACK) {
            if (pos > 0) { buf[--pos] = '\0'; }
        } else if (ch == 0 || ch == 224) {
            ch = _getch(); // swallow arrow :D*
        } else if (ch >= 32 && ch <= 126) {
            if (pos < maxlen-1 && pos < w-1) {
                buf[pos++] = (char)ch;
                buf[pos]   = '\0';
            }
        }
    }
}

/* ══════════════════════════════════════════════════════
 *  MODAL DIALOG  (centred, with message + OK/Cancel)
 * ══════════════════════════════════════════════════════ */

static int modal(const char *title, const char *msg) {
    int w  = 44, h = 7;
    int x  = (con_w - w) / 2;
    int y  = (con_h - h) / 2;
    int sel = 1; // default OK :D*

    while (1) {
        fill_box(x, y, w, h, C_WHITE, C_BLACK);
        draw_dbox(x, y, w, h, C_BCYAN, C_BLACK);
        box_title(x, y, w, title, C_BLACK, C_BCYAN);

        gotoxy(x + (w - (int)strlen(msg)) / 2, y + 2);
        color(C_BWHITE, C_BLACK);
        printf("%s", msg);

        // buttons :D*
        int bx = x + w/2 - 12;
        gotoxy(bx, y+4);
        color(sel == 0 ? C_BLACK : C_BWHITE, sel == 0 ? C_BWHITE : C_BLACK);
        printf("  [ Cancel ]  ");
        color(sel == 1 ? C_BLACK : C_BWHITE, sel == 1 ? C_BWHITE : C_BLACK);
        printf("  [  OK  ]  ");

        int ch = _getch();
        if (ch == 0 || ch == 224) {
            ch = _getch();
            if (ch == KEY_LEFT || ch == KEY_RIGHT) sel = !sel;
        } else if (ch == KEY_ENTER) {
            return sel;
        } else if (ch == KEY_ESC) {
            return 0;
        }
    }
}

/* ══════════════════════════════════════════════════════
 *  ADD / EDIT FORM
 * ══════════════════════════════════════════════════════ */

static int form_edit(Todo *t, int is_new) {
    int w = 60, h = 20;
    int x = (con_w - w) / 2;
    int y = (con_h - h) / 2;

    fill_box(x, y, w, h, C_WHITE, C_BLACK);
    draw_dbox(x, y, w, h, C_BYELLOW, C_BLACK);
    box_title(x, y, w, is_new ? "  NEW TASK  " : "  EDIT TASK  ", C_BLACK, C_BYELLOW);

    // field labels :D*
    const char *labels[] = {"Title   :", "Tag     :", "Date    :", "Priority:"};
    int field_y[] = {y+3, y+6, y+9, y+12};

    for (int i = 0; i < 4; i++) {
        gotoxy(x+3, field_y[i]);
        color(C_BCYAN, C_BLACK);
        printf("%s", labels[i]);
    }

    // decorative separator :D*
    hsep(x, y+14, w, C_BYELLOW, C_BLACK);

    gotoxy(x+3, y+15);
    color(C_BBLACK, C_BLACK);
    printf("ESC=cancel  ENTER=confirm  TAB=cycle priority");

    // field buffers :D*
    char tbuf[MAX_TITLE], gbuf[MAX_TAG], dbuf[MAX_DATE];
    strcpy(tbuf, t->title);
    strcpy(gbuf, t->tag);
    strcpy(dbuf, t->date);
    int pri = t->priority;

    // draw priority badge :D*
    #define DRAW_PRI() do { \
        gotoxy(x+14, field_y[3]); \
        color(C_BLACK, pri_fg(pri)); \
        printf(" %-4s ", pri_label(pri)); \
        color(C_BBLACK, C_BLACK); \
        printf("  (TAB to cycle)"); \
    } while(0)

    DRAW_PRI();

    int field = 0; // currently active field :D*
    #define FW (w-16)

    // render one text field :D*
    #define DRAW_FIELD(buf, fy) do { \
        gotoxy(x+14, (fy)); \
        color(C_BWHITE, C_BBLACK); \
        printf("%-*.*s", FW, FW, (buf)); \
    } while(0)

    DRAW_FIELD(tbuf, field_y[0]);
    DRAW_FIELD(gbuf, field_y[1]);
    DRAW_FIELD(dbuf, field_y[2]);

    // navigate fields with TAB / arrows, edit with chars :D*
    while (1) {
        // highlight active field indicator :D*
        for (int i = 0; i < 3; i++) {
            gotoxy(x+13, field_y[i]);
            color(i == field ? C_BYELLOW : C_BBLACK, C_BLACK);
            putchar(i == field ? '\x10' : ' '); // arrow or blank :D*
        }

        char *cur_buf = (field == 0) ? tbuf : (field == 1) ? gbuf : dbuf;
        int   cur_max = (field == 0) ? MAX_TITLE : (field == 1) ? MAX_TAG : MAX_DATE;

        if (field == 3) {
            // priority field, only TAB/ENTER/ESC :D*
            gotoxy(x+14, field_y[3]);
            show_cursor();
            int ch = _getch();
            hide_cursor();
            if (ch == '\t' || ch == ' ') {
                pri = (pri + 1) % 3;
                DRAW_PRI();
            } else if (ch == KEY_UP)    { field = 2; }
            else if (ch == KEY_DOWN)    { field = 0; }
            else if (ch == KEY_ENTER)   { break; }
            else if (ch == KEY_ESC)     { return 0; }
            else if (ch == 0 || ch == 224) {
                ch = _getch();
                if (ch == KEY_UP)   field = 2;
                if (ch == KEY_DOWN) field = 0;
            }
            continue;
        }

        // text field input :D*
        int pos = (int)strlen(cur_buf);
        show_cursor();
        gotoxy(x+14+pos, field_y[field]);
        int ch = _getch();
        hide_cursor();

        if (ch == KEY_ESC) return 0;
        if (ch == KEY_ENTER || ch == '\t') {
            field = (field + 1) % 4;
        } else if (ch == KEY_BACK) {
            if (pos > 0) cur_buf[--pos] = '\0';
        } else if (ch == 0 || ch == 224) {
            ch = _getch();
            if (ch == KEY_UP)   field = (field + 3) % 4;
            if (ch == KEY_DOWN) field = (field + 1) % 4;
        } else if (ch >= 32 && ch <= 126) {
            if (pos < cur_max-1 && pos < FW-1) {
                cur_buf[pos++] = (char)ch;
                cur_buf[pos]   = '\0';
            }
        }
        DRAW_FIELD(tbuf, field_y[0]);
        DRAW_FIELD(gbuf, field_y[1]);
        DRAW_FIELD(dbuf, field_y[2]);
    }

    if (strlen(tbuf) == 0) return 0; // refuse empty title :D*

    strcpy(t->title,    tbuf);
    strcpy(t->tag,      gbuf);
    strcpy(t->date,     dbuf);
    t->priority = pri;
    return 1;
}

/* ══════════════════════════════════════════════════════
 *  HEADER
 * ══════════════════════════════════════════════════════ */

static void draw_header(void) {
    draw_dbox(0, 0, con_w, 4, C_BCYAN, C_BLACK);
    fill_box(0, 0, con_w, 4, C_WHITE, C_BLACK);

    // ASCII art logo (compact) :D*
    gotoxy(3, 1);
    color(C_BMAGENTA, C_BLACK);
    printf("\xDB\xDB \xDB\xDB\xDB\xDB\xDB ");
    color(C_BCYAN, C_BLACK);
    printf("TODO");
    color(C_BYELLOW, C_BLACK);
    printf(" \xB2\xB2\xB2 ");
    color(C_BWHITE, C_BLACK);
    printf("Fancy Terminal Task Manager");

    // right-side stats :D*
    int done = 0;
    for (int i = 0; i < todo_count; i++) if (todos[i].done) done++;
    gotoxy(con_w - 26, 1);
    color(C_BBLACK, C_BLACK);
    printf("[");
    color(C_BGREEN, C_BLACK);
    printf("%2d done", done);
    color(C_BBLACK, C_BLACK);
    printf("|");
    color(C_BRED, C_BLACK);
    printf("%2d left", todo_count - done);
    color(C_BBLACK, C_BLACK);
    printf("|");
    color(C_BWHITE, C_BLACK);
    printf("%2d total", todo_count);
    color(C_BBLACK, C_BLACK);
    printf("]");

    // view-mode tabs :D*
    const char *tabs[] = {" ALL ", " ACTIVE ", " DONE "};
    int tx = 3;
    gotoxy(tx, 2);
    for (int i = 0; i < 3; i++) {
        if (i == view_mode) {
            color(C_BLACK, C_BYELLOW);
        } else {
            color(C_BBLACK, C_BLACK);
        }
        printf("%s", tabs[i]);
        color(C_BBLACK, C_BLACK);
        printf(" ");
    }
}

/* ══════════════════════════════════════════════════════
 *  STATUS BAR
 * ══════════════════════════════════════════════════════ */

static void draw_statusbar(const char *msg) {
    int y = con_h - 2;
    draw_sbox(0, y, con_w, 3, C_BBLACK, C_BLACK);
    fill_box(0, y, con_w, 3, C_WHITE, C_BLACK);

    gotoxy(2, y+1);
    color(C_BCYAN, C_BLACK); printf("[A]");
    color(C_BWHITE, C_BLACK); printf("dd  ");
    color(C_BCYAN, C_BLACK); printf("[E]");
    color(C_BWHITE, C_BLACK); printf("dit  ");
    color(C_BCYAN, C_BLACK); printf("[D]");
    color(C_BWHITE, C_BLACK); printf("el  ");
    color(C_BCYAN, C_BLACK); printf("[Space]");
    color(C_BWHITE, C_BLACK); printf("Toggle  ");
    color(C_BCYAN, C_BLACK); printf("[X]");
    color(C_BWHITE, C_BLACK); printf("CSV  ");
    color(C_BCYAN, C_BLACK); printf("[Tab]");
    color(C_BWHITE, C_BLACK); printf("View  ");
    color(C_BCYAN, C_BLACK); printf("[Q]");
    color(C_BWHITE, C_BLACK); printf("uit");

    if (msg && msg[0]) {
        int mlen = (int)strlen(msg);
        gotoxy(con_w - mlen - 3, y+1);
        color(C_BLACK, C_BYELLOW);
        printf(" %s ", msg);
    }
}

/* ══════════════════════════════════════════════════════
 *  TODO LIST PANEL
 * ══════════════════════════════════════════════════════ */

static int visible_count(void) {
    if (view_mode == 0) return todo_count;
    int c = 0;
    for (int i = 0; i < todo_count; i++) {
        if (view_mode == 1 && !todos[i].done) c++;
        if (view_mode == 2 &&  todos[i].done) c++;
    }
    return c;
}

// map visible index to real index :D*
static int vis_to_real(int vi) {
    if (view_mode == 0) return vi;
    int c = 0;
    for (int i = 0; i < todo_count; i++) {
        int show = (view_mode == 1 && !todos[i].done) || (view_mode == 2 && todos[i].done);
        if (show) {
            if (c == vi) return i;
            c++;
        }
    }
    return -1;
}

static void draw_list(void) {
    int lx = 1, ly = 4;
    int lw = con_w - 2;
    int list_h = con_h - 7; // rows available for items :D*

    // column header :D*
    gotoxy(lx, ly);
    color(C_BLACK, C_BCYAN);
    printf(" %-3s %-*.*s %-*.*s %-10s %-5s %-6s",
        "#",
        lw - 32, lw - 32, "TITLE",
        MAX_TAG-2, MAX_TAG-2, "TAG",
        "DATE",
        "PRI",
        "STATUS");
    // pad to width :D*
    color(C_BLACK, C_BCYAN);
    printf("%-*s", lw - (int)(lw - 32 + MAX_TAG - 2 + 10 + 5 + 6 + 10), "");

    int vc = visible_count();
    // clamp scroll :D*
    if (selected >= vc) selected = vc > 0 ? vc-1 : 0;
    if (selected < scroll_off) scroll_off = selected;
    if (selected >= scroll_off + list_h) scroll_off = selected - list_h + 1;

    for (int row = 0; row < list_h; row++) {
        int vi = scroll_off + row;
        gotoxy(lx, ly+1+row);

        if (vi >= vc) {
            color(C_BBLACK, C_BLACK);
            for (int c = 0; c < lw; c++) putchar(' ');
            continue;
        }

        int ri = vis_to_real(vi);
        Todo *t = &todos[ri];
        int is_sel = (vi == selected);

        // row background :D*
        int rbg = is_sel ? C_BLUE : (row % 2 == 0 ? C_BLACK : C_BBLACK);

        // done items get dimmed :D*
        int rfg = t->done ? C_BBLACK : C_BWHITE;
        if (is_sel) rfg = C_BWHITE;

        color(rfg, rbg);
        printf(" %-3d ", ri+1);

        // title, done = strikethrough approximated with dim :D*
        if (t->done) color(C_BBLACK, rbg); else color(is_sel ? C_BYELLOW : C_BWHITE, rbg);
        printf("%-*.*s ", lw-32, lw-32, t->title);

        color(C_BCYAN, rbg);
        printf("%-*.*s ", MAX_TAG-2, MAX_TAG-2, t->tag);

        color(C_BBLACK+1, rbg);
        printf("%-10s ", t->date[0] ? t->date : "-");

        // priority badge :D*
        if (is_sel) { color(C_BLACK, pri_fg(t->priority)); }
        else        { color(pri_fg(t->priority), rbg); }
        printf("%-4s ", pri_label(t->priority));

        // status :D*
        if (t->done) { color(C_BGREEN, rbg); printf("\x07 DONE  "); }
        else         { color(C_BRED,   rbg); printf("\xFB TODO  "); }

        color(rfg, rbg);
        // fill rest of row :D*
        int used = 4 + (lw-32) + 1 + (MAX_TAG-2) + 1 + 11 + 5 + 7;
        for (int c = used; c < lw; c++) putchar(' ');
    }

    // scroll indicator :D*
    if (vc > list_h) {
        int bar_h = list_h;
        int thumb = (selected * bar_h) / vc;
        for (int r = 0; r < bar_h; r++) {
            gotoxy(con_w-1, ly+1+r);
            color(r == thumb ? C_BWHITE : C_BBLACK, C_BLACK);
            putchar(r == thumb ? '\xDB' : '\xB0');
        }
    }
}

/* ══════════════════════════════════════════════════════
 *  DETAIL PANEL — small info box below list :D*
 * ══════════════════════════════════════════════════════ */
static void draw_detail(void) {
    int vc = visible_count();
    if (vc == 0) return;
    int ri = vis_to_real(selected);
    if (ri < 0) return;
    Todo *t = &todos[ri];

    int dy = con_h - 6;
    int dw = con_w - 2;
    draw_sbox(1, dy, dw, 4, C_BBLACK, C_BLACK);
    fill_box(1, dy, dw, 4, C_WHITE, C_BLACK);

    gotoxy(3, dy+1);
    color(C_BCYAN, C_BLACK); printf("Title : ");
    color(C_BWHITE, C_BLACK); printf("%-40.40s", t->title);
    color(C_BBLUE, C_BLACK); printf("  Tag : ");
    color(C_BWHITE, C_BLACK); printf("%-16.16s", t->tag);

    gotoxy(3, dy+2);
    color(C_BCYAN, C_BLACK); printf("Date  : ");
    color(C_BWHITE, C_BLACK); printf("%-10s", t->date[0] ? t->date : "none");
    color(C_BBLUE, C_BLACK); printf("  Pri : ");
    color(pri_fg(t->priority), C_BLACK); printf("%-4s", pri_label(t->priority));
    color(C_BBLUE, C_BLACK); printf("  Status: ");
    if (t->done) { color(C_BGREEN, C_BLACK); printf("DONE"); }
    else         { color(C_BRED,   C_BLACK); printf("TODO"); }
}

/* ══════════════════════════════════════════════════════
 *  FULL REDRAW
 * ══════════════════════════════════════════════════════ */
static void redraw(const char *msg) {
    get_console_size();
    draw_header();
    draw_list();
    draw_detail();
    draw_statusbar(msg);
    color(C_WHITE, C_BLACK);
}

/* ══════════════════════════════════════════════════════
 *  MAIN
 * ══════════════════════════════════════════════════════ */
int main(void) {
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // set console to a good size :D*
    system("mode con: cols=100 lines=35");
    SetConsoleTitle("TODO \xFB Terminal");
    clrscr();
    hide_cursor();
    load_todos();

    char status[64] = "";
    int running = 1;

    while (running) {
        redraw(status);
        status[0] = '\0';

        int ch = _getch();

        if (ch == 0 || ch == 224) {
            ch = _getch();
            int vc = visible_count();
            if (ch == KEY_UP)   { if (selected > 0) selected--; }
            if (ch == KEY_DOWN) { if (selected < vc-1) selected++; }
            if (ch == KEY_DELETE) {
                // Delete key = delete task :D*
                goto do_delete;
            }
            continue;
        }

        switch (ch) {
        // navigation :D*
        case 'k': case 'K':
            if (selected > 0) selected--;
            break;
        case 'j': case 'J':
            if (selected < visible_count()-1) selected++;
            break;

        // view tabs :D*
        case '\t':
            view_mode = (view_mode + 1) % 3;
            selected = 0; scroll_off = 0;
            break;

        // add :D*
        case 'a': case 'A': {
            if (todo_count >= MAX_TODOS) {
                strcpy(status, "List full!");
                break;
            }
            Todo nt = {0};
            today_str(nt.date);
            nt.priority = PRI_MED;
            clrscr();
            if (form_edit(&nt, 1)) {
                todos[todo_count++] = nt;
                selected = todo_count - 1;
                save_todos();
                strcpy(status, "Task added");
            }
            clrscr();
            break;
        }

        // edit :D*
        case 'e': case 'E': {
            int vc = visible_count();
            if (vc == 0) break;
            int ri = vis_to_real(selected);
            if (ri < 0) break;
            clrscr();
            if (form_edit(&todos[ri], 0)) {
                save_todos();
                strcpy(status, "Task updated");
            }
            clrscr();
            break;
        }

        // toggle done :D*
        case ' ': {
            int vc = visible_count();
            if (vc == 0) break;
            int ri = vis_to_real(selected);
            if (ri >= 0) {
                todos[ri].done = !todos[ri].done;
                save_todos();
                strcpy(status, todos[ri].done ? "Marked done" : "Marked todo");
            }
            break;
        }

        // delete :D*
        case 'd': case 'D':
        do_delete: {
            int vc = visible_count();
            if (vc == 0) break;
            int ri = vis_to_real(selected);
            if (ri < 0) break;
            redraw(""); // ensure screen is clean for modal :D*
            if (modal(" Delete Task ", "Delete this task? This cannot be undone.")) {
                for (int i = ri; i < todo_count-1; i++) todos[i] = todos[i+1];
                todo_count--;
                if (selected >= visible_count() && selected > 0) selected--;
                save_todos();
                strcpy(status, "Task deleted");
            }
            clrscr();
            break;
        }

        // export CSV :D*
        case 'x': case 'X':
            export_csv();
            strcpy(status, "Exported todos.csv!");
            break;

        // quit :D*
        case 'q': case 'Q': case KEY_ESC:
            redraw("");
            if (modal(" Quit ", "Save & exit TODO?")) running = 0;
            else clrscr();
            break;
        }
    }

    clrscr();
    color(C_BWHITE, C_BLACK);
    gotoxy(0, 0);
    printf("Goodbye! Your tasks are saved in '%s'.\n", SAVE_FILE);
    printf("CSV export: '%s'  (press X next time to export)\n\n", CSV_FILE);
    show_cursor();
    return 0;
}
