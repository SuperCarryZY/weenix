#include "drivers/tty/tty.h"
#include "drivers/chardev.h"
#include "drivers/dev.h"
#include "drivers/keyboard.h"
#include "kernel.h"
#include "mm/kmalloc.h"
#include "util/debug.h"
#include <errno.h>

#ifndef NTERMS
#define NTERMS 3
#endif

ssize_t tty_read(chardev_t *cdev, size_t pos, void *buf, size_t count);
ssize_t tty_write(chardev_t *cdev, size_t pos, const void *buf, size_t count);

chardev_ops_t tty_cdev_ops = {.read = tty_read,
                              .write = tty_write,
                              .mmap = NULL,
                              .fill_pframe = NULL,
                              .flush_pframe = NULL};

tty_t *ttys[NTERMS] = {NULL};

size_t active_tty;

static void tty_receive_char_multiplexer(uint8_t c);

void tty_init()
{
    for (unsigned i = 0; i < NTERMS; i++)
    {
        tty_t *tty = ttys[i] = kmalloc(sizeof(tty_t));
        vterminal_init(&tty->tty_vterminal);
        ldisc_init(&tty->tty_ldisc);

        tty->tty_cdev.cd_id = MKDEVID(TTY_MAJOR, i);
        list_link_init(&tty->tty_cdev.cd_link);
        tty->tty_cdev.cd_ops = &tty_cdev_ops;

        kmutex_init(&tty->tty_write_mutex);
        kmutex_init(&tty->tty_read_mutex);

        long ret = chardev_register(&tty->tty_cdev);
        KASSERT(!ret);
    }
    active_tty = 0;
    vterminal_make_active(&ttys[active_tty]->tty_vterminal);
    KASSERT(ttys[active_tty]);

    keyboard_init(tty_receive_char_multiplexer);
}

/**
 * Reads from the tty to the buffer.
 *
 * You should first lock the read mutex of the tty. You should
 * then wait until there is something in the line discipline's buffer and only
 * read from the ldisc's buffer if there are new characters.
 *
 * To prevent being preempted, you should set IPL using INTR_KEYBOARD
 * correctly and revert it once you are done.
 *
 * @param  cdev  the character device that represents tty
 * @param  pos   the position to start reading from; should be ignored
 * @param  buf   the buffer to read into
 * @param  count the maximum number of bytes to read
 * @return       the number of bytes actually read into the buffer
 */
ssize_t tty_read(chardev_t *cdev, size_t pos, void *buf, size_t count)
{
    tty_t *tty = cd_to_tty(cdev);
    
    // Lock the read mutex and set interrupt level to prevent preemption
    kmutex_lock(&tty->tty_read_mutex);
    uint8_t old_ipl = intr_setipl(INTR_KEYBOARD);
    
    // Wait until there are characters available to read
    long ret = ldisc_wait_read(&tty->tty_ldisc);
    if (ret < 0){
        intr_setipl(old_ipl);
        kmutex_unlock(&tty->tty_read_mutex);
        return ret;
    }
    
    // Read from the line discipline buffer and restore interrupt level
    size_t bytes_read = ldisc_read(&tty->tty_ldisc, (char *)buf, count);
    intr_setipl(old_ipl);
    kmutex_unlock(&tty->tty_read_mutex);
    return bytes_read;
}

/**
 * Writes to the tty from the buffer.
 *
 * You should first lock the write mutex of the tty. Then you can use 
 * `vterminal_write` to write to the terminal. Don't forget to use IPL to 
 * guard this from preemption!
 *
 * @param  cdev  the character device that represents tty
 * @param  pos   the position to start reading from; should be ignored
 * @param  buf   the buffer to read from
 * @param  count the maximum number of bytes to write to the terminal
 * @return       the number of bytes actually written
 */
ssize_t tty_write(chardev_t *cdev, size_t pos, const void *buf, size_t count)
{
    tty_t *tty = cd_to_tty(cdev);
    
    // Lock the write mutex and set interrupt level to prevent preemption
    kmutex_lock(&tty->tty_write_mutex);
    uint8_t old_ipl = intr_setipl(INTR_KEYBOARD);
    
    // Write to the virtual terminal and restore interrupt level
    size_t bytes_written = vterminal_write(&tty->tty_vterminal, (const char *)buf, count);
    intr_setipl(old_ipl);
    kmutex_unlock(&tty->tty_write_mutex);
    return bytes_written;
}

static void tty_receive_char_multiplexer(uint8_t c)
{
    tty_t *tty = ttys[active_tty];

    if (c >= F1 && c <= F12)
    {
        if (c - F1 < NTERMS)
        {
            /* TODO: this is totally unsafe... Fix it */
            active_tty = (unsigned)c - F1;
            tty = ttys[active_tty];
            vterminal_make_active(&tty->tty_vterminal);
        }
        return;
    }
    if (c == CR)
        c = LF;
    else if (c == DEL)
        c = BS;

    vterminal_t *vt = &tty->tty_vterminal;
    switch ((unsigned)c)
    {
    case SCROLL_DOWN:
    case SCROLL_UP:
        // vterminal_scroll(vt, c == SCROLL_DOWN ? 1 : -1);
        break;
    case SCROLL_DOWN_PAGE:
    case SCROLL_UP_PAGE:
        // vterminal_scroll(vt, c == SCROLL_DOWN_PAGE ? vt->vt_height :
        // -vt->vt_height);
        break;
    case ESC:
        // vterminal_scroll_to_bottom(vt);
        break;
    default:
        ldisc_key_pressed(&tty->tty_ldisc, c);
        // vterminal_key_pressed(vt);
        break;
    }
}
