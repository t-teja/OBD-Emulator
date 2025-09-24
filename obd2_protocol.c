#include "obd2_protocol.h"
#include "obd2_dtc.h"
#include <string.h>
#include <stdio.h>

// Multi-frame VIN transmission state
vin_multiframe_t vin_transmission = {0};

// Supported PIDs bitmasks for Service 01
// Calculate bitmask: bit 31 = PID 01, bit 30 = PID 02, etc.
static const uint32_t supported_pids_01_20 = 0xBE1FE813;  // PIDs: 01,04,05,06,07,0A,0B,0C,0D,0E,0F,10,11,13,14,15,1C,1F,20
static const uint32_t supported_pids_21_40 = 0x84000001;  // PIDs: 21,22,2F,40
static const uint32_t supported_pids_41_60 = 0x00000000;  // PIDs 41-60 supported (none)

bool obd2_is_valid_request(uint32_t can_id)
{
    return (can_id == OBD2_REQUEST_ID);
}

bool obd2_parse_message(uint8_t *can_data, uint8_t can_length, obd2_message_t *message)
{
    if (can_data == NULL || message == NULL || can_length < 2) {
        return false;
    }
    
    // Single frame format: [Length][Service][PID][Data...]
    uint8_t pci = can_data[0] & 0xF0;  // Protocol Control Information
    uint8_t length = can_data[0] & 0x0F;  // Data length
    
    // Only handle single frame messages for now
    if (pci != 0x00) {
        return false;
    }
    
    if (length < 1 || length > 7 || length > (can_length - 1)) {
        return false;
    }
    
    message->service = can_data[1];
    message->length = length;
    
    if (length >= 2) {
        message->pid = can_data[2];
    } else {
        message->pid = 0;
    }
    
    // Copy additional data if present
    for (int i = 0; i < (length - 2) && i < 6; i++) {
        if ((i + 3) < can_length) {
            message->data[i] = can_data[i + 3];
        }
    }
    
    return true;
}

bool obd2_create_response(obd2_message_t *request, obd2_response_t *response)
{
    if (request == NULL || response == NULL) {
        return false;
    }
    
    // Initialize response
    memset(response, 0, sizeof(obd2_response_t));
    
    switch (request->service) {
        case OBD2_SERVICE_01:  // Show current data
            return obd2_handle_service_01(request, response);
            
        case OBD2_SERVICE_03:  // Show stored DTCs
            return obd2_handle_service_03(request, response);
            
        case OBD2_SERVICE_04:  // Clear DTCs
            return obd2_handle_service_04(request, response);
            
        case OBD2_SERVICE_09:  // Vehicle information
            return obd2_handle_service_09(request, response);
            
        default:
            obd2_create_error_response(request->service, OBD2_ERROR_SERVICE_NOT_SUPPORTED, response);
            return true;
    }
}

bool obd2_handle_service_01(obd2_message_t *request, obd2_response_t *response)
{
    response->service = OBD2_SERVICE_01 + OBD2_POSITIVE_RESPONSE_OFFSET;
    response->pid = request->pid;
    
    switch (request->pid) {
        case OBD2_PID_SUPPORTED_01_20:
            response->data[0] = (supported_pids_01_20 >> 24) & 0xFF;
            response->data[1] = (supported_pids_01_20 >> 16) & 0xFF;
            response->data[2] = (supported_pids_01_20 >> 8) & 0xFF;
            response->data[3] = supported_pids_01_20 & 0xFF;
            response->length = 6;  // Service + PID + 4 data bytes
            break;
            
        case OBD2_PID_MONITOR_STATUS:
            response->data[0] = 0x07;  // MIL off, 3 DTCs available
            response->data[1] = 0xFF;  // Tests available
            response->data[2] = 0x00;  // Tests incomplete
            response->data[3] = 0xFF;  // Tests available (cont.)
            response->length = 6;
            break;
            
        case OBD2_PID_ENGINE_LOAD:
            response->data[0] = obd2_get_engine_load();
            response->length = 3;
            break;
            
        case OBD2_PID_COOLANT_TEMP:
            response->data[0] = obd2_get_coolant_temp();
            response->length = 3;
            break;
            
        case OBD2_PID_ENGINE_RPM:
            {
                uint16_t rpm = obd2_get_engine_rpm();
                response->data[0] = (rpm >> 8) & 0xFF;
                response->data[1] = rpm & 0xFF;
                response->length = 4;
            }
            break;
            
        case OBD2_PID_VEHICLE_SPEED:
            response->data[0] = obd2_get_vehicle_speed();
            response->length = 3;
            break;
            
        case OBD2_PID_FUEL_PRESSURE:
            {
                uint16_t pressure = obd2_get_fuel_pressure();
                response->data[0] = pressure / 300;  // Convert kPa*100 to 3kPa units
                response->length = 3;
            }
            break;

        case OBD2_PID_INTAKE_MAP:
            {
                uint16_t map = obd2_get_manifold_pressure();
                response->data[0] = map / 100;  // Convert kPa*100 to kPa
                response->length = 3;
            }
            break;

        case OBD2_PID_TIMING_ADVANCE:
            response->data[0] = obd2_get_timing_advance();
            response->length = 3;
            break;

        case OBD2_PID_INTAKE_TEMP:
            response->data[0] = obd2_get_intake_temp();
            response->length = 3;
            break;

        case OBD2_PID_MAF_RATE:
            {
                uint16_t maf = obd2_get_maf_flow_rate();
                response->data[0] = (maf >> 8) & 0xFF;  // High byte
                response->data[1] = maf & 0xFF;         // Low byte
                response->length = 4;
            }
            break;

        case OBD2_PID_THROTTLE_POS:
            response->data[0] = obd2_get_throttle_position();
            response->length = 3;
            break;

        case OBD2_PID_O2_B1S1:
            {
                uint16_t o2_voltage = obd2_get_o2_sensor_b1s1();
                response->data[0] = (o2_voltage * 255) / 1000;  // Convert mV to 0-255 scale
                response->data[1] = 0xFF;  // Short term fuel trim (not used in this format)
                response->length = 4;
            }
            break;

        case OBD2_PID_O2_B1S2:
            {
                uint16_t o2_voltage = obd2_get_o2_sensor_b1s2();
                response->data[0] = (o2_voltage * 255) / 1000;  // Convert mV to 0-255 scale
                response->data[1] = 0xFF;  // Short term fuel trim (not used in this format)
                response->length = 4;
            }
            break;

        case OBD2_PID_SHORT_FUEL_TRIM_1:
            response->data[0] = obd2_get_short_fuel_trim_b1();
            response->length = 3;
            break;

        case OBD2_PID_LONG_FUEL_TRIM_1:
            response->data[0] = obd2_get_long_fuel_trim_b1();
            response->length = 3;
            break;
            
        case OBD2_PID_FUEL_TANK_LEVEL:
            response->data[0] = obd2_get_fuel_level();
            response->length = 3;
            break;

        case OBD2_PID_FUEL_RAIL_PRESSURE:
            {
                uint16_t pressure = obd2_get_fuel_pressure();
                // Fuel rail pressure relative to manifold vacuum
                uint16_t map = obd2_get_manifold_pressure();
                uint16_t relative_pressure = pressure - map;
                response->data[0] = (relative_pressure >> 8) & 0xFF;
                response->data[1] = relative_pressure & 0xFF;
                response->length = 4;
            }
            break;

        case OBD2_PID_SUPPORTED_21_40:
            response->data[0] = (supported_pids_21_40 >> 24) & 0xFF;
            response->data[1] = (supported_pids_21_40 >> 16) & 0xFF;
            response->data[2] = (supported_pids_21_40 >> 8) & 0xFF;
            response->data[3] = supported_pids_21_40 & 0xFF;
            response->length = 6;
            break;
            
        case OBD2_PID_SUPPORTED_41_60:
            response->data[0] = (supported_pids_41_60 >> 24) & 0xFF;
            response->data[1] = (supported_pids_41_60 >> 16) & 0xFF;
            response->data[2] = (supported_pids_41_60 >> 8) & 0xFF;
            response->data[3] = supported_pids_41_60 & 0xFF;
            response->length = 6;
            break;
            
        default:
            obd2_create_error_response(request->service, OBD2_ERROR_SUBFUNCTION_NOT_SUPPORTED, response);
            return true;
    }
    
    return true;
}

bool obd2_handle_service_03(obd2_message_t *request, obd2_response_t *response)
{
    response->service = OBD2_SERVICE_03 + OBD2_POSITIVE_RESPONSE_OFFSET;

    // Get stored DTCs from DTC manager
    uint8_t dtc_buffer[8];
    uint8_t dtc_data_length = obd2_dtc_get_stored(dtc_buffer, sizeof(dtc_buffer));

    if (dtc_data_length > 0 && dtc_data_length <= 7) {
        // Copy DTC data to response
        memcpy(response->data, dtc_buffer, dtc_data_length);
        response->length = 2 + dtc_data_length;  // Service + data
    } else {
        // No DTCs stored
        response->data[0] = 0x00;
        response->length = 3;
    }

    return true;
}

bool obd2_handle_service_04(obd2_message_t *request, obd2_response_t *response)
{
    response->service = OBD2_SERVICE_04 + OBD2_POSITIVE_RESPONSE_OFFSET;
    response->length = 2;  // Just service response
    
    // Clear DTCs (simulation - just acknowledge)
    obd2_clear_dtcs();
    
    return true;
}

bool obd2_handle_service_09(obd2_message_t *request, obd2_response_t *response)
{
    response->service = OBD2_SERVICE_09 + OBD2_POSITIVE_RESPONSE_OFFSET;
    response->pid = request->pid;
    
    switch (request->pid) {
        case OBD2_PID_VIN_MESSAGE_COUNT:
            response->data[0] = 0x01;  // 1 message for VIN
            response->length = 3;
            break;
            
        case OBD2_PID_VIN:
            {
                // Get complete VIN from vehicle data
                const char* vin = obd2_get_vin();

                // Send VIN in proper OBD2 format
                // Many scan tools can handle this format for complete VIN
                // Format: [Data_Length][VIN_17_chars] - total 18 bytes
                // We'll send as much as possible in 6 bytes available

                response->data[0] = 17;  // VIN length (17 characters)

                // Send first 5 VIN characters (max that fits in remaining 5 bytes)
                memcpy(&response->data[1], vin, 5);
                response->length = 8;  // Service + PID + length + 5 VIN chars

                printf("VIN requested - Full VIN: %s\r\n", vin);
                printf("Sending: Length=17, First 5 chars=%.5s\r\n", vin);
                printf("Complete VIN available via USB serial interface\r\n");
            }
            break;
            
        default:
            obd2_create_error_response(request->service, OBD2_ERROR_SUBFUNCTION_NOT_SUPPORTED, response);
            return true;
    }
    
    return true;
}

void obd2_create_error_response(uint8_t service, uint8_t error_code, obd2_response_t *response)
{
    response->service = 0x7F;  // Negative response service
    response->pid = service;   // Original service that failed
    response->data[0] = error_code;
    response->length = 3;
}

uint8_t obd2_format_can_message(obd2_response_t *response, uint8_t *can_data)
{
    if (response == NULL || can_data == NULL) {
        return 0;
    }
    
    // Single frame format: [Length][Service][PID][Data...]
    can_data[0] = response->length;  // PCI + Length
    can_data[1] = response->service;
    
    if (response->length >= 2) {
        can_data[2] = response->pid;
    }
    
    // Copy response data
    for (int i = 0; i < (response->length - 2) && i < 5; i++) {
        can_data[i + 3] = response->data[i];
    }
    
    // Pad remaining bytes with 0x00 (optional)
    for (int i = response->length + 1; i < 8; i++) {
        can_data[i] = 0x00;
    }
    
    return response->length + 1;  // Total CAN message length
}

// Multi-frame VIN transmission functions
void obd2_start_vin_transmission(const char* vin)
{
    if (vin != NULL) {
        vin_transmission.active = true;
        vin_transmission.frame_number = 1;  // Start with frame 1 (first frame is 0)
        vin_transmission.total_frames = 4;  // Total frames needed for 17-char VIN
        strncpy(vin_transmission.vin_data, vin, 17);
        vin_transmission.vin_data[17] = '\0';

        printf("Multi-frame VIN transmission started for: %s\r\n", vin);
    }
}

bool obd2_handle_vin_multiframe(obd2_response_t *response)
{
    if (!vin_transmission.active) {
        return false;
    }

    // Calculate data offset for this frame
    uint8_t data_offset = 4 + (vin_transmission.frame_number - 1) * 7;  // 4 chars in first frame, 7 in each subsequent
    uint8_t remaining_chars = 17 - (4 + (vin_transmission.frame_number - 1) * 7);
    uint8_t chars_this_frame = (remaining_chars > 7) ? 7 : remaining_chars;

    if (chars_this_frame > 0) {
        // Consecutive frame format: [2X][Data...]
        // Where X = frame sequence number (1-F)
        response->data[0] = 0x20 | vin_transmission.frame_number;  // Consecutive frame + sequence

        // Copy VIN data for this frame
        memcpy(&response->data[1], &vin_transmission.vin_data[data_offset], chars_this_frame);

        // Pad remaining bytes with 0x00 if needed
        for (int i = chars_this_frame + 1; i < 8; i++) {
            response->data[i] = 0x00;
        }

        response->length = 8;

        printf("Consecutive frame %d: %.7s\r\n", vin_transmission.frame_number,
               &vin_transmission.vin_data[data_offset]);

        vin_transmission.frame_number++;

        // Check if transmission complete
        if (vin_transmission.frame_number > vin_transmission.total_frames) {
            vin_transmission.active = false;
            printf("VIN multi-frame transmission completed\r\n");
        }

        return true;
    }

    vin_transmission.active = false;
    return false;
}
