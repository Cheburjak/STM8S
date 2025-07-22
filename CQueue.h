#pragma once

#define CQ_MAXCAP 15

typedef struct{
  unsigned char cap;
  unsigned char cnt;
  unsigned char head;
  unsigned char tail;
  unsigned char data[CQ_MAXCAP];
}CQueue;

void CQueueInit(CQueue* cq, unsigned char cap);
void CQueuePush(CQueue* cq, unsigned char val);
unsigned char CQueuePop(CQueue* cq, unsigned char Default_);