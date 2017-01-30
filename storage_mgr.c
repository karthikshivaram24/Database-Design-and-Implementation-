#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "storage_mgr.h"


void
initStorageManager (void)
{

}

/*******************************************************************
* NAME :              createPageFile (char *fileName)
*
* DESCRIPTION :       Creates a new page file fileName. The initial file size will be one page.
*                     This method will fill this single page with '\0' bytes.
* INPUTS :
*       PARAMETERS:
*           char *    fileName              Name of the File to create and save
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                     Error code:
*            Values: RC_FILE_NOT_PAGE_SIZE  (9)File not created of one PAGE_SIZE.
*                    RC_OK                  (0)Function successfully executed.
* PROCESS :
*                   [1]  Create Empty File of File Name.
*                   [2]  Allocate Memmory of one Page Size to this empty file and Initialize it with zero.
*                   [3]  Check the size of the created file.
*                   [4]  Throw error if file is of not right size.
*                   [5]  Return RC_OK if file was created successfully of one PAGE_SIZE.
* AUTHOR :
* DATE    NAME
* 20Sep16 Seungho Han
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Seungho Han        Original code
*/

RC
createPageFile (char *fileName)
{
  FILE *fstream;
  fstream = fopen(fileName, "w+" );
  char *filepointer = (char *) calloc(PAGE_SIZE, sizeof(char));
  size_t sizeOfFile = fwrite(filepointer, sizeof(char), PAGE_SIZE, fstream);
  if(sizeOfFile != PAGE_SIZE)
  {
    return RC_FILE_NOT_PAGE_SIZE ;
  }
  fclose(fstream);
  return RC_OK;
}


/*******************************************************************
* NAME :            openPageFile (char *fileName,, SM_FileHandle *fHandle)
*
* DESCRIPTION :     Opens an existing page file. It will return RC_FILE_NOT_FOUND if the file does not exist.
*                   The second parameter is an existing file handle. If opening the file is successful,
*                   then the fields of this file handle will be initialized with the information about the opened file.
* INPUTS :
*       PARAMETERS:
*           char                  *fileName    File to open
*           struct SM_FileHandle  *fHandle     Information about the file and it's Properties (in storage_mgr.h)
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                     Error code:
*            Values: RC_FILE_NOT_FOUND      File not found to open.
*                    RC_OK                  File is opened successfully.
* PROCESS :
*                   [1]  Opens the file using File Name.
*                   [2]  If the file stream returns null then the file does not exist.
*                   [3]  Else we seek to the end of the file and obtain file size.
*                   [4]  Then we initialize all the fields of SM_FileHandle.
*                   [5]  After successful initialization we Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Seungho Han
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Seungho Han        Original code
*/

RC
openPageFile (char *fileName, SM_FileHandle *fHandle){
  FILE *fstream;
  fstream = fopen(fileName, "r+");
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  else
  {
    fseek(fstream, 0L, SEEK_END);
    long file_sz = ftell(fstream);

    fHandle->fileName = fileName;
    fHandle->totalNumPages = (int)file_sz/PAGE_SIZE;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = fstream;
  }

  return RC_OK;
}


/*******************************************************************
* NAME :            closePageFile (SM_FileHandle *fHandle)
*
* DESCRIPTION :     Closes an open page file.
*
* INPUTS :
*       PARAMETERS:
*           struct SM_FileHandle  *fHandle     Information about the file and it's Properties (in storage_mgr.h)
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_HANDLE_NOT_INIT     File Handle not initialized.
*                    RC_OK                       File is closed successfully.
*
* PROCESS :
*                   [1]  Obtains current file Stream from the "mgmtinfo" from *fHandle.
*                   [2]  Checks if the file stream is Null, or if the Filename and File Stream is null.
*                   [3]  If NULL then we return File Handle not initialized error.
*                   [4]  Else we close the File, using the File Stream.
*                   [5]  After successful closing we Return RC_OK.
*
* AUTHOR :
* DATE    NAME
* 20Sep16 Seungho Han
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Seungho Han        Original code
*/

RC
closePageFile (SM_FileHandle *fHandle)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL || (fstream == NULL && fHandle->fileName == NULL))
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  else
  {
    fclose(fstream);
  }
  return RC_OK;
}

/*******************************************************************
* NAME :            destroyPageFile (char *fileName)
*
* DESCRIPTION :     Deletes a page file.
*
* INPUTS :
*       PARAMETERS:
*           char       *fileName    File to destroy/delete.
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                     Error code:
*            Values: RC_FILE_NOT_DELETED    File not deleted.
*                    RC_OK                  File is deleted successfully.
*
* PROCESS :
*                   [1]  Deletes the file using filename.
*                   [2]  If error occured during deleting then we return File not deleted error.
*                   [3]  If File was deleted successfully we return RC_OK.
*
* AUTHOR :
* DATE    NAME
* 20Sep16 Sukrit Gulati
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Sukrit Gulati      Original code
*/
RC
destroyPageFile (char *fileName)
{
  int error_code = remove(fileName);
  if (error_code != 0)
  {
    return RC_FILE_NOT_DELETED;
  }
  return RC_OK;
}


/*******************************************************************
* NAME :              readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       The method reads the pageNumth block from a file and stores its content in the memory pointed to by the memPage page handle.
*                     If the file has less than pageNum pages, the method will return RC_READ_NON_EXISTING_PAGE.
* INPUTS :
*       PARAMETERS:
*           int               pageNum               Page Number / Block numberOfPages
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_NOT_FOUND          (9)File not found error.
*                    RC_READ_NON_EXISTING_PAGE  (4)Reading a page that doesnt exist.
*                    RC_BLOCK_READ_ERROR        (6)Error reading a block in memory.
*                    RC_OK                      (0)Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  Check to see the totalNumPages is greater than pageNum, if it isn't then return RC_READ_NON_EXISTING_PAGE.
*                   [3]  Then we calculate offset to seek by, using the pageNum and then we read that page.
*                   [4]  We check to see if read returns the proper blocksize read. If not we return  RC_BLOCK_READ_ERROR.
*                   [5]  If no error is encountered till here then we update the current Page Position of the file.
*                   [6]  We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Sukrit Gulati
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Sukrit Gulati      Original code
*/
/* reading blocks from disc */
RC
readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  if (fHandle->totalNumPages < pageNum)
  {
    return RC_READ_NON_EXISTING_PAGE;
  }
  int offset = (pageNum) * PAGE_SIZE;
  //set offset to non-existing to read the next block
  fseek(fstream, offset, SEEK_SET);
  size_t block_file_size = fread(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_BLOCK_READ_ERROR;
  }
  fHandle->curPagePos = pageNum;
  return RC_OK;
}

/*******************************************************************
* NAME :              getBlockPos (SM_FileHandle *fHandle)
*
* DESCRIPTION :       This method Return's the current page position in a file.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*
* OUTPUTS :
*
*       RETURN :
*            Type:   int                         Error code:
*            Values: curPagePos                  The current Page/Block Position.
*
* PROCESS :
*                   [1]  Check to see if fHandle is initialized, if it isn't then we retun RC_FILE_HANDLE_NOT_INIT.
*                   [2]  If it is initialized then we return the fhandle->curPagePos .
*
* AUTHOR :
* DATE    NAME
* 20Sep16 Sukrit Gulati
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Sukrit Gulati      Original code
*/
int
getBlockPos (SM_FileHandle *fHandle)
{
  if (fHandle == NULL){
    return RC_FILE_HANDLE_NOT_INIT;
  }
  return fHandle->curPagePos;
}
/*******************************************************************
* NAME :              readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Read the first respective page in a file.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_NOT_FOUND          (9)File not found error.
*                    RC_BLOCK_READ_ERROR        (6)Error reading a block in memory.
*                    RC_OK                      (0)Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  Then we seek from the beginning. And we read the block of the file.
*                   [3]  We check to see if read returns the proper blocksize read. If not we return  RC_BLOCK_READ_ERROR.
*                   [4]  We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Sukrit Gulati
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Sukrit Gulati      Original code
*/
RC
readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  fseek(fstream, 0, SEEK_SET);
  size_t block_file_size = fread(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_BLOCK_READ_ERROR;
  }
  return RC_OK;
}


/*******************************************************************
* NAME :              readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Read the previous respective page in a file, from the current position.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_NOT_FOUND          (9)File not found error.
*                    RC_BLOCK_READ_ERROR        (6)Error reading a block in memory.
*                    RC_PREV_BLOCK_NOT_EXIST    (7)previous block does not exist.
*                    RC_OK                      (0)Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  We check if the current Page position of the file is 0, if it is then no previous block exists and we return RC_PREV_BLOCK_NOT_EXIST
*                   [3]  Then we calculate the offset by reducing the current page number, and we read the block.
*                   [4]  We check to see if read returns the proper blocksize read. If not we return  RC_BLOCK_READ_ERROR.
*                   [5]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Karthik Shivaram
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Karthik Shivaram   Original code
*/
RC
readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  if (fHandle->curPagePos == 0)
  {
    return RC_PREV_BLOCK_NOT_EXIST;
  }
  int offset = (fHandle->curPagePos - 1) * PAGE_SIZE;
  //set offset to start of the previous block.
  fseek(fstream, offset, SEEK_SET);
  size_t block_file_size = fread(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_BLOCK_READ_ERROR;
  }
  return RC_OK;
}


/*******************************************************************
* NAME :              readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Read the current page in a file, using the current position.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_NOT_FOUND          (9)File not found error.
*                    RC_BLOCK_READ_ERROR        (6)Error reading a block in memory.
*                    RC_OK                      (0)Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  Then we calculate the offset by using current page position, and we read the block.
*                   [3]  We check to see if read returns the proper blocksize read. If not we return  RC_BLOCK_READ_ERROR.
*                   [4]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Karthik Shivaram
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Karthik Shivaram   Original code
*/
RC
readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  int offset = (fHandle->curPagePos) * PAGE_SIZE;
  //set offset to start of the previous block.
  fseek(fstream, offset, SEEK_SET);
  size_t block_file_size = fread(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_BLOCK_READ_ERROR;
  }
  return RC_OK;
}

/*******************************************************************
* NAME :              readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Read the next page in a file, using the current position.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_NOT_FOUND          (9) File not found error.
*                    RC_NEXT_BLOCK_NOT_EXIST    (10)Next Block to read does not exist.
*                    RC_BLOCK_READ_ERROR        (6) Error reading a block in memory.
*                    RC_OK                      (0) Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  Then we check to see if next block exists , ie current block is not the last block. If false we return RC_NEXT_BLOCK_NOT_EXIST.
*                   [2]  Then we calculate the offset from current page position + 1, and we read the block.
*                   [3]  We check to see if read returns the proper blocksize read. If not we return  RC_BLOCK_READ_ERROR.
*                   [4]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Karthik Shivaram
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Karthik Shivaram   Original code
*/
RC
readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  if(fHandle->curPagePos + 1 > fHandle->totalNumPages){
    return RC_NEXT_BLOCK_NOT_EXIST ;
  }
  int offset = (fHandle->curPagePos + 1) * PAGE_SIZE;
  //set offset to start of the previous block.
  fseek(fstream, offset, SEEK_SET);
  size_t block_file_size = fread(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_BLOCK_READ_ERROR;
  }
  return RC_OK;
}

/*******************************************************************
* NAME :              readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Read the Last page in a file, using the current position.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                          Error code:
*            Values: RC_FILE_NOT_FOUND          (9) File not found error.
*                    RC_BLOCK_READ_ERROR        (6) Error reading a block in memory.
*                    RC_OK                      (0) Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  Then we seek to the beginning of the last block of the file, and we read the block.
*                   [3]  We check to see if read returns the proper blocksize read. If not we return  RC_BLOCK_READ_ERROR.
*                   [4]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Karthik Shivaram
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Karthik Shivaram   Original code
*/
RC
readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  //set offset to non-existing to read the next block
  fseek(fstream, -PAGE_SIZE, SEEK_END);
  size_t block_file_size = fread(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_BLOCK_READ_ERROR;
  }
  return RC_OK;
}

/*******************************************************************
* NAME :              writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Write a page to disk using an absolute position.
*
* INPUTS :
*       PARAMETERS:
*           int               pageNum               Page Number / Block number, Of Pages
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                         Error code:
*            Values: RC_FILE_NOT_FOUND          (9) File not found error.
*                    RC_WRITE_NON_EXISTING_PAGE (11)Error trying to write to a non existing page.
*                    RC_WRITE_FAILED            (3) Writing to File Failed.
*                    RC_OK                      (0) Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  If the pageNum is greater than totalNumPages then we try to write a page that does not exist, hence we return RC_WRITE_NON_EXISTING_PAGE
*                   [3]  We check to see if write returns the proper blocksize read. If not we return  RC_WRITE_FAILED.
*                   [4]  We update the curPagePos in the fHandle .
*                   [5]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Saurabh Tiwari
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Saurabh Tiwari   Original code
*/
/* writing blocks to a page file */
RC
writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  if (fHandle->totalNumPages < pageNum)
  {
    return RC_WRITE_NON_EXISTING_PAGE;
  }
  int offset = (pageNum) * PAGE_SIZE;
  //set offset to non-existing to read the next block
  fseek(fstream, offset, SEEK_SET);

  size_t block_file_size = fwrite(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_WRITE_FAILED;
  }
  fHandle->curPagePos = pageNum;
  return RC_OK;
}

/*******************************************************************
* NAME :              writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
*
* DESCRIPTION :       Write a page to disk using the current position.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           SM_PageHandle     memPage               A page handle is an pointer to an area in memory
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                         Error code:
*            Values: RC_FILE_NOT_FOUND          (9) File not found error.
*                    RC_WRITE_FAILED            (3) Writing to File Failed.
*                    RC_OK                      (0) Function successfully executed.
* PROCESS :
*                   [1]  Obtain file stream from mgmtInfo, check if the stream is null , if it is null return RC_FILE_NOT_FOUND.
*                   [2]  We update the offset using curPagePos. And then we write to that position.
*                   [3]  We check to see if write returns the proper blocksize read. If not we return  RC_WRITE_FAILED.
*                   [4]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Saurabh Tiwari
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Saurabh Tiwari   Original code
*/
/* writing blocks to a page file */
RC
writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  FILE *fstream = fHandle->mgmtInfo;
  if (fstream == NULL)
  {
    return RC_FILE_NOT_FOUND;
  }
  int offset = (fHandle->curPagePos) * PAGE_SIZE;
  //set offset to non-existing to read the next block
  fseek(fstream, offset, SEEK_SET);

  size_t block_file_size = fwrite(memPage, sizeof(char), PAGE_SIZE, fstream);
  if(block_file_size == 0)
  {
    return RC_WRITE_FAILED;
  }
  return RC_OK;
}

/*******************************************************************
* NAME :              appendEmptyBlock (SM_FileHandle *fHandle)
*
* DESCRIPTION :       Increase the number of pages in the file by one. The new last page will be filled with zero bytes.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                         Error code:
*            Values: RC_FILE_HANDLE_NOT_INIT    (2) File handle not initialized error.
*                    RC_BLOCK_APPEND_ERROR      (8) Error appending block to a file.
*                    RC_OK                      (0) Function successfully executed.
* PROCESS :
*                   [1]  we check if the fHandle has been initialized, if not we return RC_FILE_HANDLE_NOT_INIT.
*                   [2]  We calculate the offset length by using totalNumPages and incrementing it by 1.
*                   [3]  We truncate the file , to increase it by a block and append /0 bytes to this new appended block.
*                   [4]  If the truncate_status is not 0 (successful) then we return RC_BLOCK_APPEND_ERROR.
*                   [5]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Saurabh Tiwari
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Saurabh Tiwari   Original code
*/
/* writing blocks to a page file */
RC
appendEmptyBlock (SM_FileHandle *fHandle)
{
  if (fHandle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  FILE *filepointer;
  int i;
  filepointer = fopen(fHandle->fileName, "r+");
  off_t offset_length = (fHandle->totalNumPages + 1) * PAGE_SIZE;
  int truncate_status = ftruncate(fileno(filepointer), offset_length);
  fseek(filepointer, 0, SEEK_END);
  if (truncate_status != 0)
  {
    return RC_BLOCK_APPEND_ERROR;
  }
  for(i = 0; i < PAGE_SIZE; i++)
  {
   fwrite("\0",1, 1,filepointer);
   fseek(filepointer,0,SEEK_END);
  }

  fHandle->totalNumPages = fHandle->totalNumPages + 1;
  fHandle->mgmtInfo = filepointer;

  return RC_OK;

}
/*******************************************************************
* NAME :              ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
*
* DESCRIPTION :      If the file has less than numberOfPages pages then it increases the size of file to numberOfPages.
*
* INPUTS :
*       PARAMETERS:
*           SM_FileHandle     *fHandle              Structure used to store file Information
*           int                numberOfPages        Number of Pages to cross verify.
* OUTPUTS :
*
*       RETURN :
*            Type:   RC                         Error code:
*            Values: RC_FILE_HANDLE_NOT_INIT    (2) File handle not initialized error.
*                    RC_BLOCK_APPEND_ERROR      (8) Error appending block to a file.
*                    RC_OK                      (0) Function successfully executed.
* PROCESS :
*                   [1]  we check if the fHandle has been initialized, if not we return RC_FILE_HANDLE_NOT_INIT.
*                   [2]  We check to see if the totalNumPages of the file is less than numberOfPages, if it is we increase the file size to that of numberOfPages.
*                   [3]  If all methods are successful We Return RC_OK.
* AUTHOR :
* DATE    NAME
* 20Sep16 Saurabh Tiwari
*
* CHANGES:
* REF NO    DATE       WHO                DETAIL
*    1      20Sep16    Saurabh Tiwari   Original code
*/
/* writing blocks to a page file */
RC
ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
  if (fHandle == NULL)
  {
    return RC_FILE_HANDLE_NOT_INIT;
  }
  int totalNumPages = fHandle->totalNumPages;
    for (;fHandle->totalNumPages < numberOfPages;)
      appendEmptyBlock(fHandle);
  return RC_OK;
}
