TEST_STORAGE_MGR=test_assign1_1
TEST_STORAGE_MGR_EXTRA=test_assign1_2
TEST_BUFFER_MGR=test_assign2_1
TEST_RCORD_MGR=test_assign3_1
CC=gcc -g

all: ${TEST_STORAGE_MGR} ${TEST_STORAGE_MGR_EXTRA} ${TEST_BUFFER_MGR} ${TEST_RCORD_MGR}

${TEST_STORAGE_MGR}: dberror.o storage_mgr.o test_assign1_1.o
	${CC} test_assign1_1.o storage_mgr.o dberror.o -g -Wall -lm -o test_assign1_1

${TEST_STORAGE_MGR_EXTRA}: dberror.o storage_mgr.o test_assign1_2.o
	${CC} test_assign1_2.o storage_mgr.o dberror.o -g -Wall -lm -o test_assign1_2

${TEST_BUFFER_MGR}: dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o caching_mechanism.o test_assign2_1.o
	${CC} test_assign2_1.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o dberror.o caching_mechanism.o -g -Wall -lm -o test_assign2_1

${TEST_RCORD_MGR}: dberror.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o caching_mechanism.o object_parser.o record_mgr.o rm_serializer.o expr.o test_assign3_1.o
	${CC} test_assign3_1.o storage_mgr.o buffer_mgr_stat.o buffer_mgr.o object_parser.o dberror.o caching_mechanism.o record_mgr.o rm_serializer.o expr.o -g -Wall -lm -o test_assign3_1



dberror.o: dberror.c dberror.h
	${CC} -c dberror.c -o dberror.o

storage_mgr.o: storage_mgr.c storage_mgr.h
	${CC} -c storage_mgr.c -o storage_mgr.o

test_assign1_1.o: test_assign1_1.c test_helper.h
	${CC} -c test_assign1_1.c -o test_assign1_1.o

test_assign1_2.o: test_assign1_2.c test_helper.h
	${CC} -c test_assign1_2.c -o test_assign1_2.o

buffer_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h
	${CC} -c buffer_mgr_stat.c -o buffer_mgr_stat.o

buffer_mgr.o: buffer_mgr.c buffer_mgr.h
	${CC} -c buffer_mgr.c -o buffer_mgr.o

caching_mechanism.o: caching_mechanism.c caching_mechanism.h
	${CC} -c caching_mechanism.c -o caching_mechanism.o

record_mgr.o: record_mgr.c record_mgr.h
	${CC} -c record_mgr.c -o record_mgr.o

rm_serializer.o: rm_serializer.c
	${CC} -c rm_serializer.c -o rm_serializer.o

expr.o: expr.c expr.h
	${CC} -c expr.c -o expr.o

object_parser.o: object_parser.c object_parser.h
	${CC} -c object_parser.c -o object_parser.o


test_assign2_1.o: test_assign2_1.c test_helper.h
	${CC} -c test_assign2_1.c -o test_assign2_1.o

test_assign3_1.o: test_assign3_1.c test_helper.h test_expr.c
	${CC} -c test_assign3_1.c -o test_assign3_1.o

clean:
	rm -f *.o
	rm -f test_assign1_1
	rm -f test_assign1_2
	rm -f test_assign2_1
	rm -f test_assign3_1
	rm -f testbuffer.bin
	rm -f test_table_t
	rm -f test_table_r

