#ifndef CACHING_MECHANISM_H
#define CACHING_MECHANISM_H

#include "buffer_mgr.h"

// method definitions of caching mechanisms
void fifo (BM_BufferPool *const bm, Frame *freshframe);
void lru (BM_BufferPool *const bm, Frame *freshframe);
void copyNewFrameInOld(Frame *oldframe, Frame *freshframe);
void decreaseRankingForPages(BM_BufferPool *const bm, int frameindex);

#endif
