#!/usr/bin/python3
from valiant_mfg_test import ValiantMFGTest

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Valiant IPerf')
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument('-s', action='store_true')
    mode_group.add_argument('-c', type=str, help='IP address of iPerf server')
    parser.add_argument('--device_ip_address', type=str, required=False, default='10.10.10.1')
    args = parser.parse_args()

    mfg_test = ValiantMFGTest(f'http://{args.device_ip_address}:80/jsonrpc', print_payloads=True)
    if args.s:
        print(f'Device ({args.device_ip_address}) acting as iPerf server')
        response = mfg_test.iperf_start(is_server=True)
        print(response)
    else:  # args.c
        print('Device ({args.device_ip_address}) acting as iPerf client, connecting to {args.c}')
        response = mfg_test.iperf_start(is_server=False, server_ip_address=args.c)
        print(response)

    # response = mfg_test.iperf_stop()
    # print(response)
