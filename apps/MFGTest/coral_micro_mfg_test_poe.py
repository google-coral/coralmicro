#!/usr/bin/python3

from coral_micro_mfg_test import CoralMicroMFGTest

if __name__ == '__main__':
    """
    Sample runner for the RPCs implemented in this class.
    """
    import argparse
    parser = argparse.ArgumentParser(description='Dev Board Micro MFGTest')
    parser.add_argument('--ip_address', type=str, required=False, default='10.10.10.1')
    args = parser.parse_args()
    mfg_test = CoralMicroMFGTest(f'http://{args.ip_address}:80/jsonrpc', print_payloads=True)
    print(mfg_test.eth_get_ip())

    print(mfg_test.eth_write_phy(31, 0x0))
    print(mfg_test.eth_write_phy(9, 0x0))
    print(mfg_test.eth_write_phy(4, 0x61))
    print(mfg_test.eth_write_phy(25, 0x843))
    print(mfg_test.eth_write_phy(0, 0x9200))
