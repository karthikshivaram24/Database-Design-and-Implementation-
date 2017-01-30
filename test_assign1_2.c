#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// test name
char *testName;

/* test output files */
#define TESTPF "test_pagefile.bin"

/* prototypes for test functions */
static void testAppendPageContent(void);

/* main function running all tests */
int
main (void)
{
  testName = "";

  initStorageManager();

  testAppendPageContent();

  return 0;
}



/* Try to create, open, and close a page file */
void
testAppendPageContent(void)
{
  SM_FileHandle fh;
  SM_PageHandle ph;
  int i;

  testName = "test append/readLastBlock";

  ph = (SM_PageHandle) malloc(PAGE_SIZE);

  // create a new page file
  TEST_CHECK(createPageFile (TESTPF));
  TEST_CHECK(openPageFile (TESTPF, &fh));
  printf("created and opened file\n");

   for (i=0; i < PAGE_SIZE; i++)
    ph[i] = (i % 10) + '0';

// writing on second block
  TEST_CHECK(writeBlock(1,&fh,ph));
  printf("writing on second block\n");

 // reading first block
 TEST_CHECK(readFirstBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "zero bytes as expected because we wrote on the second block");
  printf("reading first block\n");

   // reading last block
 TEST_CHECK(readLastBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "expected value as second block is the last block");
  printf("reading last block\n");

     // appending last block
 TEST_CHECK(appendEmptyBlock (&fh));
  printf("appending empty block\n");

    // reading last block
 TEST_CHECK(readLastBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++){
    ASSERT_TRUE((ph[i] == 0), "expected zero bytes as we appended empty block");
  }
  
  //increasing the block for capacity
 TEST_CHECK(ensureCapacity (3, &fh));
 printf("ensured capacity\n");

//check if the capacity has increased
  TEST_CHECK(readLastBlock (&fh, ph));
  for (i=0; i < PAGE_SIZE; i++){
    ASSERT_TRUE((ph[i] == 0), "expected zero bytes as we ensured capacity");
  }
  //destroy new page file*
 TEST_CHECK(destroyPageFile (TESTPF));

  TEST_DONE();
}
