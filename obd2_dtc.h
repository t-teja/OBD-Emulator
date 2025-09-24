#ifndef __OBD2_DTC_H__
#define __OBD2_DTC_H__

#include <stdint.h>
#include <stdbool.h>

// Maximum number of DTCs that can be stored
#define MAX_STORED_DTCS     10

// DTC status bits
#define DTC_STATUS_TEST_FAILED              0x01
#define DTC_STATUS_TEST_FAILED_THIS_CYCLE   0x02
#define DTC_STATUS_PENDING                  0x04
#define DTC_STATUS_CONFIRMED                0x08 
#define DTC_STATUS_TEST_NOT_COMPLETED_SINCE_CLEAR 0x10
#define DTC_STATUS_TEST_FAILED_SINCE_CLEAR  0x20
#define DTC_STATUS_TEST_NOT_COMPLETED_THIS_CYCLE 0x40
#define DTC_STATUS_WARNING_INDICATOR_REQUESTED 0x80

// DTC types (first character)
#define DTC_TYPE_POWERTRAIN     'P'
#define DTC_TYPE_CHASSIS        'C'
#define DTC_TYPE_BODY           'B'
#define DTC_TYPE_NETWORK        'U'

// Common DTC codes
#define DTC_P0000   0x0000  // No DTCs
#define DTC_P0100   0x0100  // Mass Air Flow Circuit Malfunction
#define DTC_P0101   0x0101  // Mass Air Flow Circuit Range/Performance Problem
#define DTC_P0102   0x0102  // Mass Air Flow Circuit Low Input
#define DTC_P0103   0x0103  // Mass Air Flow Circuit High Input
#define DTC_P0110   0x0110  // Intake Air Temperature Circuit Malfunction
#define DTC_P0115   0x0115  // Engine Coolant Temperature Circuit Malfunction
#define DTC_P0120   0x0120  // Throttle Position Sensor Circuit Malfunction
#define DTC_P0125   0x0125  // Insufficient Coolant Temperature for Closed Loop Fuel Control
#define DTC_P0130   0x0130  // O2 Sensor Circuit Malfunction (Bank 1 Sensor 1)
#define DTC_P0135   0x0135  // O2 Sensor Heater Circuit Malfunction (Bank 1 Sensor 1)
#define DTC_P0171   0x0171  // System Too Lean (Bank 1)
#define DTC_P0172   0x0172  // System Too Rich (Bank 1)
#define DTC_P0300   0x0300  // Random/Multiple Cylinder Misfire Detected
#define DTC_P0301   0x0301  // Cylinder 1 Misfire Detected
#define DTC_P0302   0x0302  // Cylinder 2 Misfire Detected
#define DTC_P0303   0x0303  // Cylinder 3 Misfire Detected
#define DTC_P0304   0x0304  // Cylinder 4 Misfire Detected
#define DTC_P0420   0x0420  // Catalyst System Efficiency Below Threshold (Bank 1)
#define DTC_P0430   0x0430  // Catalyst System Efficiency Below Threshold (Bank 2)
#define DTC_P0500   0x0500  // Vehicle Speed Sensor Malfunction
#define DTC_P0505   0x0505  // Idle Control System Malfunction
#define DTC_P0510   0x0510  // Closed Throttle Position Switch Malfunction

// DTC structure
typedef struct {
    uint16_t code;          // DTC code (without type prefix)
    uint8_t status;         // DTC status byte
    uint8_t type;           // DTC type (P, C, B, U)
    bool active;            // Is this DTC slot active
    uint32_t timestamp;     // When the DTC was set
} dtc_entry_t;

// DTC manager structure
typedef struct {
    dtc_entry_t dtcs[MAX_STORED_DTCS];
    uint8_t count;          // Number of active DTCs
    bool mil_status;        // Malfunction Indicator Lamp status
    uint32_t clear_timestamp; // When DTCs were last cleared
} dtc_manager_t;

// Function prototypes
void obd2_dtc_init(void);
bool obd2_dtc_add(uint16_t code, uint8_t type, uint8_t status);
bool obd2_dtc_remove(uint16_t code, uint8_t type);
void obd2_dtc_clear_all(void);
uint8_t obd2_dtc_get_count(void);
bool obd2_dtc_get_mil_status(void);
void obd2_dtc_set_mil_status(bool status);

// Get DTCs for different services
uint8_t obd2_dtc_get_stored(uint8_t *buffer, uint8_t max_size);
uint8_t obd2_dtc_get_pending(uint8_t *buffer, uint8_t max_size);
uint8_t obd2_dtc_get_permanent(uint8_t *buffer, uint8_t max_size);

// DTC status and management
void obd2_dtc_update_status(uint16_t code, uint8_t type, uint8_t status);
bool obd2_dtc_exists(uint16_t code, uint8_t type);
uint8_t obd2_dtc_get_status(uint16_t code, uint8_t type);

// Utility functions
void obd2_dtc_format_code_string(uint16_t code, uint8_t type, char *str);
uint16_t obd2_dtc_parse_code_string(const char *str, uint8_t *type);
uint16_t obd2_dtc_format_for_transmission(uint16_t code, uint8_t type);
void obd2_dtc_print_all(void);

// Test functions for simulation
void obd2_dtc_simulate_fault(uint16_t code, uint8_t type);
void obd2_dtc_simulate_random_faults(void);
void obd2_dtc_test_scenario(void);

// Advanced DTC simulation functions
void obd2_dtc_simulate_realistic_faults(void);
void obd2_dtc_simulate_cold_start_issues(void);
void obd2_dtc_simulate_emissions_failure(void);
void obd2_dtc_simulate_fuel_system_issues(void);
void obd2_dtc_simulate_ignition_misfires(void);

#endif // __OBD2_DTC_H__
