Recode Manager Assignment
--------------------------------------------------------------------------------------------------------------------

Group Members:
--------------
Saurabh Tiwari
Karthik Shivaram
Sukrit Gulati
Seungho Han
--------------------------------------------------------------------------------------------------------------------

List of Files for the Assignment
---------------------------------
Makefile
buffer_mgr.c
buffer_mgr.h
buffer_mgr_stat.c
buffer_mgr_stat.h
caching_mechanism.c
caching_mechanism.h
dt.h
dberror.c
dberror.h
storage_mgr.c
storage_mgr.h
expr.c
expr.h
object_parser.c
object_parser.h
record_mgr.c
record_mgr.h
rm_serializer.c
Deserialize.c
tables.h
test_assign1_1.c
test_assign1_2.c
test_assign2_1.c
test_assign3_1.c
test_expr.c
test_helper.h
Note: There is an additional test file for this assignment.
------------------------------------------------------------------------------------------------------------------

Changes done in each method:
----------------------------
NAME : 	createTable (char *name, Schema *schema)

[1]  Takes a Schema information as arguments.
[2]  Checks file accessibility then creates a page file and opens the page file.
[3]  Throws errors according to the results from access, createPageFile and openPageFile.
[3]  Gets serialized schema.
[4]  Writes serialized schema into block.
[5]  Throws error when writeBlock is failed.


NAME : openTable (RM_TableData *rel, char *name)

[1]  Takes an object of RM_TableData and file name.
[2]  Opens a file with file name and create a read pointer to read the file.
[3]  Makes an object of BM_BufferPool and store into table management data.
[3]  Initializes buffer pool and makes page pinnded.
[4]  Set page number into 0.
[5]  Executes parser to make string (data) to schema and stores parsed data into schema of relation.


NAME : closeTable (RM_TableData *rel)

[1]  Takes an object of RM_TableData and makes page unpinned.
[2]  Shuts down buffer pool.
[3]  Returns result code.


NAME : deleteTable (char *name)

[1]  Destroys table with table name and return result code.


NAME : getNumTuples (RM_TableData *rel)

[1]  Takes a RM_TablData object.
[2]  Gets record and counts number of tuple.
[3]  At the same time, increases number of pages of Record ID.


NAME : insertRecord (RM_TableData *rel, Record *record)

[1]  Creates a temporary record ID and a free page at the end of the temporary page.
[2]  Stores free page into table management data and record.
[3]  Execute serializer with record and schema.
[4]  Pins page with buffer pool, temporary page and number of free page.
[5]  Initializes the record in page to \0.
[6]  Writes the serialized record to the page struct.
[7]  Throws errors if error occurs.


NAME : deleteRecord (RM_TableData *rel, RID id)

[1]  Create memory space to store tomb stone flagged record.
[2]  Pins page and concatenates tomb stone flag and data (string) to be deleted.
[3]  Updates page id then initializes the record in page to \0.
[4]  Throws errors if error occurs


NAME : updateRecord (RM_TableData *rel, Record *record)

[1]  Gets relation and record.
[2]  Serializes record, pin page writes serialized record into page handle.
[3]  Makes page dirty, unpins page and forces page.
[4]  Free serialized record and page handle.


NAME : getRecord (RM_TableData *rel, RID id, Record *record)

[1]  Pins page and stores data of page handle into temporary data.
[2]  Sets the record ID and the data read from page file after deserializing it.
[3]  Throws error when there are no more tuples.


NAME : startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)

[1]  Takes relation, scan handle and condition.
[2]  Stores above arguments into scan management data.


NAME : next (RM_ScanHandle *scan, Record *record)

[1]  Initializes the RID from the scan management data.
[2]  Initializes condition with expression from scan management data.
[3]  If condition is NULL then get record without evaluating expression.
[4]  If condition has expression, then get record and evaluate expression.
     Stores record when result of condition from evalExpr is true.
[5]  Set redord ID to 1 and return no more tuples.


NAME : closeScan (RM_ScanHandle *scan)

[1]  Gets scan management data.
[2]  Free attributes of scan management struct.

NAME : getRecordSize (Schema *schema)

[1]  Gets record size by using getRecordSizeOffset.
[2]  Return record size.

NAME : *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)

[1]  Creates a schema.
[2]  Initializes number of attribute, attribute names, data types, length, key size and keys.
[3]  Return schema.

NAME : freeSchema (Schema *schema)

[1]  Gets a schema.
[2]  Deallocated the schema.

NAME : freeRecord (Record *record)

[1]  Gets a record.
[2]  Deallocated the record.

NAME : getAttr (Record *record, Schema *schema, int attrNum, Value **value)

[1]  Gets offset of record and calculate the capacity to add value.
[2]  Copies the attribute to value according to the data types.
[3]  Returns unknown data type when the datatype is invalid.

NAME : setAttr (Record *record, Schema *schema, int attrNum, Value *value)

[1]  Gets offset of record and calculate the capacity to store value.
[2]  Copies the value to attribute according to the data types.
[3]  Returns unknown data type when the datatype is invalid.

NAME : getRecordSizeOffset(Schema *schema, int attrNum)
[1]  Gets a schema and number of attributes.
[2]  Adds offset as much as the size of data type traversing all schema.
[3]  Returns offset.
-------------------------------------------------------------------------------------------------------------

Execution Instructions:
-----------------------
There is a Makefile in this assignment, make use of that to compile and run.
Step 1: unzip the file.
Step 2: go in the directory from the commandline.
Step 3: Make sure you have the standard GCC compiler installed on your system. Fourier server is a nice way to test.
Step 4: run $ make clean; make
Step 5: run $ ./test_assign3_1 to run the assignment1 test
——————————————————————————————————————————————————————————————————————————————————————————————————————————————
