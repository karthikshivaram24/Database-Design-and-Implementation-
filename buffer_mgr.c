#include "buffer_mgr.h"
#include <stdlib.h>
#include "caching_mechanism.h"


/*
 * This method initializes the buffer pool.
 *
 * @author Saurabh Tiwari
 */
RC
initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		  const int numPages, ReplacementStrategy strategy,
		  void *stratData)
{

	if (pageFileName == NULL || numPages<=0)
	{
		return RC_FILE_NOT_FOUND;
	}
	Stats *stats = MAKE_STATS();
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	Frame *frames = malloc (sizeof(Frame)*numPages);

	int i;
	for (i=0; i<bm->numPages; i++)
	{
		frames[i].pagedata = NULL;
		frames[i].pagenum = -1;
		frames[i].dirtybit = DIRTY_UNFLAG;
		frames[i].fixedcount = 0;
		frames[i].ranking = INT_MAX;
	}
	bm->mgmtData = frames;

	stats->hitcount = 0;
	stats->readcount = 0;
	stats->writecount = 0;
	stats->lastposition = 0;
	bm->statData = stats;
	return RC_OK;
}
/**
* This method is used for cleaning the memory like buffer pool and
* other data that has been used for many continuous operations.
* @author Sukrit Gulati
*/
RC
shutdownBufferPool(BM_BufferPool *const bm)
{
	Frame * frames = (Frame *)bm->mgmtData;
	forceFlushPool(bm);
	int i;
	bool isPageInBuffer = false;
	for (i = 0; i<bm->numPages; i++)
	{
		if(frames[i].fixedcount != 0)
		{
			isPageInBuffer = true;
		}
	}
	free(frames);
	frames = NULL;
	bm->mgmtData = NULL;
	if (isPageInBuffer)
	{
		return RC_PAGE_IN_BUFFER_ERROR;
	}
	else
	{
		return RC_OK;
	}
}

/**
* This method is used for force writing every page which has a dirty bit
* in it.
* @author Sukrit Gulati
*/
RC
forceFlushPool(BM_BufferPool *const bm){
    if (	bm->mgmtData == NULL){

        return RC_BUFFER_NOTINITIALIZED;
    }

 Frame * frames = (Frame *) bm->mgmtData;
 Stats * stat = (Stats*) bm->statData;
 int i;
 for (i = 0; i<bm->numPages; i++)
 {
	 if (frames[i].dirtybit == DIRTY_FLAG && frames[i].fixedcount == 0)
	 {
		 writeBlockToPage(bm, &frames[i]);
	 }
 }
 return RC_OK;
}


/**
* This method is used for marking the page dirtybit.
* If there is anything written to the page then it marks that page as dirty
*
* @author Seungho Han.
*/
RC
markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (	bm->mgmtData == NULL){

        return RC_BUFFER_NOTINITIALIZED;
    }

	Frame *frames = ((Frame *)(bm->mgmtData));
	int i;
	bool foundPageNum = false;
	for (i = 0; i<bm->numPages; i++)
	{
		if (frames[i].pagenum == page->pageNum)
		{
			frames[i].dirtybit = DIRTY_FLAG;
			foundPageNum = true;
			break;
		}
	}
	if (foundPageNum == true)
	{
		return RC_OK;

	} else {
			return RC_MARK_DIRTY_ERROR;
	}
}

/**
* Unpins the page from the current pool of buffer.
* That means that none of the clients are using it.
* @author Saurabh Tiwari
*/
RC
unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){

	Frame *frames = ((Frame *)(bm->mgmtData));
	int i;
	for (i = 0; i<bm->numPages; i++)
	{
		if (frames[i].pagenum == page->pageNum)
		{
			frames[i].fixedcount = frames[i].fixedcount - 1;
			return RC_OK;
		}
	}
	return RC_UNPIN_PAGE_ERROR;
}

/**
* Forces the page to be written to the disk.
* Irrespective of whther it is marked dirty or not.
* @author Sukrit Gulati
*/
RC
forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    int check;
    if (	bm->mgmtData == NULL){

    return RC_BUFFER_NOTINITIALIZED;
    }

    if (page->pageNum == -1 ) {//|| page->pageNum > bm->numPages
        return RC_PAGE_ERROR;
    }

	Frame * frames = (Frame *) bm->mgmtData;
	int i;
	for (i= 0; i<bm->numPages; i++)
	{
		if (frames[i].pagenum == page->pageNum)
		{
			check = writeBlockToPage(bm, &frames[i]);
		}
	}
    if(check == 0){
        return RC_OK;
    }else{
        return RC_FORCE_POOL_ERROR;
    }
}

/**
* Pins the page in the current buffer.
* also it is the primary method which decides on the basis of
* replacement strategy whther the page that has been pinned
* will replace by the replacement methods either LRU or FIFO.
*
* @author Saurabh Tiwari
*/
RC
pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
	    const PageNumber pageNum)
{
	Frame * frames = (Frame *) bm->mgmtData;
	Stats * stat = (Stats *)bm->statData;
	Frame *newframe = (Frame *) malloc(sizeof(Frame));
	int i;
	for (i=0 ; i<bm->numPages; i++)
	{
		if (frames[i].pagenum == -1)
		{
			SM_FileHandle filehandle;
			openPageFile (bm->pageFile, &filehandle);
			frames[i].pagedata = (SM_PageHandle) malloc(PAGE_SIZE);
			int readerror = readBlock(pageNum, &filehandle, frames[i].pagedata);
			if(i==0)
			{
				ensureCapacity(pageNum, &filehandle);
			}
			closePageFile(&filehandle);
			frames[i].pagenum = pageNum;
			frames[i].fixedcount = 1;
			stat->lastposition = stat->lastposition + 1;
			stat->readcount = stat->readcount + 1;
			bm->statData = stat;
			decreaseRankingForPages(bm, i);
			page->pageNum = pageNum;
			page->data = frames[i].pagedata;
			return RC_OK;
		}
		else if (frames[i].pagenum == pageNum)
		{
			if (frames[i].dirtybit == DIRTY_FLAG)
			{
				writeBlockToPage(bm, &frames[i]);
			}
			frames[i].fixedcount++;
			stat->lastposition = stat->lastposition;
			stat->hitcount = stat->hitcount + 1;
			decreaseRankingForPages(bm, i);
			bm->statData = stat;
			page->pageNum = pageNum;
			page->data = frames[i].pagedata;
			return RC_OK;
	  }
	}
	SM_FileHandle filehandle;
	openPageFile (bm->pageFile, &filehandle);
	newframe->pagedata = (SM_PageHandle) malloc(PAGE_SIZE);
	if (checkForLastPageNum(pageNum)){
		ensureCapacity(pageNum, &filehandle);
	}
	readBlock(pageNum, &filehandle, newframe->pagedata);
	closePageFile(&filehandle);
	newframe->pagenum = pageNum;
	newframe->fixedcount = 1;
	newframe->dirtybit = DIRTY_UNFLAG;
	newframe->ranking = INT_MAX;
	stat->readcount = stat->readcount + 1;
	bm->statData = stat;
	page->pageNum = pageNum;
	page->data = newframe->pagedata;

	if (bm->strategy == RS_FIFO)
	{
	 fifo(bm, newframe);
	 free(newframe);
	 newframe = NULL;
	 return RC_OK;
	}
	if (bm->strategy == RS_LRU)
	{
		lru(bm, newframe);
		free(newframe);
		newframe = NULL;
		return RC_OK;
	}
	return RC_PIN_PAGE_ERROR;
}

/**
* Gets the frame contents from the pages and returns the
* Page numbers which are present in the frame.
*
* @author Karthik Shivaram
*/
PageNumber
*getFrameContents (BM_BufferPool *const bm)
{
	PageNumber * pagenumberarray = malloc(sizeof(PageNumber)*bm->numPages);
	Frame * frames = (Frame *) bm->mgmtData;
	int i;
	for (i = 0; i<bm->numPages; i++)
	{
		pagenumberarray[i] = frames[i].pagenum;
	}
	return pagenumberarray;
}

/**
* Gets the dirty flags from the current set of frames
*
* @author Karthik Shivaram
*/
bool
*getDirtyFlags (BM_BufferPool *const bm)
{
    if (	bm->mgmtData == NULL){

        return false;
    }

	bool * dirtyflagarray = malloc(sizeof(bool)*bm->numPages);
	Frame * frames = (Frame *)bm->mgmtData;
	int i;
	for (i=0; i<bm->numPages; i++)
	{
		if(frames[i].dirtybit == DIRTY_FLAG)
		{
			dirtyflagarray[i] = true;
		} else
		{
			dirtyflagarray[i] = false;
		}
	}
	return dirtyflagarray;
}

/**
* Get the fix counts of elements that are in buffer pool.
*
* @author Seungho Han
*/
int
*getFixCounts (BM_BufferPool *const bm){


	int * fixedcountarray = malloc(sizeof(int)*bm->numPages);
	Frame * frames = (Frame *)bm->mgmtData;
	int i;
	for (i = 0; i<bm->numPages; i++)
	{
		fixedcountarray[i] = frames[i].fixedcount;
	}
	return fixedcountarray;
}

/**
* This method gets the total read operations performed
*
* @author Sukrit Gulati
*/
int
getNumReadIO (BM_BufferPool *const bm){

    if (	bm->mgmtData == NULL){

        return RC_BUFFER_NOTINITIALIZED;
    }

 if (bm->statData != NULL)
 {
	 Stats * stats = (Stats *) bm->statData;
	 return stats->readcount;
 } else
 {
	 return 0;
 }
}

/*
* This method gets the total number of write operations
* performed for a single opearation till the Buffer pool was
* freed from the memory.
*
* @author Seungho Han
*/
int
getNumWriteIO (BM_BufferPool *const bm)
{
    if (	bm->mgmtData == NULL){

        return RC_BUFFER_NOTINITIALIZED;
    }

 if (bm->statData != NULL)
 {
	 Stats * stats = (Stats *) bm->statData;
	 return stats->writecount;
 } else
 {
	 return 0;
 }
}

/**
* A method that will write block of memory to storage
*
* @author Karthik Shivaram
*/
int writeBlockToPage(BM_BufferPool *const bm, Frame *frame)
{
	SM_FileHandle fh;
	Stats * stats = (Stats *) bm->statData;
	openPageFile(bm->pageFile, &fh);
	writeBlock(frame->pagenum, &fh, frame->pagedata);
	closePageFile(&fh);
	frame->dirtybit = DIRTY_UNFLAG;
	stats->writecount++;
	bm->statData = stats;
	return RC_OK;
}

/**
* Checks whether the given page is last or not
* using the large page definition value defined
* as a constant.
* @author Saurabh Tiwari
*/
bool checkForLastPageNum(int pageNum)
{
	if(pageNum % (LARGE_PAGE-1) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
