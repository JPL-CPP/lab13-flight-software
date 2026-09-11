; =====================================================================
; File:   fault.S
; Author: <your names here>
;
; Lab 13 - Flight Software: Fault Tolerance the NASA Way
; ECE 3301L - Introduction to Microcontrollers Laboratory
;
; YOU WRITE this assembly kernel. The C executive (lab13.c, provided,
; do not modify) declares the shared globals and calls your routines:
;
;   volatile uint8_t tmr_a, tmr_b, tmr_c;   three redundant copies
;   volatile uint8_t tmr_voted;             majority-vote result
;   volatile uint8_t tmr_corrected;         nonzero if a copy disagreed
;   volatile uint8_t chk_table[8];          protected data table
;   volatile uint8_t chk_sum;               computed checksum
;
;   void tmr_vote(void);     vote a/b/c -> tmr_voted, tmr_corrected
;   void mem_checksum(void); sum chk_table[0..7] -> chk_sum
;
; Interface convention: globals in, globals out. C symbol `x` is
; `_x` here. Use movff to move between banked C globals and your
; access-bank scratch; use banksel + BANKMASK() to write C globals
; directly.
; =====================================================================

#include <xc.inc>

GLOBAL  _tmr_vote, _mem_checksum            ; exported to C
GLOBAL  _tmr_a, _tmr_b, _tmr_c              ; imported from C
GLOBAL  _tmr_voted, _tmr_corrected
GLOBAL  _chk_table, _chk_sum

; scratch in the access bank - reachable without BANKSEL
PSECT udata_acs
va:     DS 1
vb:     DS 1
vc:     DS 1
cnt:    DS 1

PSECT ftext,class=CODE,reloc=2

; ---------------------------------------------------------------------
; tmr_vote - Triple Modular Redundancy majority vote.
;
;   voted = (a==b) ? a : (a==c) ? a : b
;   corrected = nonzero if ANY copy disagreed
;
; TODO:
;   1. movff the three C globals into va/vb/vc
;   2. clear _tmr_corrected  (banksel _tmr_corrected then
;      clrf BANKMASK(_tmr_corrected),b)
;   3. compare with cpfseq (compare F with W, skip if equal):
;        a == b ?  yes -> is c also equal? if not, flag corrected.
;                  voted = a either way.
;        a != b ?  a == c means b was hit (a wins);
;                  otherwise a is the odd one out (b wins).
;                  flag corrected in both cases.
;   4. movff the winner into _tmr_voted
; ---------------------------------------------------------------------
_tmr_vote:
    ; TODO - implement the vote (see steps above)
    return

; ---------------------------------------------------------------------
; mem_checksum - integrity check over chk_table[0..7].
;
; JPL "Power of 10" rule 2: every loop has a fixed, provable bound.
; Yours is exactly 8.
;
; TODO:
;   1. lfsr 0,_chk_table       (FSR0 -> table)
;   2. counter = 8, sum = 0
;   3. loop: add POSTINC0 into the sum, decfsz the counter
;   4. movff the sum into _chk_sum
; ---------------------------------------------------------------------
_mem_checksum:
    ; TODO - implement the checksum (see steps above)
    return

    END
