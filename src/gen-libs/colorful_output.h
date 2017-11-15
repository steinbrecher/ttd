//
// Created by Greg Steinbrecher on 4/8/16.
//

#ifndef TTD_COLORFUL_OUTPUT_H
#define TTD_COLORFUL_OUTPUT_H

#ifdef COLORFUL_OUTPUT
#ifndef _WIN32
#define KNRM  "\x1B[0m"
#define KBLD  "\x1B[1m"
#define KITL  "\x1B[3m"
#define KUND  "\x1B[4m"

#define KRED  "\x1B[91m"
#define KGRN  "\x1B[32m"
#define KBGRN "\x1B[92m"
#define KYEL  "\x1B[93m"
#define KBLU  "\x1B[94m"
#define KMAG  "\x1B[95m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[97m"

#define CURSOROFF "\x1B[?25l"
#define CURSORON "\x1B[?25h"
#define CURSOR_TOP_LEFT "\x1B[1;1H"
#define CLEAR_SCREEN "\x1b[2J" CURSOR_TOP_LEFT

#define KHEAD1 KNRM KBLD KUND
#define KHEAD2 KNRM KBLD
#define KFILE KNRM KRED
#define KNUMBER KNRM KMAG KBLD
#define KTIME KNRM KBLD
#define KRATE KNRM KMAG
#else
#define KNRM
#define KBLD
#define KITL
#define KUND

#define KRED
#define KGRN
#define KBGRN
#define KYEL
#define KBLU
#define KMAG
#define KCYN
#define KWHT

#define CURSOROFF
#define CURSORON
#define CURSOR_TOP_LEFT
#define CLEAR_SCREEN

#define KHEAD1
#define KHEAD2
#define KFILE
#define KNUMBER
#define KTIME
#define KRATE
#endif // #ifndef _WIN32
#endif // #ifdef COLORFUL_OUTPUT


#endif //TTD_COLORFUL_OUTPUT_H
