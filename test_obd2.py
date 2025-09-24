#!/usr/bin/env python3
"""
OBD2 Emulator Test Script

This script can be used to test the OBD2 emulator by sending standard
OBD2 requests and validating responses. It requires a CAN interface
to communicate with the emulator.

Requirements:
- python-can library
- CAN interface (USB-to-CAN adapter, etc.)

Usage:
    python3 test_obd2.py --interface socketcan --channel can0
    python3 test_obd2.py --interface serial --channel /dev/ttyUSB0
"""

import argparse
import time
import sys

try:
    import can
except ImportError:
    print("Error: python-can library not found. Install with: pip install python-can")
    sys.exit(1)

# OBD2 Constants
OBD2_REQUEST_ID = 0x7DF
OBD2_RESPONSE_ID = 0x7E8

# Test cases for OBD2 requests
TEST_CASES = [
    {
        'name': 'Supported PIDs [01-20]',
        'request': [0x02, 0x01, 0x00],
        'expected_service': 0x41,
        'expected_pid': 0x00,
        'description': 'Check which PIDs are supported in range 01-20'
    },
    {
        'name': 'Engine RPM',
        'request': [0x02, 0x01, 0x0C],
        'expected_service': 0x41,
        'expected_pid': 0x0C,
        'description': 'Read current engine RPM'
    },
    {
        'name': 'Vehicle Speed',
        'request': [0x02, 0x01, 0x0D],
        'expected_service': 0x41,
        'expected_pid': 0x0D,
        'description': 'Read current vehicle speed'
    },
    {
        'name': 'Engine Coolant Temperature',
        'request': [0x02, 0x01, 0x05],
        'expected_service': 0x41,
        'expected_pid': 0x05,
        'description': 'Read engine coolant temperature'
    },
    {
        'name': 'Throttle Position',
        'request': [0x02, 0x01, 0x11],
        'expected_service': 0x41,
        'expected_pid': 0x11,
        'description': 'Read throttle position'
    },
    {
        'name': 'Engine Load',
        'request': [0x02, 0x01, 0x04],
        'expected_service': 0x41,
        'expected_pid': 0x04,
        'description': 'Read calculated engine load'
    },
    {
        'name': 'Stored DTCs',
        'request': [0x01, 0x03],
        'expected_service': 0x43,
        'expected_pid': None,
        'description': 'Read stored diagnostic trouble codes'
    },
    {
        'name': 'Pending DTCs',
        'request': [0x01, 0x07],
        'expected_service': 0x47,
        'expected_pid': None,
        'description': 'Read pending diagnostic trouble codes'
    },
    {
        'name': 'VIN Message Count',
        'request': [0x02, 0x09, 0x01],
        'expected_service': 0x49,
        'expected_pid': 0x01,
        'description': 'Get VIN message count'
    },
    {
        'name': 'Vehicle Identification Number',
        'request': [0x02, 0x09, 0x02],
        'expected_service': 0x49,
        'expected_pid': 0x02,
        'description': 'Read Vehicle Identification Number'
    }
]

class OBD2Tester:
    def __init__(self, interface, channel, bitrate=500000):
        """Initialize OBD2 tester with CAN interface"""
        self.interface = interface
        self.channel = channel
        self.bitrate = bitrate
        self.bus = None
        self.test_results = []
        
    def connect(self):
        """Connect to CAN bus"""
        try:
            self.bus = can.interface.Bus(
                interface=self.interface,
                channel=self.channel,
                bitrate=self.bitrate
            )
            print(f"Connected to CAN bus: {self.interface}:{self.channel} @ {self.bitrate} bps")
            return True
        except Exception as e:
            print(f"Failed to connect to CAN bus: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from CAN bus"""
        if self.bus:
            self.bus.shutdown()
            print("Disconnected from CAN bus")
    
    def send_request(self, data):
        """Send OBD2 request and wait for response"""
        if not self.bus:
            return None
            
        # Pad data to 8 bytes
        padded_data = data + [0x00] * (8 - len(data))
        
        # Create and send CAN message
        msg = can.Message(
            arbitration_id=OBD2_REQUEST_ID,
            data=padded_data,
            is_extended_id=False
        )
        
        try:
            self.bus.send(msg)
            print(f"Sent: {' '.join(f'{b:02X}' for b in data)}")
            
            # Wait for response (timeout 2 seconds)
            response = self.bus.recv(timeout=2.0)
            
            if response and response.arbitration_id == OBD2_RESPONSE_ID:
                print(f"Received: {' '.join(f'{b:02X}' for b in response.data)}")
                return response.data
            else:
                print("No response received or wrong ID")
                return None
                
        except Exception as e:
            print(f"Error sending request: {e}")
            return None
    
    def validate_response(self, response, expected_service, expected_pid=None):
        """Validate OBD2 response format and content"""
        if not response or len(response) < 2:
            return False, "Invalid response length"
        
        # Check if it's an error response
        if response[1] == 0x7F:
            error_service = response[2] if len(response) > 2 else 0
            error_code = response[3] if len(response) > 3 else 0
            return False, f"Error response: Service 0x{error_service:02X}, Code 0x{error_code:02X}"
        
        # Check service ID
        if response[1] != expected_service:
            return False, f"Wrong service ID: expected 0x{expected_service:02X}, got 0x{response[1]:02X}"
        
        # Check PID if specified
        if expected_pid is not None:
            if len(response) < 3 or response[2] != expected_pid:
                expected = expected_pid if expected_pid is not None else "N/A"
                actual = response[2] if len(response) > 2 else "N/A"
                return False, f"Wrong PID: expected 0x{expected:02X}, got 0x{actual:02X}"
        
        return True, "Valid response"
    
    def interpret_data(self, test_case, response):
        """Interpret response data based on test case"""
        if not response or len(response) < 3:
            return "No data"
        
        name = test_case['name']
        data = response[3:]  # Skip length, service, PID
        
        if name == 'Engine RPM' and len(data) >= 2:
            rpm = ((data[0] << 8) + data[1]) / 4
            return f"RPM: {rpm:.0f}"
        
        elif name == 'Vehicle Speed' and len(data) >= 1:
            speed = data[0]
            return f"Speed: {speed} km/h"
        
        elif name == 'Engine Coolant Temperature' and len(data) >= 1:
            temp = data[0] - 40
            return f"Coolant Temp: {temp}°C"
        
        elif name == 'Throttle Position' and len(data) >= 1:
            throttle = (data[0] * 100) / 255
            return f"Throttle: {throttle:.1f}%"
        
        elif name == 'Engine Load' and len(data) >= 1:
            load = (data[0] * 100) / 255
            return f"Engine Load: {load:.1f}%"
        
        elif name == 'Supported PIDs [01-20]' and len(data) >= 4:
            supported = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3]
            return f"Supported PIDs: 0x{supported:08X}"
        
        elif name == 'Stored DTCs' and len(data) >= 1:
            dtc_count = data[0]
            return f"DTC Count: {dtc_count}"
        
        elif name == 'VIN Message Count' and len(data) >= 1:
            count = data[0]
            return f"VIN Messages: {count}"
        
        elif name == 'Vehicle Identification Number' and len(data) > 0:
            vin_part = ''.join(chr(b) for b in data if 32 <= b <= 126)
            return f"VIN Part: '{vin_part}'"
        
        else:
            return f"Raw data: {' '.join(f'{b:02X}' for b in data)}"
    
    def run_test(self, test_case):
        """Run a single test case"""
        print(f"\n--- {test_case['name']} ---")
        print(f"Description: {test_case['description']}")
        
        response = self.send_request(test_case['request'])
        
        if response is None:
            result = {
                'name': test_case['name'],
                'passed': False,
                'error': 'No response received',
                'data': None
            }
        else:
            valid, message = self.validate_response(
                response, 
                test_case['expected_service'], 
                test_case.get('expected_pid')
            )
            
            if valid:
                data_interpretation = self.interpret_data(test_case, response)
                print(f"✓ PASS: {message}")
                print(f"Data: {data_interpretation}")
                result = {
                    'name': test_case['name'],
                    'passed': True,
                    'error': None,
                    'data': data_interpretation
                }
            else:
                print(f"✗ FAIL: {message}")
                result = {
                    'name': test_case['name'],
                    'passed': False,
                    'error': message,
                    'data': None
                }
        
        self.test_results.append(result)
        time.sleep(0.5)  # Small delay between tests
    
    def run_all_tests(self):
        """Run all test cases"""
        print("Starting OBD2 Emulator Test Suite")
        print("=" * 50)
        
        for test_case in TEST_CASES:
            self.run_test(test_case)
        
        self.print_summary()
    
    def print_summary(self):
        """Print test summary"""
        print("\n" + "=" * 50)
        print("TEST SUMMARY")
        print("=" * 50)
        
        passed = sum(1 for result in self.test_results if result['passed'])
        total = len(self.test_results)
        
        print(f"Tests passed: {passed}/{total}")
        print(f"Success rate: {(passed/total)*100:.1f}%")
        
        if passed < total:
            print("\nFailed tests:")
            for result in self.test_results:
                if not result['passed']:
                    print(f"  - {result['name']}: {result['error']}")

def main():
    parser = argparse.ArgumentParser(description='OBD2 Emulator Test Script')
    parser.add_argument('--interface', default='socketcan', 
                       help='CAN interface type (socketcan, serial, etc.)')
    parser.add_argument('--channel', default='can0',
                       help='CAN channel (can0, /dev/ttyUSB0, etc.)')
    parser.add_argument('--bitrate', type=int, default=500000,
                       help='CAN bitrate (default: 500000)')
    
    args = parser.parse_args()
    
    tester = OBD2Tester(args.interface, args.channel, args.bitrate)
    
    if not tester.connect():
        sys.exit(1)
    
    try:
        tester.run_all_tests()
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    finally:
        tester.disconnect()

if __name__ == '__main__':
    main()
