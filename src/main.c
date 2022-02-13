#include <pm.h>

#include "small_chars_font.h"

#include <stdint.h>
#include <string.h>

#define NINTENDO_OFFSET 0x21A4
#define GAME_NAME_OFFSET 0x21AC

void load_tilemap(uint8_t *);
void vsync(void);
void keys(uint8_t a, uint8_t l);
void load_lines(uint8_t scroll, uint8_t cursor_pos);

// this data is used by the BIOS's game select menu for keeping track of stuff

struct {
    uint8_t cur_scroll;
    uint8_t per_page;
    uint8_t max_scroll;
} page_status _at(0x1528);

uint8_t imgbuf[0x0800] _at(0x1530);

uint8_t cur_cursor;

uint8_t get_keys(void) {
    uint8_t a = KEY_PAD;
    uint8_t b = KEY_PAD;
    
    while (b != a) {
        a = b;
        b = KEY_PAD;
    }
    
    return b ^ 0xFF;
}

// hard limit of 21 games, since the menu uses single bytes
// to hold numbers that are increased/decreased by 12
#define NUM_GAMES 21

struct {
    uint8_t pos;
    char name[12];
} games[NUM_GAMES];

const char nintendo_string[8] = "NINTENDO";

#define MIN(a, b) ((a) < (b) ? (a) : (b))
    
void draw_name(const char _far * name, uint8_t index) {
    uint8_t _near * dest = (uint8_t _near *)0x19D0 + index * 72;
    
    uint8_t i;
    uint8_t j;
    
    uint8_t drawn = 0;
    
    struct game {
        uint8_t len;
        uint8_t data[7];
    } * cur_symbol;
    
    for (i = 0; i < 12; i++) {
        unsigned char c = name[i];
        
        if (c == '0') {
            break;
        }
        
        if (c < 0x20) {
            c = 0;
        } else {
            c -= 0x20;
        }
        
        if (c > 0x80) {
            c -= 0x20;
        }
        
        cur_symbol = (struct game *)small_chars_font[c];
        
        for (j = 0; j < MIN(cur_symbol->len, 7); j++) {
            *dest++ = cur_symbol->data[j];
            drawn++;
        }
        
        for (; j < cur_symbol->len; j++) {
            *dest++ = 0;
            drawn++;
        }
    }
    
    while (drawn < 72) {
        *dest++ = 0;
        drawn++;
    }
}

void clear_name(uint8_t index) {
    uint8_t _near * dest = (uint8_t _near *)0x19D0 + index * 72;
    
    uint8_t drawn;
    
    for (drawn = 0; drawn < 72; drawn++) {
        *dest++ = 0;
    }
}

uint8_t num_games = 0;

#define BANK_SWITCH (*((volatile uint8_t _far *)0x1FFFFF))

uint8_t params_in_ram;

uint8_t function_in_ram[0x13];

void switch_memory_bank_ram(void) {
    BANK_SWITCH = params_in_ram;
    
    // we didn't enter normally (and the ROM is patched to never enter normally), so
    // in order to boot the first game (if that's what we've selected), we can't just
    // reboot - that will just launch the bootloader again. instead, we rely on the
    // way entering the bootloader works - the BIOS branches to __START partway through
    // booting, and the boot continues as normal when we return from it, but crucially,
    // after the check for MN at 0x2100 (which is what's patched). we can't return normally
    // from code() though, since we might have just left the bank (and there isn't enough
    // ROM space for two code paths, one for when we select the first game and one when we
    // don't), so instead I measured how much SP is increased between the start of code()
    // and now, then we use that as a constant in the program. we add 0x19 to SP and
    // return normally from this function, which acts like returning from code() and lets
    // us continue booting, with or without the switched bank.
#pragma asm
    ADD SP, #19h
#pragma endasm
}

void run_in_ram(void (*f_begin)(void)) {
    memcpy(function_in_ram, f_begin, sizeof(function_in_ram));
    ((void (*)(void))function_in_ram)();
}

void switch_memory_bank(uint8_t bank) {
    params_in_ram = bank;
    run_in_ram(switch_memory_bank_ram);
}

int code(void) {
    uint8_t prev_keys = 0;
    uint8_t key_timer = 0;
    
    uint8_t i;
    uint32_t j;
    
    num_games = 0;
    
    for (i = 0, j = 0; i < 32 && num_games < NUM_GAMES; i++, j += 0x010000) {
        if (strncmp((const char *)(j + NINTENDO_OFFSET), nintendo_string, 8) == 0) {
            games[num_games].pos = i;
            memcpy(games[num_games].name, (uint8_t *)(j + GAME_NAME_OFFSET), 16);
            
            draw_name(games[num_games].name + 4, num_games);
            
            num_games++;
        }
    }
    
    for (i = num_games; i < 5; i++) {
        clear_name(i);
    }
    
    cur_cursor = 0;
    
    page_status.cur_scroll = 0 * 0x0C;
    page_status.per_page = MIN(4, num_games - 1) * 0x0C;
    page_status.max_scroll = (num_games - 1) * 0x0C;
    
    // we enter from BIOS context, so the image data and
    // tilemaps are already in RAM at known offsets
    
    load_tilemap((uint8_t *)0x1590);
    load_lines(page_status.cur_scroll, cur_cursor);

    while (1) {
        uint8_t new_keys;
        
        vsync();
        
        if ((new_keys = get_keys()) != prev_keys) {
            
            if (new_keys & KEY_A) {
                i = games[cur_cursor / 12].pos;
                
                // copied from zoranc's bootloader
                
                if ((i & 0x07) == 0) {
                    switch_memory_bank(0x80 | (i >> 3));
                } else {
                    switch_memory_bank(i);
                }
                
            } else {
                keys(KEY_PAD ^ 0xFF, cur_cursor);
            }
            
            prev_keys = new_keys;
            key_timer = 0;
            
        } else if (prev_keys != 0) {
            if (key_timer > 36) {
                keys(KEY_PAD ^ 0xFF, cur_cursor);
            } else {
                key_timer++;
            }
        }
    }
}

#pragma asm

; // trampolines

_load_tilemap:
    LD XP, #0 ; // the BIOS function doesn't clear XP
    JRL 0BDCh

_vsync:
    JRL 0BFFh
    
_keys: ; // need a more complex trampoline to handle this function
    PUSH IX
    LD IX, #_page_status
    CARL 0C8Bh
    POP IX
    LD [_cur_cursor], L
    RET
    
_load_lines:
    JRL 0CCCh

; // from c88's libc, since alc88 can't handle linking against library .asm files

__MULXI:	PUSH	IP		; save XP, YP        
Step1:          LD      BA, IY          ; load '2' in A
                PUSH    B               ; save '1' on stack
                LD      HL, IX          ; load 'b' in L
                MLT
                EX      BA, HL                          
                LD      YP, A           ; store result in 'n', overflow stays in B

                LD      A, L            ; recall multiplier
                LD      HL, IX          ; load 'a'
                LD      L, H
                MLT
                LD      A, L
                ADD     A, B            ; add overflow
                LD      XP, A           ; store result in 'm'
                LD      B, H            ; recall overflow
                JRS     NC, Step2
                INC     B               ; correct overflow in B
                
Step2:          LD      HL, IX          ; load 'b' in L
                POP     A               ; load '1' in A
                MLT
                LD      A, XP           ; recall previous result
                LD      B, L            ; get current result
                ADD     A, B            ; add result
                
ReturnAdmin:    LD      B, A            ; store 'm'
                LD      A, YP           ; store 'n'
                
                POP	    IP		; restore XP, YP
		RET

#pragma endasm
