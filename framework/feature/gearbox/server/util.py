import argparse
import json

from config import parse_config

def parse_args():
    parser = argparse.ArgumentParser(description='Gearbox manager server.')
    parser.add_argument('-f', type=str, required=True, help='Config file of gearbox.')

    args = parser.parse_args()
    gearbox_config = parse_config(args.f)
    return gearbox_config

def save_data(data, path):
    """Append data to a JSON file."""
    with open(path, 'a') as file:
        json.dump(data, file)
        file.write('\n')
