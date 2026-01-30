#include "../../headers/SPIHandler.h"
#include "../../services/SPIHandler.cpp"
#include <stdlib.h>

SPIHandler spi_handler(20, 1); // adjust the buffer size according to the size of the data being sent
std::vector<uint8_t> all_data;

void setup(){
    Serial.begin(115200);
    spi_handler.initialize();
    Serial.println("spi Handler initialized.");
}

void loop(){
    all_data.clear();
    all_data = spi_handler.receive();
    if (!all_data.empty()){
        // Convert bytes to string
        String received_message = "";
        for (size_t i = 0; i < all_data.size(); i++){
            received_message += (char)all_data[i];
        }
        
        Serial.print("Received message: ");
        Serial.println(received_message);
        
        // Parse the message format: "pwm_pin-dir_pin-current_pwm-dir_bit"
        int dash_positions[3];
        int dash_count = 0;
        
        // Find dash positions
        for (int i = 0; i < received_message.length() && dash_count < 3; i++) {
            if (received_message[i] == '-') {
                dash_positions[dash_count] = i;
                dash_count++;
            }
        }
        
        if (dash_count == 3) {
            // Extract values
            int pwm_pin = received_message.substring(0, dash_positions[0]).toInt();
            int dir_pin = received_message.substring(dash_positions[0] + 1, dash_positions[1]).toInt();
            int current_pwm = received_message.substring(dash_positions[1] + 1, dash_positions[2]).toInt();
            int dir_bit = received_message.substring(dash_positions[2] + 1).toInt();
            
            // Print parsed values
            Serial.println("Parsed values:");
            Serial.print("PWM Pin: "); Serial.println(pwm_pin);
            Serial.print("Dir Pin: "); Serial.println(dir_pin);
            Serial.print("Current PWM: "); Serial.println(current_pwm);
            Serial.print("Dir Bit: "); Serial.println(dir_bit);
        } else {
            Serial.println("Error: Invalid message format");
        }
        
        Serial.println();
    }
}