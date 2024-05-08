import argparse
import json

def parse_args():
    parser = argparse.ArgumentParser(description='Gearbox collector server.')
    parser.add_argument('--host', type=str, required=True, help='Host IP address of this gearbox collector')
    parser.add_argument('--port', type=int, required=True, help='Host oort number of this gearbox collector')

    args = parser.parse_args()
    return args

def save_data(data, path):
    """Append data to a JSON file."""
    with open(path, 'a') as file:
        json.dump(data, file)
        file.write('\n')