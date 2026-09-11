/*
 * File:   lab13.c
 * Author: Jacky Li
 *
 * Lab 13 - Flight Software: Fault Tolerance the NASA Way
 * ECE 3301L - Introduction to Microcontrollers Laboratory
 *
 * The C "executive" for the fault-tolerance demo. The critical
 * routines (majority vote, checksum) live in fault.S - see that file
 * for the C<->asm interface convention.
 *
 * Subsystems demonstrated:
 *   1. TMR:      pot reading stored in 3 copies; S1 corrupts one copy;
 *                the vote outvotes it and counts the corrected upset.
 *   2. Integrity: 8-byte table protected by an asm checksum; S2
 *                corrupts a byte; detection triggers scrub-and-repair
 *                from the const gold copy.
 *   3. Watchdog: main loop pets the WDT; S1+S2 together fake a hang;
 *                the WDT resets the board, and startup reports
 *                "recovered from hang" via the reset-cause bits.
 *
 * Pin Assignments:
 *   RA0 (AN0) - potentiometer ("sensor")
 *   RD0-RD3   - corrected-upset counter (binary)
 *   RD4-RD7   - top 4 bits of the voted sensor value
 *   RA4       - integrity alarm LED (blinks during scrub)
 *   RB4 (S1)  - inject a TMR upset (corrupts one copy)
 *   RC5 (S2)  - corrupt the checksum table
 *   S1+S2     - fake a hang (stop petting the watchdog)
 *
 * Reset-cause display at boot (on RD0-RD7, ~2 s):
 *   0x0F = cold/power-on boot     0xF0 = recovered from WDT hang
 */

#include <xc.h>
#include <stdint.h>
#include "PIC18F46K22-Config.h"

#define _XTAL_FREQ 16000000UL

/* ---- Shared state: the asm kernel reads/writes these ---- */
volatile uint8_t tmr_a, tmr_b, tmr_c;
volatile uint8_t tmr_voted;
volatile uint8_t tmr_corrected;
volatile uint8_t chk_table[8];
volatile uint8_t chk_sum;

/* ---- Asm kernel entry points (fault.S) ---- */
extern void tmr_vote(void);
extern void mem_checksum(void);

/* Gold copy of the protected table (in flash, can't be upset) */
static const uint8_t gold_table[8] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x13, 0x37, 0x42, 0xA5
};

static uint8_t upset_count = 0;     /* corrected upsets, shown RD0-RD3 */
static uint8_t expected_sum;        /* checksum of the healthy table  */

static void init(void) {
    OSCCONbits.IRCF = 0b111;        /* 16 MHz HFINTOSC */
    OSCCONbits.SCS  = 0b10;

    /* RA0 analog in (pot), rest digital */
    ANSELA = 0x00;
    ANSELAbits.ANSA0 = 1;
    TRISAbits.TRISA0 = 1;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    TRISD = 0x00;  LATD = 0x00;     /* LEDs */
    TRISAbits.TRISA4 = 0;           /* alarm LED */
    LATAbits.LATA4 = 0;
    TRISBbits.TRISB4 = 1;           /* S1 */
    TRISCbits.TRISC5 = 1;           /* S2 */

    /* ADC: AN0, VDD/VSS, FOSC/32, 8 TAD, right-justified (Lab 6) */
    ADCON0bits.CHS  = 0;
    ADCON0bits.ADON = 1;
    ADCON1 = 0x00;
    ADCON2bits.ADFM = 1;
    ADCON2bits.ACQT = 0b100;
    ADCON2bits.ADCS = 0b010;
}

static uint8_t read_sensor(void) {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    /* keep the top 8 of the 10 bits - plenty for the demo */
    return (uint8_t)((((uint16_t)ADRESH << 8) | ADRESL) >> 2);
}

static void table_restore(void) {
    for (uint8_t i = 0; i < 8; i++) chk_table[i] = gold_table[i];
}

static void show_reset_cause(void) {
    if (RCONbits.TO == 0) {
        /* Watchdog timeout: we hung, the dog barked, we recovered. */
        for (uint8_t i = 0; i < 3; i++) {
            LATD = 0xF0; __delay_ms(300);
            LATD = 0x00; __delay_ms(200);
        }
        RCONbits.TO = 1;                /* re-arm the flag */
    } else {
        LATD = 0x0F; __delay_ms(800);   /* normal cold boot */
        LATD = 0x00;
    }
}

void main(void) {
    init();
    show_reset_cause();

    table_restore();
    mem_checksum();                 /* asm: checksum the healthy table */
    expected_sum = chk_sum;

    WDTCONbits.SWDTEN = 1;          /* watchdog ON - pet it or perish */

    while (1) {
        CLRWDT();                   /* pet the dog, every cycle */

        /* ---- 1. Sample and store with triple redundancy ---- */
        uint8_t s = read_sensor();
        tmr_a = s;  tmr_b = s;  tmr_c = s;

        /* ---- Fault injection (buttons are active-low) ---- */
        uint8_t s1 = (PORTBbits.RB4 == 0);
        uint8_t s2 = (PORTCbits.RC5 == 0);

        if (s1 && s2) {
            /* Fake a hang: flight task stuck in a loop, no petting.
             * ~1 s later the WDT resets us; show_reset_cause() will
             * report it on the next boot. */
            while (1) { ; }
        }
        if (s1) {
            tmr_b ^= 0xFF;          /* cosmic ray hits copy B */
        }
        if (s2) {
            chk_table[3] ^= 0xFF;   /* cosmic ray hits the table */
        }

        /* ---- 2. Vote (asm). A hit copy is outvoted silently. ---- */
        tmr_vote();
        if (tmr_corrected) {
            if (upset_count < 15) upset_count++;
        }

        /* ---- 3. Integrity check (asm) + scrub on mismatch ---- */
        mem_checksum();
        if (chk_sum != expected_sum) {
            /* corruption detected: alarm + restore from gold copy */
            for (uint8_t i = 0; i < 3; i++) {
                LATAbits.LATA4 = 1; __delay_ms(60);
                LATAbits.LATA4 = 0; __delay_ms(60);
            }
            table_restore();
        }

        /* ---- Display: RD7-RD4 = voted value, RD3-RD0 = upsets ---- */
        LATD = (uint8_t)((tmr_voted & 0xF0) | (upset_count & 0x0F));

        __delay_ms(100);            /* ~10 Hz loop, well inside WDT */
    }
}
