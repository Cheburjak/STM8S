#include "CQueue.h"
void CQueueInit(CQueue* cq, unsigned char cap)
{
  cq->cnt = 0;
  cq->cap = cap;
  cq->head = 0;
  cq->tail = 0;
}

void CQueuePush(CQueue* cq, unsigned char val)
{
  if(cq->cnt < cq->cap)
  {
    cq->data[cq->tail] = val;
    cq->tail = (cq->tail+1) % cq->cap;
    cq->cnt++;
  }
}

unsigned char CQueuePop(CQueue* cq, unsigned char Default_)
{
  unsigned char val = Default_;
  if (cq->cnt > 0)
  {
    val = cq->data[cq->head];
    cq->head = (cq->head+1) % cq->cap;
    cq->cnt--;
  }
  return val;
}