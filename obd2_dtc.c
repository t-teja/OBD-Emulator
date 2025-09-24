#include "obd2_dtc.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Global DTC manager
static dtc_manager_t dtc_manager;

void obd2_dtc_init(void)
{
    memset(&dtc_manager, 0, sizeof(dtc_manager_t));
    dtc_manager.count = 0;
    dtc_manager.mil_status = false;
    dtc_manager.clear_timestamp = to_ms_since_boot(get_absolute_time());
    
    printf("DTC manager initialized\r\n");
}

bool obd2_dtc_add(uint16_t code, uint8_t type, uint8_t status)
{
    // Check if DTC already exists
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (dtc_manager.dtcs[i].active && 
            dtc_manager.dtcs[i].code == code && 
            dtc_manager.dtcs[i].type == type) {
            // Update existing DTC status
            dtc_manager.dtcs[i].status |= status;
            return true;
        }
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (!dtc_manager.dtcs[i].active) {
            dtc_manager.dtcs[i].code = code;
            dtc_manager.dtcs[i].type = type;
            dtc_manager.dtcs[i].status = status;
            dtc_manager.dtcs[i].active = true;
            dtc_manager.dtcs[i].timestamp = to_ms_since_boot(get_absolute_time());
            dtc_manager.count++;
            
            // Set MIL if this is a confirmed DTC
            if (status & DTC_STATUS_CONFIRMED) {
                dtc_manager.mil_status = true;
            }
            
            printf("Added DTC: %c%04X with status 0x%02X\r\n", type, code, status);
            return true;
        }
    }
    
    printf("DTC storage full, cannot add %c%04X\r\n", type, code);
    return false;
}

bool obd2_dtc_remove(uint16_t code, uint8_t type)
{
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (dtc_manager.dtcs[i].active && 
            dtc_manager.dtcs[i].code == code && 
            dtc_manager.dtcs[i].type == type) {
            
            dtc_manager.dtcs[i].active = false;
            dtc_manager.count--;
            
            printf("Removed DTC: %c%04X\r\n", type, code);
            
            // Check if we should turn off MIL
            if (dtc_manager.count == 0) {
                dtc_manager.mil_status = false;
            }
            
            return true;
        }
    }
    
    return false;
}

void obd2_dtc_clear_all(void)
{
    memset(dtc_manager.dtcs, 0, sizeof(dtc_manager.dtcs));
    dtc_manager.count = 0;
    dtc_manager.mil_status = false;
    dtc_manager.clear_timestamp = to_ms_since_boot(get_absolute_time());
    
    printf("All DTCs cleared\r\n");
}

uint8_t obd2_dtc_get_count(void)
{
    return dtc_manager.count;
}

bool obd2_dtc_get_mil_status(void)
{
    return dtc_manager.mil_status;
}

void obd2_dtc_set_mil_status(bool status)
{
    dtc_manager.mil_status = status;
}

uint8_t obd2_dtc_get_stored(uint8_t *buffer, uint8_t max_size)
{
    uint8_t count = 0;
    uint8_t pos = 0;
    
    // First byte: number of DTCs
    if (pos < max_size) {
        buffer[pos++] = dtc_manager.count;
    }
    
    // Add DTC codes (2 bytes each)
    for (int i = 0; i < MAX_STORED_DTCS && count < dtc_manager.count && (pos + 1) < max_size; i++) {
        if (dtc_manager.dtcs[i].active && (dtc_manager.dtcs[i].status & DTC_STATUS_CONFIRMED)) {
            // Format DTC code for OBD2 transmission
            uint16_t obd2_code = obd2_dtc_format_for_transmission(dtc_manager.dtcs[i].code, dtc_manager.dtcs[i].type);
            buffer[pos++] = (obd2_code >> 8) & 0xFF;
            buffer[pos++] = obd2_code & 0xFF;
            count++;
        }
    }
    
    return pos;
}

uint8_t obd2_dtc_get_pending(uint8_t *buffer, uint8_t max_size)
{
    uint8_t count = 0;
    uint8_t pos = 0;
    
    // Count pending DTCs
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (dtc_manager.dtcs[i].active && (dtc_manager.dtcs[i].status & DTC_STATUS_PENDING)) {
            count++;
        }
    }
    
    // First byte: number of DTCs
    if (pos < max_size) {
        buffer[pos++] = count;
    }
    
    // Add pending DTC codes
    count = 0;
    for (int i = 0; i < MAX_STORED_DTCS && (pos + 1) < max_size; i++) {
        if (dtc_manager.dtcs[i].active && (dtc_manager.dtcs[i].status & DTC_STATUS_PENDING)) {
            uint16_t obd2_code = obd2_dtc_format_for_transmission(dtc_manager.dtcs[i].code, dtc_manager.dtcs[i].type);
            buffer[pos++] = (obd2_code >> 8) & 0xFF;
            buffer[pos++] = obd2_code & 0xFF;
            count++;
        }
    }
    
    return pos;
}

uint8_t obd2_dtc_get_permanent(uint8_t *buffer, uint8_t max_size)
{
    // For simulation, permanent DTCs are the same as confirmed DTCs
    // In a real implementation, permanent DTCs would persist even after clearing
    return obd2_dtc_get_stored(buffer, max_size);
}

uint16_t obd2_dtc_format_for_transmission(uint16_t code, uint8_t type)
{
    uint16_t result = 0;
    
    // Set the first 2 bits based on DTC type
    switch (type) {
        case DTC_TYPE_POWERTRAIN:
            result = 0x0000;  // 00xxxxxxxxxxxxxx
            break;
        case DTC_TYPE_CHASSIS:
            result = 0x4000;  // 01xxxxxxxxxxxxxx
            break;
        case DTC_TYPE_BODY:
            result = 0x8000;  // 10xxxxxxxxxxxxxx
            break;
        case DTC_TYPE_NETWORK:
            result = 0xC000;  // 11xxxxxxxxxxxxxx
            break;
        default:
            result = 0x0000;
            break;
    }
    
    // Add the 14-bit code
    result |= (code & 0x3FFF);
    
    return result;
}

bool obd2_dtc_exists(uint16_t code, uint8_t type)
{
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (dtc_manager.dtcs[i].active && 
            dtc_manager.dtcs[i].code == code && 
            dtc_manager.dtcs[i].type == type) {
            return true;
        }
    }
    return false;
}

uint8_t obd2_dtc_get_status(uint16_t code, uint8_t type)
{
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (dtc_manager.dtcs[i].active && 
            dtc_manager.dtcs[i].code == code && 
            dtc_manager.dtcs[i].type == type) {
            return dtc_manager.dtcs[i].status;
        }
    }
    return 0;
}

void obd2_dtc_format_code_string(uint16_t code, uint8_t type, char *str)
{
    sprintf(str, "%c%04X", type, code);
}

void obd2_dtc_print_all(void)
{
    printf("\r\n=== Stored DTCs ===\r\n");
    printf("Count: %d\r\n", dtc_manager.count);
    printf("MIL Status: %s\r\n", dtc_manager.mil_status ? "ON" : "OFF");
    
    for (int i = 0; i < MAX_STORED_DTCS; i++) {
        if (dtc_manager.dtcs[i].active) {
            printf("DTC %d: %c%04X, Status: 0x%02X\r\n", 
                   i, dtc_manager.dtcs[i].type, dtc_manager.dtcs[i].code, dtc_manager.dtcs[i].status);
        }
    }
    printf("==================\r\n\r\n");
}

// Test and simulation functions
void obd2_dtc_simulate_fault(uint16_t code, uint8_t type)
{
    uint8_t status = DTC_STATUS_TEST_FAILED | DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED;
    obd2_dtc_add(code, type, status);
}

void obd2_dtc_simulate_random_faults(void)
{
    // Simulate some common faults
    obd2_dtc_simulate_fault(DTC_P0171, DTC_TYPE_POWERTRAIN);  // System Too Lean
    obd2_dtc_simulate_fault(DTC_P0301, DTC_TYPE_POWERTRAIN);  // Cylinder 1 Misfire
    
    printf("Simulated random faults\r\n");
}

void obd2_dtc_test_scenario(void)
{
    printf("Running DTC test scenario...\r\n");

    // Clear all DTCs
    obd2_dtc_clear_all();

    // Add some test DTCs
    obd2_dtc_add(DTC_P0100, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
    obd2_dtc_add(DTC_P0171, DTC_TYPE_POWERTRAIN, DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
    obd2_dtc_add(DTC_P0301, DTC_TYPE_POWERTRAIN, DTC_STATUS_TEST_FAILED | DTC_STATUS_CONFIRMED);

    // Print status
    obd2_dtc_print_all();

    printf("DTC test scenario completed\r\n");
}

// Advanced DTC simulation based on vehicle conditions
void obd2_dtc_simulate_realistic_faults(void)
{
    static uint32_t last_simulation = 0;
    static uint32_t simulation_counter = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Run simulation every 10 seconds
    if (current_time - last_simulation < 10000) {
        return;
    }

    last_simulation = current_time;
    simulation_counter++;

    // Get current vehicle parameters for condition-based faults
    extern uint8_t obd2_get_engine_load(void);
    extern uint16_t obd2_get_engine_rpm(void);
    extern uint8_t obd2_get_coolant_temp(void);
    extern uint8_t obd2_get_throttle_position(void);

    uint8_t load = (obd2_get_engine_load() * 100) / 255;
    uint16_t rpm = obd2_get_engine_rpm() / 4;
    uint8_t coolant = obd2_get_coolant_temp() - 40;
    uint8_t throttle = (obd2_get_throttle_position() * 100) / 255;

    // Simulate various fault conditions based on vehicle state

    // 1. High RPM conditions - potential engine issues
    if (rpm > 5000 && simulation_counter % 15 == 0) {
        if (!obd2_dtc_exists(DTC_P0300, DTC_TYPE_POWERTRAIN)) {
            obd2_dtc_add(DTC_P0300, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
            printf("DTC P0300: Random/Multiple Cylinder Misfire (High RPM condition)\r\n");
        }
    }

    // 2. High engine load - potential fuel system issues
    if (load > 80 && simulation_counter % 20 == 0) {
        if (!obd2_dtc_exists(DTC_P0171, DTC_TYPE_POWERTRAIN)) {
            obd2_dtc_add(DTC_P0171, DTC_TYPE_POWERTRAIN,
                        DTC_STATUS_TEST_FAILED | DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
            printf("DTC P0171: System Too Lean Bank 1 (High load condition)\r\n");
        }
    }

    // 3. Temperature-related faults
    if (coolant > 100 && simulation_counter % 25 == 0) {
        if (!obd2_dtc_exists(DTC_P0115, DTC_TYPE_POWERTRAIN)) {
            obd2_dtc_add(DTC_P0115, DTC_TYPE_POWERTRAIN, DTC_STATUS_CONFIRMED);
            printf("DTC P0115: Engine Coolant Temperature Circuit (Overheating)\r\n");
        }
    }

    // 4. Throttle position sensor issues
    if (throttle > 90 && simulation_counter % 30 == 0) {
        if (!obd2_dtc_exists(DTC_P0120, DTC_TYPE_POWERTRAIN)) {
            obd2_dtc_add(DTC_P0120, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
            printf("DTC P0120: Throttle Position Sensor Circuit (Wide open throttle)\r\n");
        }
    }

    // 5. Random intermittent faults (simulate real-world conditions)
    if (simulation_counter % 45 == 0) {
        uint16_t random_dtcs[] = {DTC_P0100, DTC_P0101, DTC_P0110, DTC_P0130, DTC_P0420, DTC_P0500};
        int random_index = simulation_counter % 6;
        uint16_t dtc_code = random_dtcs[random_index];

        if (!obd2_dtc_exists(dtc_code, DTC_TYPE_POWERTRAIN)) {
            uint8_t status = DTC_STATUS_PENDING;
            if (simulation_counter % 3 == 0) {
                status |= DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED;
            }
            obd2_dtc_add(dtc_code, DTC_TYPE_POWERTRAIN, status);
            printf("DTC P%04X: Intermittent fault detected\r\n", dtc_code);
        }
    }

    // 6. Catalyst efficiency (after extended operation)
    if (simulation_counter > 100 && simulation_counter % 60 == 0) {
        if (!obd2_dtc_exists(DTC_P0420, DTC_TYPE_POWERTRAIN)) {
            obd2_dtc_add(DTC_P0420, DTC_TYPE_POWERTRAIN,
                        DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
            printf("DTC P0420: Catalyst System Efficiency Below Threshold (Extended operation)\r\n");
        }
    }

    // 7. Oxygen sensor faults (gradual degradation)
    if (simulation_counter % 80 == 0) {
        if (!obd2_dtc_exists(DTC_P0130, DTC_TYPE_POWERTRAIN)) {
            obd2_dtc_add(DTC_P0130, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
            printf("DTC P0130: O2 Sensor Circuit Malfunction Bank 1 Sensor 1\r\n");
        }
    }

    // 8. Occasionally clear some pending DTCs (simulate intermittent issues)
    if (simulation_counter % 50 == 0) {
        // Clear some pending DTCs to simulate intermittent faults
        for (int i = 0; i < MAX_STORED_DTCS; i++) {
            if (dtc_manager.dtcs[i].active &&
                (dtc_manager.dtcs[i].status & DTC_STATUS_PENDING) &&
                !(dtc_manager.dtcs[i].status & DTC_STATUS_CONFIRMED)) {
                printf("Clearing intermittent DTC P%04X\r\n", dtc_manager.dtcs[i].code);
                obd2_dtc_remove(dtc_manager.dtcs[i].code, dtc_manager.dtcs[i].type);
                break;  // Only clear one at a time
            }
        }
    }
}

// Simulate specific automotive scenarios
void obd2_dtc_simulate_cold_start_issues(void)
{
    printf("Simulating cold start issues...\r\n");
    obd2_dtc_add(DTC_P0125, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
    obd2_dtc_add(DTC_P0110, DTC_TYPE_POWERTRAIN, DTC_STATUS_TEST_FAILED);
    printf("Added cold start related DTCs\r\n");
}

void obd2_dtc_simulate_emissions_failure(void)
{
    printf("Simulating emissions system failure...\r\n");
    obd2_dtc_add(DTC_P0420, DTC_TYPE_POWERTRAIN,
                DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
    obd2_dtc_add(DTC_P0430, DTC_TYPE_POWERTRAIN,
                DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
    obd2_dtc_add(DTC_P0130, DTC_TYPE_POWERTRAIN, DTC_STATUS_CONFIRMED);
    printf("Added emissions system DTCs - MIL should be ON\r\n");
}

void obd2_dtc_simulate_fuel_system_issues(void)
{
    printf("Simulating fuel system issues...\r\n");
    obd2_dtc_add(DTC_P0171, DTC_TYPE_POWERTRAIN,
                DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
    obd2_dtc_add(DTC_P0172, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
    printf("Added fuel system DTCs\r\n");
}

void obd2_dtc_simulate_ignition_misfires(void)
{
    printf("Simulating ignition system misfires...\r\n");
    obd2_dtc_add(DTC_P0300, DTC_TYPE_POWERTRAIN,
                DTC_STATUS_CONFIRMED | DTC_STATUS_WARNING_INDICATOR_REQUESTED);
    obd2_dtc_add(DTC_P0301, DTC_TYPE_POWERTRAIN, DTC_STATUS_CONFIRMED);
    obd2_dtc_add(DTC_P0302, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
    obd2_dtc_add(DTC_P0303, DTC_TYPE_POWERTRAIN, DTC_STATUS_PENDING);
    printf("Added ignition misfire DTCs\r\n");
}
