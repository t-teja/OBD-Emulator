#include <stdio.h>
#include "pico/stdlib.h"
#include "obd2_handler.h"
#include "obd2_protocol.h"
#include "obd2_dtc.h"
#include "xl2515.h"

#define LED_PIN         25
#define BUTTON_PIN      22
#define STATUS_LED_PIN  2

// Application state
static struct {
    bool running;
    bool led_state;
    bool button_pressed;
    uint32_t last_led_toggle;
    uint32_t last_stats_print;
    uint32_t last_button_check;
    uint32_t startup_time;
} app_state;

// Function prototypes
void init_hardware(void);
void init_obd2_system(void);
void handle_user_interface(void);
void update_status_indicators(void);
void print_startup_banner(void);
void handle_button_press(void);
void handle_serial_command(char cmd);
void run_diagnostic_tests(void);
void print_realtime_vehicle_data(void);
void print_available_pids(void);

int main()
{
    // Initialize hardware
    init_hardware();
    
    // Print startup banner
    print_startup_banner();
    
    // Initialize OBD2 system
    init_obd2_system();
    
    // Main application loop
    printf("OBD2 Emulator started - Ready for diagnostic requests\r\n");
    printf("ADVANCED PARAMETERS: MAF, Fuel Pressure, MAP, O2 Sensors, Fuel Trim, Timing - ALL ACTIVE\r\n");
    printf("Real-time data displayed every 3 seconds via USB serial\r\n");
    printf("BUTTON/SERIAL: Use GPIO 22 button or serial commands for diagnostics ('h' for help)\r\n");
    printf("Automatic DTC simulation running based on vehicle conditions\r\n");

    // Show available PIDs
    print_available_pids();
    
    app_state.running = true;
    app_state.startup_time = to_ms_since_boot(get_absolute_time());
    
    while (app_state.running) {
        // Process OBD2 messages
        obd2_handler_process();
        
        // Handle user interface
        handle_user_interface();
        
        // Update status indicators
        update_status_indicators();
        
        // Small delay to prevent overwhelming the system
        sleep_ms(10);
    }
    
    return 0;
}

void init_hardware(void)
{
    // Initialize standard I/O
    stdio_init_all();
    
    // Initialize LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    
    // Initialize status LED pin
    gpio_init(STATUS_LED_PIN);
    gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
    gpio_put(STATUS_LED_PIN, 0);
    
    // Initialize button pin
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    
    // Initialize application state
    app_state.led_state = false;
    app_state.button_pressed = false;
    app_state.last_led_toggle = 0;
    app_state.last_stats_print = 0;
    app_state.last_button_check = 0;
    
    printf("Hardware initialized\r\n");
}

void init_obd2_system(void)
{
    // Initialize DTC manager
    obd2_dtc_init();
    
    // Initialize OBD2 handler
    if (!obd2_handler_init()) {
        printf("ERROR: Failed to initialize OBD2 handler\r\n");
        return;
    }
    
    // Add some sample DTCs for testing
    obd2_dtc_simulate_fault(0x0171, DTC_TYPE_POWERTRAIN);  // System Too Lean
    
    printf("OBD2 system initialized successfully\r\n");
}

void print_startup_banner(void)
{
    printf("\r\n");
    printf("================================================\r\n");
    printf("    OBD2 Emulator for Raspberry Pi RP2350\r\n");
    printf("    Waveshare RP2350-CAN Development Board\r\n");
    printf("================================================\r\n");
    printf("Features:\r\n");
    printf("- Standard OBD2 protocol support\r\n");
    printf("- Real-time vehicle parameter simulation\r\n");
    printf("- Diagnostic Trouble Code (DTC) management\r\n");
    printf("- CAN bus communication at 500 kbps\r\n");
    printf("- Compatible with standard OBD2 scan tools\r\n");
    printf("================================================\r\n\r\n");
}

void handle_user_interface(void)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Check for USB serial commands
    int ch = getchar_timeout_us(0);  // Non-blocking read
    if (ch != PICO_ERROR_TIMEOUT) {
        handle_serial_command((char)ch);
    }

    // Check button state every 50ms
    if (current_time - app_state.last_button_check > 50) {
        app_state.last_button_check = current_time;

        bool button_state = !gpio_get(BUTTON_PIN);  // Active low

        if (button_state && !app_state.button_pressed) {
            // Button just pressed
            app_state.button_pressed = true;
            handle_button_press();
        } else if (!button_state && app_state.button_pressed) {
            // Button released
            app_state.button_pressed = false;
        }
    }
    
    // Print real-time data every 3 seconds (including advanced parameters)
    if (current_time - app_state.last_stats_print > 3000) {
        app_state.last_stats_print = current_time;
        print_realtime_vehicle_data();
    }
}

void handle_button_press(void)
{
    static uint8_t button_press_count = 0;
    button_press_count++;

    printf("\r\n=== Button Press %d ===\r\n", button_press_count);

    switch (button_press_count % 8) {
        case 1:
            printf("Displaying current statistics...\r\n");
            obd2_handler_stats();
            break;

        case 2:
            printf("Running diagnostic tests...\r\n");
            run_diagnostic_tests();
            break;

        case 3:
            printf("Displaying DTC information...\r\n");
            obd2_dtc_print_all();
            break;

        case 4:
            printf("Simulating cold start issues...\r\n");
            obd2_dtc_simulate_cold_start_issues();
            obd2_dtc_print_all();
            break;

        case 5:
            printf("Simulating emissions system failure...\r\n");
            obd2_dtc_simulate_emissions_failure();
            obd2_dtc_print_all();
            break;

        case 6:
            printf("Simulating fuel system issues...\r\n");
            obd2_dtc_simulate_fuel_system_issues();
            obd2_dtc_print_all();
            break;

        case 7:
            printf("Simulating ignition misfires...\r\n");
            obd2_dtc_simulate_ignition_misfires();
            obd2_dtc_print_all();
            break;

        case 0:
            printf("Clearing all DTCs...\r\n");
            obd2_dtc_clear_all();
            printf("All DTCs cleared - MIL should be OFF\r\n");
            break;
    }

    printf("======================\r\n\r\n");
}

void handle_serial_command(char cmd)
{
    printf("\r\n=== Serial Command '%c' ===\r\n", cmd);

    switch (cmd) {
        case 's':
        case 'S':
            printf("Displaying current statistics...\r\n");
            obd2_handler_stats();
            break;

        case 't':
        case 'T':
            printf("Running diagnostic tests...\r\n");
            run_diagnostic_tests();
            break;

        case 'd':
        case 'D':
            printf("Displaying DTC information...\r\n");
            obd2_dtc_print_all();
            break;

        case 'c':
        case 'C':
            printf("Simulating cold start issues...\r\n");
            obd2_dtc_simulate_cold_start_issues();
            obd2_dtc_print_all();
            break;

        case 'e':
        case 'E':
            printf("Simulating emissions system failure...\r\n");
            obd2_dtc_simulate_emissions_failure();
            obd2_dtc_print_all();
            break;

        case 'f':
        case 'F':
            printf("Simulating fuel system issues...\r\n");
            obd2_dtc_simulate_fuel_system_issues();
            obd2_dtc_print_all();
            break;

        case 'i':
        case 'I':
            printf("Simulating ignition misfires...\r\n");
            obd2_dtc_simulate_ignition_misfires();
            obd2_dtc_print_all();
            break;

        case 'x':
        case 'X':
            printf("Clearing all DTCs...\r\n");
            obd2_dtc_clear_all();
            printf("All DTCs cleared - MIL should be OFF\r\n");
            break;

        case 'v':
        case 'V':
            printf("Displaying real-time vehicle data...\r\n");
            print_realtime_vehicle_data();
            break;

        case 'n':
        case 'N':
            printf("Vehicle Identification Number (VIN):\r\n");
            printf("Complete VIN: %s\r\n", obd2_get_vin());
            printf("VIN Breakdown:\r\n");
            printf("  World Manufacturer: %.3s\r\n", obd2_get_vin());
            printf("  Vehicle Descriptor: %.6s\r\n", obd2_get_vin() + 3);
            printf("  Check Digit: %c\r\n", obd2_get_vin()[8]);
            printf("  Model Year: %c\r\n", obd2_get_vin()[9]);
            printf("  Plant Code: %c\r\n", obd2_get_vin()[10]);
            printf("  Serial Number: %.6s\r\n", obd2_get_vin() + 11);
            break;

        case 'p':
        case 'P':
            printf("Displaying available OBD2 PIDs...\r\n");
            print_available_pids();
            break;

        case 'h':
        case 'H':
        case '?':
            printf("Available commands:\r\n");
            printf("  s - Show statistics\r\n");
            printf("  t - Run diagnostic tests\r\n");
            printf("  d - Display DTCs\r\n");
            printf("  c - Cold start issues\r\n");
            printf("  e - Emissions failure\r\n");
            printf("  f - Fuel system issues\r\n");
            printf("  i - Ignition misfires\r\n");
            printf("  x - Clear all DTCs\r\n");
            printf("  v - Vehicle data\r\n");
            printf("  n - Complete VIN information\r\n");
            printf("  p - Show available PIDs\r\n");
            printf("  h - Help (this message)\r\n");
            break;

        default:
            printf("Unknown command '%c'. Type 'h' for help.\r\n", cmd);
            break;
    }

    printf("========================\r\n\r\n");
}

void print_available_pids(void)
{
    printf("\r\n=== Available OBD2 PIDs ===\r\n");
    printf("Basic Parameters:\r\n");
    printf("  PID 01: Monitor Status\r\n");
    printf("  PID 04: Engine Load (%%)\r\n");
    printf("  PID 05: Coolant Temperature (°C)\r\n");
    printf("  PID 0C: Engine RPM\r\n");
    printf("  PID 0D: Vehicle Speed (km/h)\r\n");
    printf("  PID 0F: Intake Air Temperature (°C)\r\n");
    printf("  PID 11: Throttle Position (%%)\r\n");
    printf("  PID 1F: Engine Runtime (sec)\r\n");
    printf("  PID 2F: Fuel Tank Level (%%)\r\n");

    printf("\r\nAdvanced Parameters:\r\n");
    printf("  PID 06: Short Term Fuel Trim Bank 1 (%%)\r\n");
    printf("  PID 07: Long Term Fuel Trim Bank 1 (%%)\r\n");
    printf("  PID 0A: Fuel Pressure (kPa)\r\n");
    printf("  PID 0B: Intake Manifold Pressure (kPa)\r\n");
    printf("  PID 0E: Timing Advance (degrees)\r\n");
    printf("  PID 10: MAF Air Flow Rate (g/s)\r\n");
    printf("  PID 14: O2 Sensor Bank 1 Sensor 1 (V)\r\n");
    printf("  PID 15: O2 Sensor Bank 1 Sensor 2 (V)\r\n");
    printf("  PID 22: Fuel Rail Pressure (kPa)\r\n");

    printf("\r\nDiagnostic Services:\r\n");
    printf("  Service 01: Live Data Stream\r\n");
    printf("  Service 03: Read Stored DTCs\r\n");
    printf("  Service 04: Clear DTCs\r\n");
    printf("  Service 07: Read Pending DTCs\r\n");
    printf("  Service 09: Vehicle Information (VIN)\r\n");
    printf("============================\r\n\r\n");
}

void update_status_indicators(void)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Toggle main LED every 1 second to show system is alive
    if (current_time - app_state.last_led_toggle > 1000) {
        app_state.last_led_toggle = current_time;
        app_state.led_state = !app_state.led_state;
        gpio_put(LED_PIN, app_state.led_state);
    }
    
    // Status LED indicates OBD2 activity
    bool activity = (obd2_handler_get_message_count() > 0);
    gpio_put(STATUS_LED_PIN, activity);
    
    // MIL simulation - blink status LED if DTCs are present
    if (obd2_dtc_get_mil_status()) {
        // Blink faster when MIL is on
        bool mil_blink = (current_time / 250) % 2;
        gpio_put(STATUS_LED_PIN, mil_blink);
    }
}

void run_diagnostic_tests(void)
{
    printf("Running OBD2 diagnostic tests...\r\n");
    
    // Test 1: Response generation
    printf("Test 1: Response generation\r\n");
    obd2_handler_test_response();
    
    // Test 2: Simulate various OBD2 requests
    printf("Test 2: Simulating OBD2 requests\r\n");
    obd2_handler_simulate_request(0x01, 0x00);  // Supported PIDs
    obd2_handler_simulate_request(0x01, 0x0C);  // Engine RPM
    obd2_handler_simulate_request(0x01, 0x0D);  // Vehicle Speed
    obd2_handler_simulate_request(0x01, 0x05);  // Coolant Temperature
    
    // Test 3: DTC operations
    printf("Test 3: DTC operations\r\n");
    obd2_handler_simulate_request(0x03, 0x00);  // Read stored DTCs
    obd2_handler_simulate_request(0x07, 0x00);  // Read pending DTCs
    
    // Test 4: Vehicle information
    printf("Test 4: Vehicle information\r\n");
    obd2_handler_simulate_request(0x09, 0x02);  // VIN
    
    printf("Diagnostic tests completed\r\n");
}

// Additional utility functions for demonstration

void obd2_emulator_demo_mode(void)
{
    printf("Entering demo mode - simulating vehicle driving cycle\r\n");
    
    for (int i = 0; i < 10; i++) {
        printf("Demo cycle %d/10\r\n", i + 1);
        
        // Simulate some OBD2 requests
        obd2_handler_simulate_request(0x01, 0x0C);  // RPM
        sleep_ms(500);
        obd2_handler_simulate_request(0x01, 0x0D);  // Speed
        sleep_ms(500);
        obd2_handler_simulate_request(0x01, 0x05);  // Temperature
        sleep_ms(1000);
        
        // Update status
        update_status_indicators();
    }
    
    printf("Demo mode completed\r\n");
}

void print_realtime_vehicle_data(void)
{
    // Get real-time vehicle data
    uint16_t rpm_raw = obd2_get_engine_rpm();
    uint16_t rpm = rpm_raw / 4;  // Convert from OBD2 format
    uint8_t speed = obd2_get_vehicle_speed();
    uint8_t coolant_raw = obd2_get_coolant_temp();
    int16_t coolant = coolant_raw - 40;  // Convert from OBD2 format
    uint8_t throttle_raw = obd2_get_throttle_position();
    uint8_t throttle = (throttle_raw * 100) / 255;  // Convert to percentage
    uint8_t load_raw = obd2_get_engine_load();
    uint8_t load = (load_raw * 100) / 255;  // Convert to percentage
    uint8_t fuel_raw = obd2_get_fuel_level();
    uint8_t fuel = (fuel_raw * 100) / 255;  // Convert to percentage

    // Get advanced parameters
    uint16_t maf_raw = obd2_get_maf_flow_rate();
    float maf_flow = maf_raw / 100.0f;  // Convert to g/s
    uint16_t fuel_pressure_raw = obd2_get_fuel_pressure();
    float fuel_pressure = fuel_pressure_raw / 100.0f;  // Convert to kPa
    uint16_t map_raw = obd2_get_manifold_pressure();
    float manifold_pressure = map_raw / 100.0f;  // Convert to kPa
    uint16_t o2_b1s1_raw = obd2_get_o2_sensor_b1s1();
    float o2_b1s1 = o2_b1s1_raw / 1000.0f;  // Convert to volts
    uint16_t o2_b1s2_raw = obd2_get_o2_sensor_b1s2();
    float o2_b1s2 = o2_b1s2_raw / 1000.0f;  // Convert to volts
    uint8_t stft_raw = obd2_get_short_fuel_trim_b1();
    int8_t stft = ((int16_t)stft_raw - 128) * 100 / 128;  // Convert to percentage
    uint8_t ltft_raw = obd2_get_long_fuel_trim_b1();
    int8_t ltft = ((int16_t)ltft_raw - 128) * 100 / 128;  // Convert to percentage
    uint8_t timing_raw = obd2_get_timing_advance();
    int8_t timing = timing_raw - 64;  // Convert to degrees

    printf("\r\n=== Real-Time Vehicle Data ===\r\n");
    printf("Engine RPM:     %4d RPM\r\n", rpm);
    printf("Vehicle Speed:  %4d km/h\r\n", speed);
    printf("Coolant Temp:   %4d°C\r\n", coolant);
    printf("Throttle Pos:   %4d%%\r\n", throttle);
    printf("Engine Load:    %4d%%\r\n", load);
    printf("Fuel Level:     %4d%%\r\n", fuel);
    printf("VIN:            %s\r\n", obd2_get_vin());
    printf("Engine Runtime: %lu sec\r\n", obd2_get_engine_runtime());
    printf("\r\n--- Advanced Parameters ---\r\n");
    printf("MAF Flow Rate:  %5.2f g/s\r\n", maf_flow);
    printf("Fuel Pressure:  %5.1f kPa\r\n", fuel_pressure);
    printf("Manifold Press: %5.1f kPa\r\n", manifold_pressure);
    printf("O2 Sensor B1S1: %4.3f V\r\n", o2_b1s1);
    printf("O2 Sensor B1S2: %4.3f V\r\n", o2_b1s2);
    printf("Short Fuel Trim: %+3d%%\r\n", stft);
    printf("Long Fuel Trim:  %+3d%%\r\n", ltft);
    printf("Timing Advance:  %+3d°\r\n", timing);
    printf("==============================\r\n\r\n");
}

void obd2_emulator_info(void)
{
    uint32_t uptime = to_ms_since_boot(get_absolute_time()) - app_state.startup_time;

    printf("\r\n=== OBD2 Emulator Information ===\r\n");
    printf("Uptime: %lu seconds\r\n", uptime / 1000);
    printf("System Status: %s\r\n", app_state.running ? "Running" : "Stopped");
    printf("OBD2 Handler: %s\r\n", obd2_handler_is_initialized() ? "Initialized" : "Not Initialized");
    printf("Engine State: %s\r\n", obd2_get_engine_state() ? "Running" : "Stopped");
    printf("Engine Runtime: %lu seconds\r\n", obd2_get_engine_runtime());
    printf("Messages Processed: %lu\r\n", obd2_handler_get_message_count());
    printf("Errors: %lu\r\n", obd2_handler_get_error_count());
    printf("Active DTCs: %d\r\n", obd2_dtc_get_count());
    printf("MIL Status: %s\r\n", obd2_dtc_get_mil_status() ? "ON" : "OFF");
    printf("================================\r\n\r\n");
}
