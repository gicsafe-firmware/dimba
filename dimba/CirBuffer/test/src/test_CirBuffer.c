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
#include "unity_fixture.h"
#include "CirBuffer.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
TEST_GROUP(cirBuffer);

/* ---------------------------- Local variables ---------------------------- */
static CirBuffer buf;
static unsigned char storage[128];
static unsigned char block[8];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
/* ---------------------------- Global functions --------------------------- */
TEST_SETUP(cirBuffer)
{
}

TEST_TEAR_DOWN(cirBuffer)
{
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
}

TEST(cirBuffer, GetManyElemsEqualThanStoredWrapAround)
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
    expectedElem = 4;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 5;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 6;
    cirBuffer_put(&buf, &expectedElem);
    cirBuffer_get(&buf, &expectedElem);
    cirBuffer_get(&buf, &expectedElem);

    nGetElem = cirBuffer_getBlock(&buf, block, 4);
    TEST_ASSERT_EQUAL(2, nGetElem);
    TEST_ASSERT_EQUAL(5, block[0]);
    TEST_ASSERT_EQUAL(6, block[1]);
}

TEST(cirBuffer, GetManyElemsInOrderWrapAround)
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
    expectedElem = 4;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 5;
    cirBuffer_put(&buf, &expectedElem);
    expectedElem = 6;
    cirBuffer_put(&buf, &expectedElem);

    nGetElem = cirBuffer_getBlock(&buf, block, 4);
    TEST_ASSERT_EQUAL(4, nGetElem);
    TEST_ASSERT_EQUAL(3, block[0]);
    TEST_ASSERT_EQUAL(4, block[1]);
    TEST_ASSERT_EQUAL(5, block[2]);
    TEST_ASSERT_EQUAL(6, block[3]);
}

TEST(cirBuffer, GetManyFromEmpty)
{
    int nGetElem;

    cirBuffer_init(&buf, storage, sizeof(char), 4);
    nGetElem = cirBuffer_getBlock(&buf, block, 4);

    TEST_ASSERT_EQUAL(0, nGetElem);
}

/* ------------------------------ End of file ------------------------------ */
