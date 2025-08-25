#pragma once

//-----------------------------------------
#ifdef FQ_16MHZ
#define BUSYCOUNT 0xa65 / 1
#define DELAY_15MKS() asm("push #80\n"   \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_5MKS() asm("push #26\n"   \
                         "del:\n"       \
                         "dec (1,sp)\n" \
                         "jrne del\n"   \
                         "pop a");
#define DELAY_50MKS() asm("push #200\n"  \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "nop\n"        \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_100MKS() asm("push #229\n"  \
                           "del:\n"       \
                           "dec (1,sp)\n" \
                           "nop\n"        \
                           "nop\n"        \
                           "nop\n"        \
                           "nop\n"        \
                           "jrne del\n"   \
                           "pop a");
#define DELAY_500MKS() asm("push #100\n"  \
                           "push #11\n"   \
                           "del:\n"       \
                           "dec (2,sp)\n" \
                           "jrne del\n"   \
                           "dec (1,sp)\n" \
                           "jrne del\n"   \
                           "pop a\n"      \
                           "pop a");
#define CLK_CKDIVR 0x00;
#endif

//-----------------------------------------
#ifdef FQ_8MHZ
#define BUSYCOUNT 0xa65 / 2
#define DELAY_15MKS() asm("push #40\n"   \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_5MKS() asm("push #13\n"   \
                         "del:\n"       \
                         "dec (1,sp)\n" \
                         "jrne del\n"   \
                         "pop a");
#define DELAY_50MKS() asm("push #130\n"  \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_100MKS() asm("push #200\n"  \
                           "del:\n"       \
                           "dec (1,sp)\n" \
                           "nop\n"        \
                           "jrne del\n"   \
                           "pop a");
#define DELAY_500MKS() asm("push #50\n"   \
                           "push #6\n"    \
                           "del:\n"       \
                           "dec (2,sp)\n" \
                           "jrne del\n"   \
                           "dec (1,sp)\n" \
                           "jrne del\n"   \
                           "pop a\n"      \
                           "pop a");
#define CLK_CKDIVR = 0x01;
#endif

//-----------------------------------------
#ifdef FQ_4MHZ
#define BUSYCOUNT 0xa65 / 4
#define DELAY_15MKS() asm("push #20\n"   \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_5MKS() asm("push #6\n"    \
                         "del:\n"       \
                         "dec (1,sp)\n" \
                         "jrne del\n"   \
                         "pop a");
#define DELAY_50MKS() asm("push #66\n"   \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_100MKS() asm("push #133\n"  \
                           "del:\n"       \
                           "dec (1,sp)\n" \
                           "jrne del\n"   \
                           "pop a");
#define DELAY_500MKS() asm("push #153\n"  \
                           "push #3\n"    \
                           "del:\n"       \
                           "dec (2,sp)\n" \
                           "jrne del\n"   \
                           "dec (1,sp)\n" \
                           "jrne del\n"   \
                           "pop a\n"      \
                           "pop a");
#define CLK_CKDIVR = 0x02;
#endif

//-----------------------------------------
#ifdef FQ_2MHZ
#define BUSYCOUNT 0xa65 / 8
#define DELAY_15MKS() asm("push #10\n"   \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_5MKS() asm("push #3\n"    \
                         "del:\n"       \
                         "dec (1,sp)\n" \
                         "jrne del\n"   \
                         "pop a");
#define DELAY_50MKS() asm("push #33\n"   \
                          "del:\n"       \
                          "dec (1,sp)\n" \
                          "jrne del\n"   \
                          "pop a");
#define DELAY_100MKS() asm("push #67\n"   \
                           "del:\n"       \
                           "dec (1,sp)\n" \
                           "jrne del\n"   \
                           "pop a");
#define DELAY_500MKS() asm("push #75\n"   \
                           "push #2\n"    \
                           "del:\n"       \
                           "dec (2,sp)\n" \
                           "jrne del\n"   \
                           "dec (1,sp)\n" \
                           "jrne del\n"   \
                           "pop a\n"      \
                           "pop a");
#define CLK_CKDIVR = 0x03;
#endif

//-----------------------------------------
#ifdef BUSYCOUNT
static void delay_ms(int ms)
{
  while (ms)
  {
    for (int i = 0; i < BUSYCOUNT; i++)
      ;
    ms--;
  };
};
#endif