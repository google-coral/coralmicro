#!/usr/bin/python3

from valiant_mfg_test import ValiantMFGTest

if __name__ == '__main__':
    """
    Sample runner for the RPCs implemented in this class.
    """
    mfg_test = ValiantMFGTest('http://10.10.10.1:80/jsonrpc', print_payloads=True)
    print(mfg_test.eth_get_ip())
