#define _CRT_SECURE_NO_WARNINGS

#include "../pch.h"

#include "../MockClasses/MockWrapper.h"

using namespace testing;

class MockCandle : public Candle {
public:
    MockCandle(TickerId reqId) : Candle(reqId, 0, 0.0, 0.0, 0.0, 0.0, 0, 0.0, 0) {}
};

TEST(MockCandleBufferTest, bufferInitialize) {
    MockCandleBuffer cb{ 10 };
    EXPECT_EQ(cb.getCapacity(), 10);
    EXPECT_EQ(cb.checkBufferFull(), false);
}

TEST(MockCandleBufferTest, bufferCapacity) {
    MockCandleBuffer cb{ 10 };
    EXPECT_EQ(cb.checkBufferFull(), false);
    for (size_t i = 0; i < 10; i++) {
        auto candle = std::make_unique<MockCandle>(i);
        cb.updateBuffer(std::move(candle));
    }

    EXPECT_EQ(cb.checkBufferFull(), true);

    cb.setNewBufferCapacity(15);
    EXPECT_EQ(cb.checkBufferFull(), false);

    while (!cb.checkBufferFull()) {
        continue;
    }

    EXPECT_EQ(cb.checkBufferFull(), true);
    EXPECT_EQ(cb.getCapacity(), 10);
}

TEST(MockCandleBufferTest, bufferProcess) {
    MockCandleBuffer cb{ 0 };
    std::vector<std::unique_ptr<Candle>> buf = cb.processBuffer();
    EXPECT_EQ(buf.size(), 0);
    
    cb.setNewBufferCapacity(10);
    for (size_t i = 0; i < 10; i++) {
        auto candle = std::make_unique<MockCandle>(i);
        cb.updateBuffer(std::move(candle));
    }

    buf = cb.processBuffer();
    EXPECT_EQ(buf.size(), 10);
}



