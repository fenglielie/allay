#include "allay/pbar/pbar.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_NUM 20
#define FILL_CHAR '#'
#define ALPHA 0.4  // Smoothing factor

#ifdef _WIN32
static LARGE_INTEGER freq;  // time frequency
#endif

static void pbar_time_init(void) {
#ifdef _WIN32
    QueryPerformanceFrequency(&freq);
#endif
}

static void pbar_get_time(pbar_time_point *tp) {
#ifdef _WIN32
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    tp->counter = now.QuadPart;
#else
    clock_gettime(CLOCK_MONOTONIC, &(tp->ts));
#endif
}

static double pbar_time_diff(pbar_time_point *tp_start,
                             pbar_time_point *tp_end) {
#ifdef _WIN32
    return (double)(tp_end->counter - tp_start->counter)
           / (double)freq.QuadPart;  // seconds
#else
    return (double)(tp_end->ts.tv_sec - tp_start->ts.tv_sec)
           + (double)(tp_end->ts.tv_nsec - tp_start->ts.tv_nsec)
                 / 1e9;  // seconds
#endif
}

static void update_data(pbar *pb, double arg_now) {
    if (arg_now < pb->last_arg) { return; }

    pbar_time_point tp_now;
    pbar_get_time(&tp_now);
    double elapsed_time = pbar_time_diff(&pb->last_time, &tp_now);

    double cur_rate = (arg_now - pb->last_arg) / elapsed_time;

    if (!pb->initialized || isnan(pb->last_rate) || isinf(pb->last_rate)) {
        pb->last_rate = cur_rate;
        pb->initialized = 1;
    }
    else { pb->last_rate = ALPHA * cur_rate + (1 - ALPHA) * pb->last_rate; }

    pb->last_time = tp_now;
    pb->last_arg = arg_now;
}

static void color_label(double pct, char *color_label_buffer,
                        size_t color_label_buffer_len) {
    int r = 0;
    int g = 0;
    int b = 0;

    if (pct < 0.5) {
        r = 255;
        g = (int)(255 * (pct / 0.5));
        b = 0;
    }
    else {
        r = (int)(255 * (1 - (pct - 0.5) / 0.5));
        g = 255;
        b = (int)(255 * ((pct - 0.5) / 0.5));
    }

    snprintf(color_label_buffer, color_label_buffer_len, "\033[38;2;%d;%d;%dm",
             r, g, b);
}

static void time_str(double dt, char *time_buffer, size_t time_buffer_len) {
    if (dt < 1200) { snprintf(time_buffer, time_buffer_len, "%6.2fs", dt); }
    else if (dt < 7200) {
        snprintf(time_buffer, time_buffer_len, "%6.2fm", dt / 60);
    }
    else { snprintf(time_buffer, time_buffer_len, "%6.2fh", dt / 3600); }
}

static void pct_bar(double pct, char *bar, size_t bar_len) {
    int filled_num = (int)(pct * BLOCK_NUM);
    for (int i = 0; i < BLOCK_NUM; ++i) {
        bar[i] = (i < filled_num) ? FILL_CHAR : ' ';
    }
    bar[BLOCK_NUM] = '\0';
}

void pbar_init(pbar *pb, double arg_end, PBAR_FORMAT format) {
    if (arg_end <= 0.0) {
        fprintf(stderr, "ProgressBar: arg_end must be greater than 0\n");
        exit(EXIT_FAILURE);
    }

    pb->arg_end = arg_end;
    pb->last_arg = 0;
    pb->last_rate = 0.0;
    pb->initialized = 0;
    pb->format = format;

    pbar_time_init();
    pbar_get_time(&pb->start_time);
    pb->last_time = pb->start_time;
}

double pbar_time_cost(pbar *pb) {
    pbar_time_point tp_now;
    pbar_get_time(&tp_now);
    return pbar_time_diff(&pb->start_time, &tp_now);
}

static double pbar_clamp(double val, double m1, double m2) {
    if (val < m1) { return m1; }
    if (val > m2) { return m2; }
    return val;
}

static void pbar_update_detail(pbar *pb, double arg_now, int stage_num,
                               char leading_char, char trailing_char) {
    int stage_old = (int)(pb->last_arg / pb->arg_end * stage_num);
    int stage_new = (int)(arg_now / pb->arg_end * stage_num);

    update_data(pb, arg_now);

    if (stage_new == stage_old && stage_num > 0) { return; }

    double pct = pbar_clamp(pb->last_arg / pb->arg_end, 0.0, 1.0);
    double eta_time =
        (pb->last_rate > 0) ? (pb->arg_end - pb->last_arg) / pb->last_rate : 0;
    double cost_time = pbar_time_cost(pb);

    char color_label_buffer[20] = {0};
    color_label(pct, color_label_buffer, 20);

    char cost_time_buffer[20] = {0};
    time_str(cost_time, cost_time_buffer, 20);

    char eta_time_buffer[20] = {0};
    time_str(eta_time, eta_time_buffer, 20);

    char bar[BLOCK_NUM + 1] = {0};
    pct_bar(pct, bar, BLOCK_NUM + 1);

    switch (pb->format) {
    case PCT:
        printf("%c %s%6.2f%%\033[0m%c", leading_char, color_label_buffer,
               pct * 100, trailing_char);
        break;
    case ARG:
        printf("%c %s%.2f->%.2f\033[0m%c", leading_char, color_label_buffer,
               pb->last_arg, pb->arg_end, trailing_char);
        break;
    case TIME:
        printf("%c %sET:%s RT:%s\033[0m%c", leading_char, color_label_buffer,
               cost_time_buffer, eta_time_buffer, trailing_char);
        break;
    case PCT_TIME:
        printf("%c %s%6.2f%% ET:%s RT:%s\033[0m%c", leading_char,
               color_label_buffer, pct * 100, cost_time_buffer, eta_time_buffer,
               trailing_char);
        break;
    case PCT_BAR_ARG:
        printf("%c %s[%6.2f%%][%s][%.2f->%.2f]\033[0m%c", leading_char,
               color_label_buffer, pct * 100, bar, pb->last_arg, pb->arg_end,
               trailing_char);
        break;
    case PCT_BAR_ARG_TIME:
        printf("%c %s[%6.2f%%][%s][%.2f->%.2f] ET:%s RT:%s\033[0m%c",
               leading_char, color_label_buffer, pct * 100, bar, pb->last_arg,
               pb->arg_end, cost_time_buffer, eta_time_buffer, trailing_char);
        break;
    default: break;
    }

    fflush(stdout);
}

void pbar_update(pbar *pb, double arg_now, int stage_num) {
    pbar_update_detail(pb, arg_now, stage_num, '\r', '\0');
}

void pbar_update_newline(pbar *pb, double arg_now, int stage_num) {
    pbar_update_detail(pb, arg_now, stage_num, '\0', '\n');
}
