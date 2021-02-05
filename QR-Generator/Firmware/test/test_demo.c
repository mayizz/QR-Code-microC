#include <stdint.h>
#include <unity.h>
#include <util/delay.h>
#include "fifo.h"

#include "unittest_transport.h"

void test_fifo(void)
{
    uint8_t buf[3];
    fifo_t f;
    fifo_init(&f, buf, sizeof(buf));
    fifo_put(&f, 0x11);
    fifo_put(&f, 0x22);
    fifo_put(&f, 0x33);

    TEST_ASSERT_EQUAL(0x11, fifo_get_nowait(&f));
    TEST_ASSERT_EQUAL(0x22, fifo_get_nowait(&f));
    TEST_ASSERT_EQUAL(0x33, fifo_get_nowait(&f));
}

int main()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    _delay_ms(2000);

    DDRB |= 0x2;
    PORTB &= ~0x2;

    UNITY_BEGIN(); // IMPORTANT LINE!
    RUN_TEST(test_fifo);
    UNITY_END(); // stop unit testing

    PORTB |= 0x2;
    DDRB |= 0x4;
    PORTB &= ~0x4;

    while (1)
        ;
}
