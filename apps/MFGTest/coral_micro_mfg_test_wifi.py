#!/usr/bin/python3

from coral_micro_mfg_test import CoralMicroMFGTest, Antenna

if __name__ == '__main__':
    """
    Sample runner for the RPCs implemented in this class.
    """
    import argparse
    parser = argparse.ArgumentParser(description='Dev Board Micro MFGTest')
    parser.add_argument('--ip_address', type=str, required=False, default='10.10.10.1')
    args = parser.parse_args()
    mfg_test = CoralMicroMFGTest(f'http://{args.ip_address}:80/jsonrpc', print_payloads=True)
    print(mfg_test.wifi_set_antenna(Antenna.INTERNAL))
    print(mfg_test.wifi_scan())
    print(mfg_test.wifi_get_ap('GoogleGuest'))
    print(mfg_test.wifi_set_antenna(Antenna.EXTERNAL))
    print(mfg_test.wifi_get_ap('GoogleGuest'))
    print(mfg_test.ble_scan())
    print(mfg_test.wifi_set_antenna(Antenna.INTERNAL))
    print(mfg_test.ble_scan())
    print(mfg_test.ble_find('e8:84:a5:04:eb:1e'))
