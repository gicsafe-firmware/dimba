/**
 *  \file       test_cirBuffer.c
 *  \brief      Unit test for circular buffer.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.05.15  LeFr  v1.0.00  ---
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include <string.h>
#include "unity_fixture.h"
#include "CirBuffer.h"
#include "Mock_rkhport.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
int GlobalExpectCount;
int GlobalVerifyOrder;
char *GlobalOrderError;

TEST_GROUP(cirBuffer);

/* ---------------------------- Local variables ---------------------------- */
static CirBuffer buf;
static unsigned char storage[128];
static unsigned char block[32];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_SETUP(cirBuffer)
{
    Mock_rkhport_Init();
    rkh_enter_critical_Ignore();
    rkh_exit_critical_Ignore();
    memset(block, 0xaa, sizeof(block));
}

TEST_TEAR_DOWN(cirBuffer)
{
    Mock_rkhport_Verify();
    Mock_rkhport_Destroy();
}

TEST(cirBuffer, ClearAfterInit)
{
    int result;

    result = cirBuffer_init(&buf, storage, sizeof(char), 128);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL_INT(0, buf.qty);
    TEST_ASSERT_EQUAL_INT(1, buf.elemSize);
    TEST_ASSERT_EQUAL_INT(128, buf.numElem);
    TEST_ASSERT_EQUAL(storage, buf.sto);
    TEST_ASSERT_EQUAL(storage, buf.in);
    TEST_ASSERT_EQUAL(storage, buf.out);
    TEST_ASSERT_EQUAL(&storage[128], buf.end);
}

TEST(cirBuffer, WrongArgsOnInit)
{
    int result;

    result = cirBuffer_init((CirBuffer *)0, storage, sizeof(char), 128);
    TEST_ASSERT_EQUAL(1, result);

    result = cirBuffer_init(&buf, (unsigned char *)0, sizeof(char), 128);
    TEST_ASSERT_EQUAL(1, result);

    result = cirBuffer_init(&buf, storage, 0, 128);
    TEST_ASSERT_EQUAL(1, result);

    result = cirBuffer_init(&buf, storage, sizeof(char), 0);
    TEST_ASSERT_EQUAL(1, result);
}

TEST(cirBuffer, PutOneElement)
{
    int result;
    unsigned char elem;

    elem = 55;
    cirBuffer_init(&buf, storage, sizeof(char), 128);

    result = cirBuffer_put(&buf, &elem);
    TEST_ASSERT_EQUAL(elem, storage[0]);
    TEST_ASSERT_EQUAL(0, storage[1]);
    TEST_ASSERT_EQUAL(1, buf.qty);
    TEST_ASSERT_EQUAL(0, result);
}

TEST(cirBuffer, PutOneElementWrapAround)
{
    unsigned char elem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);

    elem = 55;
    cirBuffer_put(&buf, &elem);
    cirBuffer_put(&buf, &elem);
    cirBuffer_put(&buf, &elem);
    cirBuffer_put(&buf, &elem);
    elem = 99;
    cirBuffer_put(&buf, &elem);

    TEST_ASSERT_EQUAL(99, storage[0]);
    TEST_ASSERT_EQUAL(55, storage[1]);
    TEST_ASSERT_EQUAL(55, storage[2]);
    TEST_ASSERT_EQUAL(55, storage[3]);
    TEST_ASSERT_EQUAL(4, buf.qty);
}

TEST(cirBuffer, GetOneElement)
{
    int result;
    unsigned char expectedElem;
    unsigned char elem;

    expectedElem = 88;
    cirBuffer_init(&buf, storage, sizeof(char), 4);
    cirBuffer_put(&buf, &expectedElem);

    result = cirBuffer_get(&buf, &elem);

    TEST_ASSERT_EQUAL(0, result);
    TEST_ASSERT_EQUAL(expectedElem, elem);
}

TEST(cirBuffer, GetFromEmpty)
{
    int result;
    unsigned char elem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    result = cirBuffer_get(&buf, &elem);

    TEST_ASSERT_EQUAL(1, result);
}

TEST(cirBuffer, GetInOrderWrapAround)
{
    int result;
    unsigned char expectedElem;
    unsigned char elem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    expectedElem = 1;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 2;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 3;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 4;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 5;
    cirBuffer_put(&buf, &expectedElem);

    cirBuffer_get(&buf, &elem);
    TEST_ASSERT_EQUAL(2, elem);

    cirBuffer_get(&buf, &elem);
    TEST_ASSERT_EQUAL(3, elem);

    cirBuffer_get(&buf, &elem);
    TEST_ASSERT_EQUAL(4, elem);

    cirBuffer_get(&buf, &elem);
    TEST_ASSERT_EQUAL(5, elem);

    result = cirBuffer_get(&buf, &elem);
    TEST_ASSERT_EQUAL(1, result);
}

TEST(cirBuffer, GetManyElemsLessThanStored)
{
    int nGetElem;
    unsigned char expectedElem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    expectedElem = 1;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 2;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 3;
    cirBuffer_put(&buf, &expectedElem);

    nGetElem = cirBuffer_getBlock(&buf, block, 2);
    TEST_ASSERT_EQUAL(2, nGetElem);
    TEST_ASSERT_EQUAL(1, block[0]);
    TEST_ASSERT_EQUAL(2, block[1]);
    TEST_ASSERT_EQUAL(0xaa, block[2]);
}

TEST(cirBuffer, GetManyElemsEqualThanStored)
{
    int nGetElem;
    unsigned char expectedElem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    expectedElem = 1;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 2;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 3;
    cirBuffer_put(&buf, &expectedElem);

    nGetElem = cirBuffer_getBlock(&buf, block, 3);
    TEST_ASSERT_EQUAL(3, nGetElem);
    TEST_ASSERT_EQUAL(1, block[0]);
    TEST_ASSERT_EQUAL(2, block[1]);
    TEST_ASSERT_EQUAL(3, block[2]);
    TEST_ASSERT_EQUAL(0xaa, block[3]);
}

TEST(cirBuffer, GetManyElemsMoreThanStored)
{
    int nGetElem;
    unsigned char expectedElem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    expectedElem = 1;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 2;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 3;
    cirBuffer_put(&buf, &expectedElem);

    nGetElem = cirBuffer_getBlock(&buf, block, 4);
    TEST_ASSERT_EQUAL(3, nGetElem);
    TEST_ASSERT_EQUAL(1, block[0]);
    TEST_ASSERT_EQUAL(2, block[1]);
    TEST_ASSERT_EQUAL(3, block[2]);
    TEST_ASSERT_EQUAL(0xaa, block[3]);
}

TEST(cirBuffer, GetManyElemsEqualThanStoredWrapAround)
{
    int nGetElem, i, blockSize;
    unsigned char expectedElem;

    blockSize = 4;
    cirBuffer_init(&buf, storage, sizeof(char), blockSize);
    for (i = 0; i < (blockSize + 2); ++i)
    {
        expectedElem = (unsigned char)i;
        cirBuffer_put(&buf, &expectedElem);
    }

    nGetElem = cirBuffer_getBlock(&buf, &block[1], blockSize);
    TEST_ASSERT_EQUAL(4, nGetElem);
    TEST_ASSERT_EQUAL(0xaa, block[0]);
    TEST_ASSERT_EQUAL(2, block[1]);
    TEST_ASSERT_EQUAL(3, block[2]);
    TEST_ASSERT_EQUAL(4, block[3]);
    TEST_ASSERT_EQUAL(5, block[4]);
    TEST_ASSERT_EQUAL(0xaa, block[5]);
}

TEST(cirBuffer, GetManyElemsLessThanStoredWrapAround)
{
    int nGetElem, i, blockSize;
    unsigned char expectedElem;

    blockSize = 4;
    cirBuffer_init(&buf, storage, sizeof(char), blockSize);
    for (i = 0; i < (blockSize + 2); ++i)
    {
        expectedElem = (unsigned char)i;
        cirBuffer_put(&buf, &expectedElem);
    }

    nGetElem = cirBuffer_getNumElem(&buf);
    TEST_ASSERT_EQUAL(blockSize, nGetElem);

    nGetElem = cirBuffer_getBlock(&buf, &block[1], blockSize - 1);
    TEST_ASSERT_EQUAL(3, nGetElem);     /* Test number of get elements */
    TEST_ASSERT_EQUAL(0xaa, block[0]);  /* Test correct write of block[] */
    TEST_ASSERT_EQUAL(2, block[1]);     /* Test get elements ... */
    TEST_ASSERT_EQUAL(3, block[2]);
    TEST_ASSERT_EQUAL(4, block[3]);
    TEST_ASSERT_EQUAL(0xaa, block[4]);  /* Test correct write of block[] */
                                        /* Test remainded elements in buf */
    TEST_ASSERT_EQUAL(blockSize - nGetElem, cirBuffer_getNumElem(&buf));
}

TEST(cirBuffer, GetManyElemsMoreThanStoredWrapAround)
{
    int nGetElem, i, blockSize;
    unsigned char expectedElem;

    blockSize = 4;
    cirBuffer_init(&buf, storage, sizeof(char), blockSize);
    for (i = 0; i < (blockSize + 2); ++i)
    {
        expectedElem = (unsigned char)i;
        cirBuffer_put(&buf, &expectedElem);
    }

    nGetElem = cirBuffer_getBlock(&buf, block, blockSize + 1);
    TEST_ASSERT_EQUAL(4, nGetElem);
    TEST_ASSERT_EQUAL(2, block[0]);
    TEST_ASSERT_EQUAL(3, block[1]);
    TEST_ASSERT_EQUAL(4, block[2]);
    TEST_ASSERT_EQUAL(5, block[3]);
    TEST_ASSERT_EQUAL(0xaa, block[4]);
}

TEST(cirBuffer, GetManyFromEmpty)
{
    int nGetElem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    nGetElem = cirBuffer_getBlock(&buf, block, 4);

    TEST_ASSERT_EQUAL(0, nGetElem);
}

/* ------------------------------ End of file ------------------------------ */
