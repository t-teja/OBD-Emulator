#ifndef __OBD2_PROTOCOL_H__
#define __OBD2_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

// OBD2 CAN IDs
#define OBD2_REQUEST_ID         0x7DF    // Functional request ID
#define OBD2_RESPONSE_ID_BASE   0x7E8    // Response ID base (7E8-7EF for ECUs 0-7)
#define OBD2_ECU_ID             0x7E8    // Our ECU response ID

// OBD2 Service IDs (SIDs)
#define OBD2_SERVICE_01         0x01     // Show current data
#define OBD2_SERVICE_02         0x02     // Show freeze frame data
#define OBD2_SERVICE_03         0x03     // Show stored DTCs
#define OBD2_SERVICE_04         0x04     // Clear DTCs and stored values
#define OBD2_SERVICE_05         0x05     // Test results, oxygen sensor monitoring
#define OBD2_SERVICE_06         0x06     // Test results, other component/system monitoring
#define OBD2_SERVICE_07         0x07     // Show pending DTCs
#define OBD2_SERVICE_08         0x08     // Control operation of on-board component/system
#define OBD2_SERVICE_09         0x09     // Request vehicle information
#define OBD2_SERVICE_0A         0x0A     // Permanent DTCs

// Response offset for positive responses
#define OBD2_POSITIVE_RESPONSE_OFFSET  0x40

// Common OBD2 PIDs for Service 01 (Show current data)
#define OBD2_PID_SUPPORTED_01_20        0x00    // PIDs supported [01-20]
#define OBD2_PID_MONITOR_STATUS         0x01    // Monitor status since DTCs cleared
#define OBD2_PID_FREEZE_DTC             0x02    // Freeze DTC
#define OBD2_PID_FUEL_SYSTEM_STATUS     0x03    // Fuel system status
#define OBD2_PID_ENGINE_LOAD            0x04    // Calculated engine load
#define OBD2_PID_COOLANT_TEMP           0x05    // Engine coolant temperature
#define OBD2_PID_SHORT_FUEL_TRIM_1      0x06    // Short term fuel trim—Bank 1
#define OBD2_PID_LONG_FUEL_TRIM_1       0x07    // Long term fuel trim—Bank 1
#define OBD2_PID_SHORT_FUEL_TRIM_2      0x08    // Short term fuel trim—Bank 2
#define OBD2_PID_LONG_FUEL_TRIM_2       0x09    // Long term fuel trim—Bank 2
#define OBD2_PID_FUEL_PRESSURE          0x0A    // Fuel pressure
#define OBD2_PID_INTAKE_MAP             0x0B    // Intake manifold absolute pressure
#define OBD2_PID_ENGINE_RPM             0x0C    // Engine RPM
#define OBD2_PID_VEHICLE_SPEED          0x0D    // Vehicle speed
#define OBD2_PID_TIMING_ADVANCE         0x0E    // Timing advance
#define OBD2_PID_INTAKE_TEMP            0x0F    // Intake air temperature
#define OBD2_PID_MAF_RATE               0x10    // MAF air flow rate
#define OBD2_PID_THROTTLE_POS           0x11    // Throttle position
#define OBD2_PID_SECONDARY_AIR_STATUS   0x12    // Commanded secondary air status
#define OBD2_PID_O2_SENSORS_PRESENT     0x13    // Oxygen sensors present
#define OBD2_PID_O2_B1S1                0x14    // Oxygen sensor 1 (Bank 1, Sensor 1)
#define OBD2_PID_O2_B1S2                0x15    // Oxygen sensor 2 (Bank 1, Sensor 2)
#define OBD2_PID_O2_B1S3                0x16    // Oxygen sensor 3 (Bank 1, Sensor 3)
#define OBD2_PID_O2_B1S4                0x17    // Oxygen sensor 4 (Bank 1, Sensor 4)
#define OBD2_PID_O2_B2S1                0x18    // Oxygen sensor 1 (Bank 2, Sensor 1)
#define OBD2_PID_O2_B2S2                0x19    // Oxygen sensor 2 (Bank 2, Sensor 2)
#define OBD2_PID_O2_B2S3                0x1A    // Oxygen sensor 3 (Bank 2, Sensor 3)
#define OBD2_PID_O2_B2S4                0x1B    // Oxygen sensor 4 (Bank 2, Sensor 4)
#define OBD2_PID_OBD_STANDARDS          0x1C    // OBD standards this vehicle conforms to
#define OBD2_PID_O2_SENSORS_PRESENT_2   0x1D    // Oxygen sensors present (2 banks)
#define OBD2_PID_AUX_INPUT_STATUS       0x1E    // Auxiliary input status
#define OBD2_PID_RUNTIME_START          0x1F    // Runtime since engine start
#define OBD2_PID_SUPPORTED_21_40        0x20    // PIDs supported [21-40]

// Additional PIDs for Service 01 (21-40 range)
#define OBD2_PID_DISTANCE_WITH_MIL      0x21    // Distance traveled with MIL on
#define OBD2_PID_FUEL_RAIL_PRESSURE     0x22    // Fuel Rail Pressure (relative to manifold vacuum)
#define OBD2_PID_FUEL_RAIL_GAUGE_PRESS  0x23    // Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)
#define OBD2_PID_O2_S1_WR_LAMBDA        0x24    // Oxygen Sensor 1 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S2_WR_LAMBDA        0x25    // Oxygen Sensor 2 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S3_WR_LAMBDA        0x26    // Oxygen Sensor 3 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S4_WR_LAMBDA        0x27    // Oxygen Sensor 4 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S5_WR_LAMBDA        0x28    // Oxygen Sensor 5 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S6_WR_LAMBDA        0x29    // Oxygen Sensor 6 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S7_WR_LAMBDA        0x2A    // Oxygen Sensor 7 (wide range O2S voltage and lambda)
#define OBD2_PID_O2_S8_WR_LAMBDA        0x2B    // Oxygen Sensor 8 (wide range O2S voltage and lambda)
#define OBD2_PID_COMMANDED_EGR          0x2C    // Commanded EGR
#define OBD2_PID_EGR_ERROR              0x2D    // EGR Error
#define OBD2_PID_COMMANDED_EVAP_PURGE   0x2E    // Commanded evaporative purge
#define OBD2_PID_FUEL_TANK_LEVEL        0x2F    // Fuel Tank Level Input
#define OBD2_PID_WARMUPS_SINCE_CLEAR    0x30    // Warm-ups since codes cleared
#define OBD2_PID_DISTANCE_SINCE_CLEAR   0x31    // Distance traveled since codes cleared
#define OBD2_PID_EVAP_VAPOR_PRESSURE    0x32    // Evap. System Vapor Pressure
#define OBD2_PID_ABSOLUTE_BAROMETRIC    0x33    // Absolute Barometric Pressure
#define OBD2_PID_CATALYST_TEMP_B1S1     0x3C    // Catalyst Temperature: Bank 1, Sensor 1
#define OBD2_PID_CATALYST_TEMP_B2S1     0x3D    // Catalyst Temperature: Bank 2, Sensor 1
#define OBD2_PID_CATALYST_TEMP_B1S2     0x3E    // Catalyst Temperature: Bank 1, Sensor 2
#define OBD2_PID_CATALYST_TEMP_B2S2     0x3F    // Catalyst Temperature: Bank 2, Sensor 2
#define OBD2_PID_SUPPORTED_41_60        0x40    // PIDs supported [41-60]

// Service 09 PIDs (Vehicle Information)
#define OBD2_PID_VIN_MESSAGE_COUNT      0x01    // VIN Message Count
#define OBD2_PID_VIN                    0x02    // Vehicle Identification Number
#define OBD2_PID_CALIBRATION_ID_COUNT   0x03    // Calibration ID message count
#define OBD2_PID_CALIBRATION_ID         0x04    // Calibration ID
#define OBD2_PID_CVN_COUNT              0x05    // Calibration verification numbers message count
#define OBD2_PID_CVN                    0x06    // Calibration Verification Numbers
#define OBD2_PID_IPT_COUNT              0x07    // In-use performance tracking message count
#define OBD2_PID_IPT                    0x08    // In-use performance tracking for spark ignition vehicles
#define OBD2_PID_ESN_COUNT              0x09    // ESN message count
#define OBD2_PID_ESN                    0x0A    // Engine serial number

// Error codes
#define OBD2_ERROR_GENERAL              0x10    // General reject
#define OBD2_ERROR_SERVICE_NOT_SUPPORTED 0x11   // Service not supported
#define OBD2_ERROR_SUBFUNCTION_NOT_SUPPORTED 0x12 // Sub-function not supported
#define OBD2_ERROR_INVALID_FORMAT       0x13    // Incorrect message length or invalid format
#define OBD2_ERROR_RESPONSE_TOO_LONG    0x14    // Response too long
#define OBD2_ERROR_BUSY_REPEAT_REQUEST  0x21    // Busy, repeat request
#define OBD2_ERROR_CONDITIONS_NOT_CORRECT 0x22  // Conditions not correct
#define OBD2_ERROR_REQUEST_SEQUENCE_ERROR 0x24  // Request sequence error
#define OBD2_ERROR_NO_RESPONSE_FROM_SUBNET 0x25 // No response from subnet component
#define OBD2_ERROR_FAILURE_PREVENTS_EXECUTION 0x26 // Failure prevents execution of requested action
#define OBD2_ERROR_REQUEST_OUT_OF_RANGE 0x31    // Request out of range
#define OBD2_ERROR_SECURITY_ACCESS_DENIED 0x33  // Security access denied
#define OBD2_ERROR_INVALID_KEY          0x35    // Invalid key
#define OBD2_ERROR_EXCEED_NUMBER_OF_ATTEMPTS 0x36 // Exceed number of attempts
#define OBD2_ERROR_REQUIRED_TIME_DELAY  0x37    // Required time delay not expired
#define OBD2_ERROR_UPLOAD_DOWNLOAD_NOT_ACCEPTED 0x70 // Upload/download not accepted
#define OBD2_ERROR_TRANSFER_DATA_SUSPENDED 0x71 // Transfer data suspended
#define OBD2_ERROR_GENERAL_PROGRAMMING_FAILURE 0x72 // General programming failure
#define OBD2_ERROR_WRONG_BLOCK_SEQUENCE_COUNTER 0x73 // Wrong block sequence counter
#define OBD2_ERROR_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING 0x78 // Request correctly received, but response is pending
#define OBD2_ERROR_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION 0x7E // Sub-function not supported in active session
#define OBD2_ERROR_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION 0x7F // Service not supported in active session

// OBD2 message structure
typedef struct {
    uint8_t service;        // Service ID
    uint8_t pid;           // Parameter ID
    uint8_t data[6];       // Data bytes (max 6 for single frame)
    uint8_t length;        // Total message length
} obd2_message_t;

// OBD2 response structure
typedef struct {
    uint8_t service;        // Service ID + 0x40 for positive response
    uint8_t pid;           // Parameter ID (if applicable)
    uint8_t data[7];       // Response data
    uint8_t length;        // Total response length
} obd2_response_t;

// Function prototypes
bool obd2_is_valid_request(uint32_t can_id);
bool obd2_parse_message(uint8_t *can_data, uint8_t can_length, obd2_message_t *message);
bool obd2_create_response(obd2_message_t *request, obd2_response_t *response);
void obd2_create_error_response(uint8_t service, uint8_t error_code, obd2_response_t *response);
uint8_t obd2_format_can_message(obd2_response_t *response, uint8_t *can_data);

// Service handlers
bool obd2_handle_service_01(obd2_message_t *request, obd2_response_t *response);
bool obd2_handle_service_03(obd2_message_t *request, obd2_response_t *response);
bool obd2_handle_service_04(obd2_message_t *request, obd2_response_t *response);
bool obd2_handle_service_09(obd2_message_t *request, obd2_response_t *response);

// Vehicle data simulation functions
uint8_t obd2_get_engine_load(void);
uint8_t obd2_get_coolant_temp(void);
uint16_t obd2_get_engine_rpm(void);
uint8_t obd2_get_vehicle_speed(void);
uint8_t obd2_get_intake_temp(void);
uint8_t obd2_get_throttle_position(void);
uint8_t obd2_get_fuel_level(void);
void obd2_clear_dtcs(void);

// Advanced parameter functions
uint16_t obd2_get_maf_flow_rate(void);
uint16_t obd2_get_fuel_pressure(void);
uint16_t obd2_get_manifold_pressure(void);
uint16_t obd2_get_o2_sensor_b1s1(void);
uint16_t obd2_get_o2_sensor_b1s2(void);
uint8_t obd2_get_short_fuel_trim_b1(void);
uint8_t obd2_get_long_fuel_trim_b1(void);
uint8_t obd2_get_timing_advance(void);

// VIN handling
const char* obd2_get_vin(void);
void obd2_set_vin(const char* vin);

// Multi-frame VIN transmission
typedef struct {
    bool active;
    uint8_t frame_number;
    uint8_t total_frames;
    char vin_data[18];  // 17 chars + null terminator
} vin_multiframe_t;

extern vin_multiframe_t vin_transmission;
bool obd2_handle_vin_multiframe(obd2_response_t *response);
void obd2_start_vin_transmission(const char* vin);

#endif // __OBD2_PROTOCOL_H__
