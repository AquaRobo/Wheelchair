#include "../headers/SPIHandler.h"
#include <gtest/gtest.h>

using namespace std;

TEST(SPIHandlerTest, CanInitialize) {
    SPIHandler handler(8, 1);
    handler.initialize();
    EXPECT_NO_THROW(handler.initialize());
}

TEST(SPIHandlerTest, CanSend){
    SPIHandler handler(8, 1);
    handler.initialize();

    vector<uint8_t> data_to_send = {1, 2, 3, 4, 5};
    EXPECT_NO_THROW(handler.send(data_to_send));
}

TEST(SPIHandlerTest, CanReceive) {
    SPIHandler handler(8, 1);
    handler.initialize();
    EXPECT_NO_THROW(handler.receive());
    vector<uint8_t> received_data = handler.receive();
    EXPECT_TRUE(received_data.empty()); // Initially, no data should be received
}

TEST(SPIHandlerTest, CanClose) {
    SPIHandler handler(8, 1);
    handler.initialize();
    EXPECT_NO_THROW(handler.close());
}

int main(int argc, char **argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
    // TEST(SPIHandlerTest, CanSendAndReceive) {
    //     SPIHandler handler(8, 1);
    //     handler.initialize();
        
    //     std::vector<uint8_t> dataToSend = {1, 2, 3, 4, 5};
    //     handler.send(dataToSend);
        
    //     std::vector<uint8_t> receivedData = handler.receive();
        
    //     EXPECT_EQ(receivedData.size(), dataToSend.size());
    //     for (size_t i = 0; i < dataToSend.size(); i++) {
    //         EXPECT_EQ(receivedData[i], dataToSend[i]);
    //     }
    // }
    // TEST(SPIHandlerTest, CanHandleEmptyData) {
    //     SPIHandler handler(8, 1);
    //     handler.initialize();
        
    //     std::vector<uint8_t> emptyData;
    //     handler.send(emptyData);
        
    //     std::vector<uint8_t> receivedData = handler.receive();
        
    //     EXPECT_TRUE(receivedData.empty());
    // }
    // TEST(SPIHandlerTest, CanHandleLargeData) {
    //     SPIHandler handler(8, 1);
    //     handler.initialize();
        
    //     std::vector<uint8_t> largeData(16, 1); // 16 bytes of data
    //     handler.send(largeData);
        
    //     std::vector<uint8_t> receivedData = handler.receive();
        
    //     EXPECT_EQ(receivedData.size(), largeData.size());
    //     for (size_t i = 0; i < largeData.size(); i++) {
    //         EXPECT_EQ(receivedData[i], largeData[i]);
    //     }
    // }