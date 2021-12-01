#!/usr/bin/python3

from valiant_mfg_test import ValiantMFGTest, Antenna

if __name__ == '__main__':
    """
    Sample runner for the RPCs implemented in this class.
    """
    mfg_test = ValiantMFGTest('http://10.10.10.1:80/jsonrpc', print_payloads=True)
    print(mfg_test.wifi_set_antenna(Antenna.INTERNAL))
    print(mfg_test.wifi_get_ap('GoogleGuest'))
    print(mfg_test.wifi_set_antenna(Antenna.EXTERNAL))
    print(mfg_test.wifi_get_ap('GoogleGuest'))
    print(mfg_test.ble_scan())
    print(mfg_test.wifi_set_antenna(Antenna.INTERNAL))
    print(mfg_test.ble_scan())
    print(mfg_test.ble_find('e8:84:a5:04:eb:1e'))
