#include "obd2_protocol.h"
#include "obd2_dtc.h"
#include "pico/stdlib.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// Vehicle state variables
static struct {
    uint32_t engine_runtime;        // Engine runtime in seconds
    uint16_t base_rpm;             // Base RPM
    uint8_t throttle_position;     // Throttle position (0-100%)
    uint8_t vehicle_speed;         // Vehicle speed in km/h
    uint8_t engine_load;           // Engine load (0-100%)
    uint8_t coolant_temp;          // Coolant temperature
    uint8_t intake_temp;           // Intake air temperature
    uint8_t fuel_level;            // Fuel tank level (0-100%)
    bool engine_running;           // Engine state
    uint32_t last_update;          // Last update timestamp
    char vin[18];                  // Vehicle Identification Number (17 chars + null)

    // Advanced parameters
    uint16_t maf_flow_rate;        // Mass Air Flow rate (grams/sec * 100)
    uint16_t fuel_pressure;        // Fuel pressure (kPa)
    uint16_t manifold_pressure;    // Intake manifold absolute pressure (kPa)
    uint16_t o2_sensor_b1s1;       // O2 sensor Bank 1 Sensor 1 (voltage * 1000)
    uint16_t o2_sensor_b1s2;       // O2 sensor Bank 1 Sensor 2 (voltage * 1000)
    uint8_t short_fuel_trim_b1;    // Short term fuel trim Bank 1 (%)
    uint8_t long_fuel_trim_b1;     // Long term fuel trim Bank 1 (%)
    uint8_t timing_advance;        // Timing advance (degrees before TDC)
} vehicle_state = {
    .engine_runtime = 0,
    .base_rpm = 800,               // Idle RPM
    .throttle_position = 0,
    .vehicle_speed = 0,
    .engine_load = 15,             // Idle load
    .coolant_temp = 90,            // Normal operating temp (90°C + 40 = 130 for OBD2)
    .intake_temp = 25,             // Ambient temp (25°C + 40 = 65 for OBD2)
    .fuel_level = 75,              // 75% fuel level
    .engine_running = true,
    .last_update = 0,
    .vin = "1HGBH41JXMN109186",   // Honda Civic VIN example

    // Advanced parameters initialization
    .maf_flow_rate = 1500,         // 15.00 g/s (typical idle)
    .fuel_pressure = 30000,        // 300 kPa (typical fuel rail pressure)
    .manifold_pressure = 3500,     // 35 kPa (typical idle MAP)
    .o2_sensor_b1s1 = 450,         // 0.45V (typical O2 sensor voltage)
    .o2_sensor_b1s2 = 420,         // 0.42V (downstream O2 sensor)
    .short_fuel_trim_b1 = 128,     // 0% trim (128 = 0%, 100-155 range)
    .long_fuel_trim_b1 = 128,      // 0% trim
    .timing_advance = 15           // 15 degrees before TDC
};

// Simulation parameters
static struct {
    float rpm_variation;           // RPM variation factor
    float speed_variation;         // Speed variation factor
    uint32_t simulation_cycle;     // Simulation cycle counter
} sim_params = {
    .rpm_variation = 0.0f,
    .speed_variation = 0.0f,
    .simulation_cycle = 0
};

// Forward declarations for static functions
static void simulate_engine_dynamics(void);
static void simulate_vehicle_movement(void);
static void simulate_temperature_changes(void);
static void simulate_advanced_parameters(void);

// Update vehicle simulation (call this periodically)
void obd2_update_vehicle_simulation(void)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Update every 50ms for more responsive real-time data
    if (current_time - vehicle_state.last_update < 50) {
        return;
    }
    
    vehicle_state.last_update = current_time;
    sim_params.simulation_cycle++;
    
    if (vehicle_state.engine_running) {
        vehicle_state.engine_runtime++;

        // Simulate realistic engine behavior
        simulate_engine_dynamics();
        simulate_vehicle_movement();
        simulate_temperature_changes();

        // Simulate realistic DTC generation based on conditions
        obd2_dtc_simulate_realistic_faults();
    }
}

static void simulate_engine_dynamics(void)
{
    // Create realistic driving patterns with multiple cycles
    float time_factor = sim_params.simulation_cycle * 0.005f;

    // Simulate different driving scenarios
    float city_driving = sin(time_factor) * 25.0f + 35.0f;           // City: 10-60%
    float highway_driving = sin(time_factor * 0.3f) * 15.0f + 65.0f; // Highway: 50-80%
    float idle_pattern = sin(time_factor * 2.0f) * 5.0f + 10.0f;     // Idle: 5-15%

    // Mix driving patterns based on cycle
    float pattern_selector = sin(time_factor * 0.1f);
    float throttle_base;

    if (pattern_selector > 0.3f) {
        throttle_base = highway_driving;  // Highway driving
    } else if (pattern_selector > -0.3f) {
        throttle_base = city_driving;     // City driving
    } else {
        throttle_base = idle_pattern;     // Idle/parking
    }

    // Add small random variations for realism
    float micro_variation = sin(time_factor * 5.0f) * 3.0f;
    vehicle_state.throttle_position = (uint8_t)(throttle_base + micro_variation);

    // Clamp throttle position
    if (vehicle_state.throttle_position > 100) vehicle_state.throttle_position = 100;
    if (vehicle_state.throttle_position < 0) vehicle_state.throttle_position = 0;

    // Engine load correlates with throttle but has some lag
    static uint8_t target_load = 15;
    target_load = 15 + (vehicle_state.throttle_position * 85) / 100;

    // Smooth load changes (engine response lag)
    if (vehicle_state.engine_load < target_load) {
        vehicle_state.engine_load += 2;
    } else if (vehicle_state.engine_load > target_load) {
        vehicle_state.engine_load -= 1;
    }

    // RPM calculation with realistic response
    float rpm_base = 800.0f;  // Idle RPM
    float rpm_factor = (vehicle_state.throttle_position / 100.0f) * 4500.0f;  // Max additional RPM

    // Add engine vibration and variation
    float rpm_vibration = sin(time_factor * 20.0f) * 50.0f;  // Engine vibration
    float rpm_variation = sin(time_factor * 1.5f) * 200.0f;  // Load variations

    vehicle_state.base_rpm = (uint16_t)(rpm_base + rpm_factor + rpm_vibration + rpm_variation);

    // Realistic RPM limits
    if (vehicle_state.base_rpm > 6500) vehicle_state.base_rpm = 6500;
    if (vehicle_state.base_rpm < 650) vehicle_state.base_rpm = 650;

    // Simulate advanced parameters based on engine conditions
    simulate_advanced_parameters();
}

static void simulate_vehicle_movement(void)
{
    // Vehicle speed correlates with RPM and throttle
    if (vehicle_state.throttle_position > 10) {
        float speed_factor = (vehicle_state.base_rpm - 800) / 5200.0f;  // Normalize RPM
        vehicle_state.vehicle_speed = (uint8_t)(speed_factor * 120);    // Max 120 km/h
        
        // Add some variation
        sim_params.speed_variation = sin(sim_params.simulation_cycle * 0.015f) * 10.0f;
        vehicle_state.vehicle_speed += (int8_t)sim_params.speed_variation;
    } else {
        // Gradually slow down when throttle is low
        if (vehicle_state.vehicle_speed > 0) {
            vehicle_state.vehicle_speed--;
        }
    }
    
    // Limit speed to realistic range
    if (vehicle_state.vehicle_speed > 200) vehicle_state.vehicle_speed = 200;
    if (vehicle_state.vehicle_speed < 0) vehicle_state.vehicle_speed = 0;
}

static void simulate_temperature_changes(void)
{
    // Coolant temperature simulation
    if (vehicle_state.engine_load > 50) {
        // Engine working hard, temperature rises slightly
        if (vehicle_state.coolant_temp < 95) {
            vehicle_state.coolant_temp++;
        }
    } else if (vehicle_state.engine_load < 30) {
        // Light load, temperature drops slightly
        if (vehicle_state.coolant_temp > 85) {
            vehicle_state.coolant_temp--;
        }
    }
    
    // Intake air temperature varies with engine load and ambient
    uint8_t base_intake = 25;  // Ambient temperature
    uint8_t heat_addition = (vehicle_state.engine_load * 20) / 100;  // Engine heat
    vehicle_state.intake_temp = base_intake + heat_addition;
    
    // Fuel level decreases very slowly during operation
    if (sim_params.simulation_cycle % 1000 == 0 && vehicle_state.fuel_level > 0) {
        vehicle_state.fuel_level--;
    }
}

// OBD2 data getter functions (convert to OBD2 format)

uint8_t obd2_get_engine_load(void)
{
    obd2_update_vehicle_simulation();
    // Engine load: 0-100% -> 0-255 (A*100/255)
    return (vehicle_state.engine_load * 255) / 100;
}

uint8_t obd2_get_coolant_temp(void)
{
    obd2_update_vehicle_simulation();
    // Coolant temp: °C -> °C + 40 (A-40)
    return vehicle_state.coolant_temp + 40;
}

uint16_t obd2_get_engine_rpm(void)
{
    obd2_update_vehicle_simulation();
    // Engine RPM: RPM -> RPM/4 (((A*256)+B)/4)
    return vehicle_state.base_rpm * 4;
}

uint8_t obd2_get_vehicle_speed(void)
{
    obd2_update_vehicle_simulation();
    // Vehicle speed: km/h (A)
    return vehicle_state.vehicle_speed;
}

uint8_t obd2_get_intake_temp(void)
{
    obd2_update_vehicle_simulation();
    // Intake air temp: °C -> °C + 40 (A-40)
    return vehicle_state.intake_temp + 40;
}

uint8_t obd2_get_throttle_position(void)
{
    obd2_update_vehicle_simulation();
    // Throttle position: 0-100% -> 0-255 (A*100/255)
    return (vehicle_state.throttle_position * 255) / 100;
}

uint8_t obd2_get_fuel_level(void)
{
    obd2_update_vehicle_simulation();
    // Fuel tank level: 0-100% -> 0-255 (A*100/255)
    return (vehicle_state.fuel_level * 255) / 100;
}

void obd2_clear_dtcs(void)
{
    // Clear diagnostic trouble codes (simulation)
    // In a real implementation, this would clear stored fault codes
    // For simulation, we just acknowledge the command
}

// Additional utility functions

void obd2_set_engine_state(bool running)
{
    vehicle_state.engine_running = running;
    if (!running) {
        vehicle_state.base_rpm = 0;
        vehicle_state.engine_load = 0;
        vehicle_state.throttle_position = 0;
    } else {
        vehicle_state.base_rpm = 800;  // Idle RPM
        vehicle_state.engine_load = 15; // Idle load
    }
}

bool obd2_get_engine_state(void)
{
    return vehicle_state.engine_running;
}

uint32_t obd2_get_engine_runtime(void)
{
    return vehicle_state.engine_runtime;
}

// Initialize vehicle simulation
void obd2_init_vehicle_simulation(void)
{
    vehicle_state.last_update = to_ms_since_boot(get_absolute_time());
    sim_params.simulation_cycle = 0;
    vehicle_state.engine_running = true;
}

// VIN management functions
const char* obd2_get_vin(void)
{
    return vehicle_state.vin;
}

void obd2_set_vin(const char* vin)
{
    if (vin != NULL) {
        strncpy(vehicle_state.vin, vin, 17);
        vehicle_state.vin[17] = '\0';  // Ensure null termination
    }
}

static void simulate_advanced_parameters(void)
{
    float time_factor = sim_params.simulation_cycle * 0.005f;
    uint16_t rpm = vehicle_state.base_rpm;
    uint8_t load = vehicle_state.engine_load;
    uint8_t throttle = vehicle_state.throttle_position;

    // MAF Flow Rate (Mass Air Flow) - correlates with RPM and throttle
    // Formula: Base flow + RPM factor + throttle factor + variations
    float base_maf = 2.0f;  // Base flow at idle (g/s)
    float rpm_factor = (rpm - 650) / 6000.0f * 25.0f;  // RPM contribution
    float throttle_factor = (throttle / 100.0f) * 15.0f;  // Throttle contribution
    float maf_variation = sin(time_factor * 3.0f) * 2.0f;  // Small variations

    float maf_flow = base_maf + rpm_factor + throttle_factor + maf_variation;
    if (maf_flow < 0.5f) maf_flow = 0.5f;
    if (maf_flow > 50.0f) maf_flow = 50.0f;
    vehicle_state.maf_flow_rate = (uint16_t)(maf_flow * 100);  // Store as g/s * 100

    // Fuel Pressure - varies with engine load and fuel demand
    float base_fuel_pressure = 300.0f;  // Base pressure (kPa)
    float load_pressure_factor = (load / 100.0f) * 50.0f;  // Load increases pressure
    float pressure_variation = sin(time_factor * 2.0f) * 10.0f;  // Pressure variations

    float fuel_pressure = base_fuel_pressure + load_pressure_factor + pressure_variation;
    if (fuel_pressure < 250.0f) fuel_pressure = 250.0f;
    if (fuel_pressure > 400.0f) fuel_pressure = 400.0f;
    vehicle_state.fuel_pressure = (uint16_t)(fuel_pressure * 100);  // Store as kPa * 100

    // Manifold Absolute Pressure (MAP) - inversely related to throttle
    float atmospheric_pressure = 101.3f;  // kPa at sea level
    float vacuum_factor = (100 - throttle) / 100.0f * 70.0f;  // More throttle = less vacuum
    float map_variation = sin(time_factor * 4.0f) * 3.0f;  // Engine pulsations

    float manifold_pressure = atmospheric_pressure - vacuum_factor + map_variation;
    if (manifold_pressure < 20.0f) manifold_pressure = 20.0f;
    if (manifold_pressure > 105.0f) manifold_pressure = 105.0f;
    vehicle_state.manifold_pressure = (uint16_t)(manifold_pressure * 100);  // Store as kPa * 100

    // O2 Sensor Values - simulate lambda sensor behavior
    // Upstream sensor (B1S1) - more active, switches around stoichiometric
    float lambda_target = 1.0f;  // Stoichiometric ratio
    float fuel_trim_effect = (vehicle_state.short_fuel_trim_b1 - 128) / 128.0f * 0.1f;
    float o2_variation = sin(time_factor * 8.0f) * 0.15f;  // O2 sensor switching

    float o2_voltage_b1s1 = 0.45f + fuel_trim_effect + o2_variation;
    if (o2_voltage_b1s1 < 0.1f) o2_voltage_b1s1 = 0.1f;
    if (o2_voltage_b1s1 > 0.9f) o2_voltage_b1s1 = 0.9f;
    vehicle_state.o2_sensor_b1s1 = (uint16_t)(o2_voltage_b1s1 * 1000);  // Store as mV

    // Downstream sensor (B1S2) - less active, more stable
    float o2_voltage_b1s2 = 0.42f + fuel_trim_effect * 0.5f + sin(time_factor * 2.0f) * 0.05f;
    if (o2_voltage_b1s2 < 0.2f) o2_voltage_b1s2 = 0.2f;
    if (o2_voltage_b1s2 > 0.7f) o2_voltage_b1s2 = 0.7f;
    vehicle_state.o2_sensor_b1s2 = (uint16_t)(o2_voltage_b1s2 * 1000);  // Store as mV

    // Fuel Trim - simulate closed-loop fuel control
    static float fuel_trim_integrator = 0.0f;
    float o2_error = o2_voltage_b1s1 - 0.45f;  // Error from target
    fuel_trim_integrator += o2_error * 0.1f;  // Integrate error

    // Limit integrator
    if (fuel_trim_integrator > 25.0f) fuel_trim_integrator = 25.0f;
    if (fuel_trim_integrator < -25.0f) fuel_trim_integrator = -25.0f;

    vehicle_state.short_fuel_trim_b1 = (uint8_t)(128 + fuel_trim_integrator);
    vehicle_state.long_fuel_trim_b1 = (uint8_t)(128 + fuel_trim_integrator * 0.3f);

    // Timing Advance - varies with RPM and load
    float base_timing = 10.0f;  // Base timing advance
    float rpm_timing = (rpm - 650) / 6000.0f * 25.0f;  // More advance at higher RPM
    float load_timing = -(load / 100.0f) * 8.0f;  // Less advance under load

    float timing_advance = base_timing + rpm_timing + load_timing;
    if (timing_advance < -5.0f) timing_advance = -5.0f;
    if (timing_advance > 35.0f) timing_advance = 35.0f;
    vehicle_state.timing_advance = (uint8_t)(timing_advance + 64);  // Store as degrees + 64 offset

    // Log significant parameter changes (every 30 simulation cycles = ~1.5 seconds)
    static uint32_t last_log_cycle = 0;
    if (sim_params.simulation_cycle - last_log_cycle >= 30) {
        last_log_cycle = sim_params.simulation_cycle;

        printf("Advanced Parameters Update: MAF=%.1fg/s, FuelP=%.0fkPa, MAP=%.0fkPa, O2=%.3fV, STFT=%+d%%, Timing=%+d°\r\n",
               maf_flow, fuel_pressure, manifold_pressure, o2_voltage_b1s1,
               (int8_t)((vehicle_state.short_fuel_trim_b1 - 128) * 100 / 128),
               (int8_t)(vehicle_state.timing_advance - 64));
    }
}

// Advanced parameter getter functions
uint16_t obd2_get_maf_flow_rate(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.maf_flow_rate;  // Returns g/s * 100
}

uint16_t obd2_get_fuel_pressure(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.fuel_pressure;  // Returns kPa * 100
}

uint16_t obd2_get_manifold_pressure(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.manifold_pressure;  // Returns kPa * 100
}

uint16_t obd2_get_o2_sensor_b1s1(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.o2_sensor_b1s1;  // Returns mV
}

uint16_t obd2_get_o2_sensor_b1s2(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.o2_sensor_b1s2;  // Returns mV
}

uint8_t obd2_get_short_fuel_trim_b1(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.short_fuel_trim_b1;  // Returns 128 +/- trim%
}

uint8_t obd2_get_long_fuel_trim_b1(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.long_fuel_trim_b1;  // Returns 128 +/- trim%
}

uint8_t obd2_get_timing_advance(void)
{
    obd2_update_vehicle_simulation();
    return vehicle_state.timing_advance;  // Returns degrees + 64 offset
}
