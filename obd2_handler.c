#include "obd2_handler.h"
#include "obd2_protocol.h"
#include "xl2515.h"
#include <stdio.h>
#include <string.h>

// OBD2 handler state
static struct {
    bool initialized;
    uint32_t messages_received;
    uint32_t messages_sent;
    uint32_t errors;
    uint8_t last_error_code;
} obd2_state = {
    .initialized = false,
    .messages_received = 0,
    .messages_sent = 0,
    .errors = 0,
    .last_error_code = 0
};

// Message buffers
static uint8_t rx_buffer[8];
static uint8_t tx_buffer[8];

bool obd2_handler_init(void)
{
    // Initialize CAN interface at 500 kbps (standard OBD2 speed)
    xl2515_init(KBPS500);
    
    // Initialize vehicle simulation
    obd2_init_vehicle_simulation();
    
    obd2_state.initialized = true;
    obd2_state.messages_received = 0;
    obd2_state.messages_sent = 0;
    obd2_state.errors = 0;
    
    printf("OBD2 Handler initialized - Ready to receive requests\r\n");
    return true;
}

void obd2_handler_process(void)
{
    if (!obd2_state.initialized) {
        return;
    }
    
    // Check for incoming CAN messages
    uint8_t rx_length = 0;
    uint32_t can_id = OBD2_REQUEST_ID;
    
    if (xl2515_recv(can_id, rx_buffer, &rx_length)) {
        obd2_state.messages_received++;
        
        printf("Received OBD2 request: ");
        for (int i = 0; i < rx_length; i++) {
            printf("%02X ", rx_buffer[i]);
        }
        printf("\r\n");
        
        // Process the OBD2 request
        if (obd2_process_request(rx_buffer, rx_length)) {
            printf("Request processed successfully\r\n");
        } else {
            printf("Error processing request\r\n");
            obd2_state.errors++;
        }
    }
    
    // Update vehicle simulation
    obd2_update_vehicle_simulation();
}

bool obd2_process_request(uint8_t *can_data, uint8_t can_length)
{
    obd2_message_t request;
    obd2_response_t response;
    
    // Parse the incoming OBD2 message
    if (!obd2_parse_message(can_data, can_length, &request)) {
        printf("Failed to parse OBD2 message\r\n");
        return false;
    }
    
    printf("Parsed request - Service: 0x%02X, PID: 0x%02X\r\n", 
           request.service, request.pid);
    
    // Create response based on the request
    if (!obd2_create_response(&request, &response)) {
        printf("Failed to create OBD2 response\r\n");
        return false;
    }
    
    // Format response into CAN message
    uint8_t tx_length = obd2_format_can_message(&response, tx_buffer);
    if (tx_length == 0) {
        printf("Failed to format CAN message\r\n");
        return false;
    }
    
    // Send response
    if (obd2_send_response(tx_buffer, tx_length)) {
        obd2_state.messages_sent++;
        
        printf("Sent OBD2 response: ");
        for (int i = 0; i < tx_length; i++) {
            printf("%02X ", tx_buffer[i]);
        }
        printf("\r\n");
        
        return true;
    } else {
        printf("Failed to send OBD2 response\r\n");
        return false;
    }
}

bool obd2_send_response(uint8_t *can_data, uint8_t can_length)
{
    // Send response with our ECU ID
    xl2515_send(OBD2_ECU_ID, can_data, can_length);
    return true;
}

void obd2_handler_stats(void)
{
    printf("\r\n=== OBD2 Handler Statistics ===\r\n");
    printf("Initialized: %s\r\n", obd2_state.initialized ? "Yes" : "No");
    printf("Messages Received: %lu\r\n", obd2_state.messages_received);
    printf("Messages Sent: %lu\r\n", obd2_state.messages_sent);
    printf("Errors: %lu\r\n", obd2_state.errors);
    printf("Last Error Code: 0x%02X\r\n", obd2_state.last_error_code);
    printf("Engine Running: %s\r\n", obd2_get_engine_state() ? "Yes" : "No");
    printf("Engine Runtime: %lu seconds\r\n", obd2_get_engine_runtime());
    printf("===============================\r\n\r\n");
}

// Test functions for debugging
void obd2_handler_test_response(void)
{
    printf("Testing OBD2 response generation...\r\n");
    
    // Test Service 01, PID 00 (Supported PIDs)
    obd2_message_t test_request = {
        .service = OBD2_SERVICE_01,
        .pid = OBD2_PID_SUPPORTED_01_20,
        .length = 2
    };
    
    obd2_response_t test_response;
    if (obd2_create_response(&test_request, &test_response)) {
        printf("Test response created successfully\r\n");
        printf("Service: 0x%02X, PID: 0x%02X, Length: %d\r\n",
               test_response.service, test_response.pid, test_response.length);
        
        uint8_t can_msg[8];
        uint8_t msg_len = obd2_format_can_message(&test_response, can_msg);
        
        printf("CAN message: ");
        for (int i = 0; i < msg_len; i++) {
            printf("%02X ", can_msg[i]);
        }
        printf("\r\n");
    } else {
        printf("Failed to create test response\r\n");
    }
}

void obd2_handler_simulate_request(uint8_t service, uint8_t pid)
{
    printf("Simulating OBD2 request - Service: 0x%02X, PID: 0x%02X\r\n", service, pid);
    
    // Create a simulated CAN message
    uint8_t sim_can_data[8] = {0x02, service, pid, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    if (obd2_process_request(sim_can_data, 8)) {
        printf("Simulated request processed successfully\r\n");
    } else {
        printf("Failed to process simulated request\r\n");
    }
}

// Diagnostic functions
bool obd2_handler_is_initialized(void)
{
    return obd2_state.initialized;
}

uint32_t obd2_handler_get_message_count(void)
{
    return obd2_state.messages_received;
}

uint32_t obd2_handler_get_error_count(void)
{
    return obd2_state.errors;
}

void obd2_handler_reset_stats(void)
{
    obd2_state.messages_received = 0;
    obd2_state.messages_sent = 0;
    obd2_state.errors = 0;
    obd2_state.last_error_code = 0;
    printf("OBD2 handler statistics reset\r\n");
}
