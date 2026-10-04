# ECE 3301L — Lab 13: Flight Software - Fault Tolerance the NASA Way

Starter code for the PIC18F46K22 (MPLAB X + XC8 v3.10).
MPLAB project(s) in this repo: `Lab13.X`.

## Getting the code

This lab is **released into your team's private pair repository**
(`ece3301l-pair-NN`) when the lab opens — just `git pull` there.
This public repo is a reference copy you can browse or download
(green **Code** button → Download ZIP) if you want to look ahead or
work on a machine before your pair repo is set up.

**All graded work must be committed to your pair repository** — work
committed anywhere else is not graded.

## Doing the lab

1. In your pair repo, open the project folder in MPLAB X.
2. Complete every `TODO` in the source file. The comments walk you
   through the steps; the datasheet and lab handout have the details.
3. Commit and push as you go:

   ```bash
   git add Lab13.X
   git commit -m "describe what you did"
   git push
   ```

4. Check the **Actions** tab after each push. A green check means your
   code compiles and produces firmware. A red X links to the compiler
   errors. **Green is required but is not the grade** — correct
   behavior is checked on hardware in lab.

## Submitting

1. Push your final version and wait for the green check.
2. On your pair repo's page: **Commits** → click your final commit →
   copy the browser URL. It must contain `/commit/`, like
   `https://github.com/JPL-CPP/ece3301l-pair-17/commit/a9c34f2...`
3. **One partner** pastes that URL into the Canvas assignment before
   the deadline. Canvas's timestamp is the official submission time.
4. Demo the working hardware to the instructor/TA during lab checkoff.

Both partners must contribute commits — commit history is part of the
grade record.

<!-- ci-checks:start -->
## What Lab CI checks in `Lab13.X`

Every push to `main` runs these automatically. Open the run in the **Actions** tab: the
checklist and the warnings below appear in the run summary and as annotations on your lines.

### 1. It must compile

`xc8-cc -Wall` (or `pic-as` for assembly) exactly as the grader builds it. A red X here is the
only thing that fails the run. Warnings in *your* files are listed with file and line.

### 2. Required structures (the grader's static score)

The run summary shows this list with a check mark for each one it finds in your code.
Comments do not count. These are the same patterns the grader scores.

- [ ] Majority vote compares with cpfseq
- [ ] Voted timer value exported (_tmr_voted)
- [ ] Checksum routine exported (_chk_sum)
- [ ] Memory walked with POSTINC

### 3. Mistakes this lab is known for

Warnings only: they never turn the check red, but each one has cost a pair real hours on the bench.
The id in brackets is what you will see on the annotation.

- **[L13-global]** (warns if missing) _tmr_voted and _chk_sum must be declared GLOBAL in fault.S or the C executive cannot link to them.
- **[L13-return]** (warns if missing) Routines called from C must end in return.
- **[L13-add]** (warns if missing) The checksum needs an add (addwf) across the block.

### Generic warnings on every lab

- **[C1]** Writing to PORTx instead of LATx (read-modify-write on the pins).
- **[C2]** __delay_ms/us used but _XTAL_FREQ not defined.
- **[C3]** main() has no while(1) loop.
- **[C4]** Assignment (=) inside an if/while condition.
- **[C5]** An interrupt function exists but GIE is never set.
- **[C6]** No interrupt flag (xxIF = 0) is ever cleared.
- **[C7]** PORTx read but no ANSELx configured (analog pins read 0).
- **[C8]** No TRISx assignment at all.
- **[A1]** Assembly: ANSELx written through the access bank (,a) instead of banksel + ,b.
- **[A2]** Assembly: no #include <xc.inc>.
- **[A3]** Assembly: retfie without ,1.
- **[A4]** Obsolete -presetVec/-pintVec linker flags present.

### What CI cannot see

Timing, wiring, display polarity, a dead breadboard row, the wrong chip in the socket.
A green check means it builds. Behavior is checked on hardware at check-off.
<!-- ci-checks:end -->
