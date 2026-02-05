#include "../headers/SPIHandler.h"
#include <exception>
#include <iostream>

class SPIHandlerTest{
    private:
        SPIHandler handler;
        std::vector<uint8_t> imu_data;

    public:
        // Use a member initializer list to construct handler with arguments
        SPIHandlerTest() : handler(12, 1){
            try{
                handler.initialize();
            }
            catch (const std::exception &e){
                std::cerr << "Initialization failed: " << e.what() << std::endl;
            }
        }

        void TestSendData(){
            try{
                float roll = 5.5, pitch = 6.5, yaw = 7.5;
                float imu_values[3] = {roll, pitch, yaw};
                serializeFloatArray(imu_data, imu_values, 3);
                for (int i = 0; i < 3; ++i) {
                    std::cout << imu_values[i] << " ";
                }
                handler.send(imu_data);
                std::cout << "Data sent successfully." << std::endl;
            }
            catch (const std::exception &e){
                std::cerr << "Send failed: " << e.what() << std::endl;
            }
        }

        void serializeFloatArray(std::vector<uint8_t> &buffer, float *array, size_t size) {
            for (size_t i = 0; i < size; ++i) {
                uint8_t *p = reinterpret_cast<uint8_t *>(&array[i]);
                for (size_t j = 0; j < sizeof(float); ++j) {
                    buffer.push_back(p[j]);
                }
            }
        }

        void TestReceiveData(){
            try{
                std::vector<uint8_t> received_data = handler.receive();
                std::cout << "Data received successfully." << std::endl;
                for (const auto &byte : received_data){
                    std::cout << static_cast<int>(byte) << " ";
                }
                std::cout << std::endl;
            }
            catch (const std::exception &e){
                std::cerr << "Receive failed: " << e.what() << std::endl;
            }
        }
        void TestCloseHandler(){
            try{
                handler.close();
                std::cout << "Handler closed successfully." << std::endl;
            }
            catch (const std::exception &e){
                std::cerr << "Close failed: " << e.what() << std::endl;
            }
        }
};

int main(int argc, char **argv){
    SPIHandlerTest test;
    test.TestSendData();
    // test.TestReceiveData();
    // test.TestCloseHandler();
    return 0;
}