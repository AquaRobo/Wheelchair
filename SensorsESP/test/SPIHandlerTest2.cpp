#include "../headers/SPIHandler.h"
#include <exception>
#include <iostream>

class SPIHandlerTest{
    private:
        SPIHandler handler;

    public:
        // Use a member initializer list to construct handler with arguments
        SPIHandlerTest() : handler(8, 1){
            try{
                handler.initialize();
            }
            catch (const std::exception &e){
                std::cerr << "Initialization failed: " << e.what() << std::endl;
            }
        }

        void TestSendData(){
            try{
                std::vector<uint8_t> data_to_send = {1, 2, 3, 4, 5};
                handler.send(data_to_send);
                std::cout << "Data sent successfully." << std::endl;
            }
            catch (const std::exception &e){
                std::cerr << "Send failed: " << e.what() << std::endl;
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