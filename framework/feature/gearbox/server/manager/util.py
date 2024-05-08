import socket
import argparse

def parse_args():
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

