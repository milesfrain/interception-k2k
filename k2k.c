#define _XOPEN_SOURCE 500
#ifdef VERBOSE
# include <stdio.h> /* fprintf() */
#endif
#include <stdlib.h> /* EXIT_FAILURE */
#include <errno.h> /* errno */
#include <unistd.h> /* STD*_FILENO, read() */
#include <string.h> /* memcpy() */
#include <linux/input.h> /* KEY_*, struct input_event */

#ifdef VERBOSE
# define dbgprintf(msg, ...) fprintf(stderr, msg "\n", ##__VA_ARGS__)
#else
# define dbgprintf(msg, ...) ((void)0)
#endif

enum event_values {
    EVENT_VALUE_KEYUP = 0,
    EVENT_VALUE_KEYDOWN = 1,
    EVENT_VALUE_KEYREPEAT = 2,
};

#define MAX_EVENTS 10

static struct input_event revbuf[MAX_EVENTS];
static size_t revlen = 0;
static size_t riev = 0;
static struct input_event wevbuf[MAX_EVENTS];
static size_t wevlen = 0;

static void
read_events(void) {
    for (;;) {
        switch ((revlen = read(STDIN_FILENO, revbuf, sizeof revbuf))) {
        case -1:
            if (errno == EINTR)
                continue;
            /* Fall through. */
        case 0:
            exit(EXIT_FAILURE);
        default:
            revlen /= sizeof *revbuf, riev = 0;
            return;
        }
    }
}

static void
write_events(void) {
    if (0 == wevlen)
        return;

    for (;;) {
        switch (write(STDOUT_FILENO, wevbuf, sizeof *wevbuf * wevlen)) {
        case -1:
            if (errno == EINTR)
                continue;
            exit(EXIT_FAILURE);
        default:
            wevlen = 0;
            return;
        }
    }
}

static void
write_event(struct input_event const *ev) {
    wevbuf[wevlen++] = *ev;
    /* Flush events if buffer is full. */
    if (wevlen == MAX_EVENTS)
        write_events();
}

static void
write_key_event(int code, int value) {
    struct input_event ev = {
        .type = EV_KEY,
        .code = code,
        .value = value
    };
    write_event(&ev);
}

/* KEY_* codes: /usr/include/linux/input-event-codes.h */

enum {
    STATE_START = 0,
    STATE_MIGHT_ESC = 1,
    STATE_NO_ESC = 2,
} keys_state = STATE_START;

int
main(void) {
    for (;;) {
        struct input_event ev;

        if (riev == revlen) {
            /* Flush events to be written. */
            write_events();
            /* Read new events. */
            read_events();
        }

        ev = revbuf[riev++];

        if (ev.type != EV_KEY) {
            if (ev.type == EV_MSC && ev.code == MSC_SCAN)
                goto ignore_event;

            goto write;
        }

        if (ev.code == KEY_CAPSLOCK) {
            dbgprintf("Got Caps");
            switch (ev.value) {
            case EVENT_VALUE_KEYDOWN:
                dbgprintf("    Ctrl down");
                write_key_event(KEY_LEFTCTRL, EVENT_VALUE_KEYDOWN);
                keys_state = STATE_MIGHT_ESC;
                dbgprintf("    set state to %d, %d", keys_state, STATE_MIGHT_ESC);
                goto ignore_event;
            case EVENT_VALUE_KEYREPEAT:
                dbgprintf("    Timeout, no ESC");
                keys_state = STATE_NO_ESC;
                goto ignore_event;
            case EVENT_VALUE_KEYUP:
                if (keys_state == STATE_MIGHT_ESC) {
                    dbgprintf("    Up before timeout. ESC");
                    write_key_event(KEY_LEFTCTRL, EVENT_VALUE_KEYUP);
                    write_key_event(KEY_ESC, EVENT_VALUE_KEYDOWN);
                    write_key_event(KEY_ESC, EVENT_VALUE_KEYUP);
                } else {
                    dbgprintf("    Up after timeout. No ESC");
                    write_key_event(KEY_LEFTCTRL, EVENT_VALUE_KEYUP);
                }
                keys_state = STATE_START;
                goto ignore_event;
            }
        } else if (ev.value == EVENT_VALUE_KEYDOWN) {
            dbgprintf("Got another: code %d, value %d, state %d", ev.code, ev.value, keys_state);
            if (keys_state == STATE_MIGHT_ESC) {
                /* Any other keypress cancels escape tap option */
                dbgprintf("Another keypress. No ESC");
                keys_state = STATE_NO_ESC;
            }
        }

    write:
        write_event(&ev);
    ignore_event:;

    }
}