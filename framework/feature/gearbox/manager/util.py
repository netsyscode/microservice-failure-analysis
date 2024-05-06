import socket
import argparse
import threading
import hashlib
from structs import Point


def parse_arg():
    parser = argparse.ArgumentParser(description='Gearbox manager server.')
    parser.add_argument('--host', type=str, required=True, help='Host IP address of this gearbox manager')
    parser.add_argument('--port', type=int, required=True, help='Host oort number of this gearbox manager')

    args = parser.parse_args()
    return args

def send_udp_msg(udp_socket, data, server_address, retries=10):
    """Sends data over UDP with retries on failure."""
    for _ in range(retries):
        try:
            udp_socket.sendto(data, server_address)
            # Attempt to receive the acknowledgment
            _, _ = udp_socket.recvfrom(1024)  # We don't use these values
            return  # Success, exit the function
        except socket.timeout:
            continue  # Retry if timed out
    raise ConnectionError("Failed to send data after several retries.")

def send_pathid_to_aggregation(point, path_id, target_addr, target_port):
    """Creates a UDP socket, sends point and path_id to target aggregation server."""
    aggr_server = (target_addr, target_port)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.settimeout(2)  # Set timeout for the socket

        # Prepare data
        point_encoded = point.encode()
        path_id_encoded = path_id.to_bytes(32, 'little')

        # Send data
        send_udp_msg(udp_socket, point_encoded, aggr_server)
        send_udp_msg(udp_socket, path_id_encoded, aggr_server)

def send_pathid_to_storage(path_id, target_addr, target_port):
    """Creates a UDP socket, sends path_id to target storage server."""
    storage_server = (target_addr, target_port)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as udp_socket:
        udp_socket.settimeout(2)  # Set timeout for the socket

        # Prepare data
        path_id_encoded = path_id.to_bytes(32, 'little')

        # Send data
        send_udp_msg(udp_socket, path_id_encoded, storage_server)
