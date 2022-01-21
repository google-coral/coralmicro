#!/usr/bin/python3

from valiant_mfg_test import ValiantMFGTest

if __name__ == '__main__':
    """
    Sample runner for the RPCs implemented in this class.
    """
    mfg_test = ValiantMFGTest('http://10.10.10.1:80/jsonrpc', print_payloads=True)
    print(mfg_test.eth_get_ip())

    print(mfg_test.eth_write_phy(31, 0x0))
    print(mfg_test.eth_write_phy(9, 0x0))
    print(mfg_test.eth_write_phy(4, 0x61))
    print(mfg_test.eth_write_phy(25, 0x843))
    print(mfg_test.eth_write_phy(0, 0x9200))
