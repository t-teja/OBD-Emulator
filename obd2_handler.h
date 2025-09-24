#ifndef __OBD2_HANDLER_H__
#define __OBD2_HANDLER_H__

#include <stdint.h>
#include <stdbool.h>

// Function prototypes for OBD2 handler

// Initialization and main processing
bool obd2_handler_init(void);
void obd2_handler_process(void);

// Message processing
bool obd2_process_request(uint8_t *can_data, uint8_t can_length);
bool obd2_send_response(uint8_t *can_data, uint8_t can_length);

// Statistics and diagnostics
void obd2_handler_stats(void);
bool obd2_handler_is_initialized(void);
uint32_t obd2_handler_get_message_count(void);
uint32_t obd2_handler_get_error_count(void);
void obd2_handler_reset_stats(void);

// Test and simulation functions
void obd2_handler_test_response(void);
void obd2_handler_simulate_request(uint8_t service, uint8_t pid);

// Vehicle simulation functions (from vehicle_data.c)
void obd2_init_vehicle_simulation(void);
void obd2_update_vehicle_simulation(void);
void obd2_set_engine_state(bool running);
bool obd2_get_engine_state(void);
uint32_t obd2_get_engine_runtime(void);

#endif // __OBD2_HANDLER_H__
