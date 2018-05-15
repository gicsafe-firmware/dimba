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

TEST(cirBuffer, PutOneElem)
{
    int result;
    char elem;

    elem = 'c';
    cirBuffer_init(&buf, storage, sizeof(char), 128);

    result = cirBuffer_put(&buf, &elem);
    TEST_ASSERT_EQUAL(elem, storage[0]);
    TEST_ASSERT_EQUAL(0, storage[1]);
    TEST_ASSERT_EQUAL(1, buf.qty);
    TEST_ASSERT_EQUAL(0, result);
}

/* ------------------------------ End of file ------------------------------ */
