import socket
import argparse
import json

from math import floor, ceil

from structs import AggrMetric
from config import parse_config

class Bucket(object):
    left = 0
    right = 0
    bucket_num = 0
    length = 0
    buckets = []
    total = 0

    def __init__(self, _left, _right, _bucket_num, _buckets, _total):
        self.left = _left
        self.right = _right
        self.bucket_num = _bucket_num
        self.length = (_right - _left) / _bucket_num
        self.buckets = _buckets
        self.total = _total

    def merge(self):
        tmp_i = 0
        min_l = self.right - self.left
        for i in range(0, len(self.buckets) - 1):
            tmp = floor((self.buckets[i + 1][1] - self.left) / self.length)
            board = self.length * tmp + self.left
            if self.buckets[i + 1][1] == board:
                board += self.length
            if self.buckets[i][0] < board:
                continue
            tmp_l = self.buckets[i + 1][1] - self.buckets[i][0]
            if tmp_l >= min_l:
                continue
            tmp_i = i
            min_l = tmp_l
        self.buckets[tmp_i][1] = self.buckets[tmp_i + 1][1]
        self.buckets[tmp_i][2] += self.buckets[tmp_i + 1][2]
        self.buckets.pop(tmp_i + 1)

    def add(self, metric):
        if metric < self.left or metric > self.right:
            print(f"Metric {metric} is not in range")
            return
        has_insert = False
        for i in range(0, len(self.buckets)):
            if self.buckets[i][0] > metric:
                self.buckets.insert(i, [metric, metric, 1])
                has_insert = True
                break
            if self.buckets[i][1] < metric:
                continue
            self.buckets[i][2] += 1
            has_insert = True
            break
        if not has_insert:
            self.buckets.append([metric, metric, 1])
        if len(self.buckets) > self.bucket_num:
            self.merge()
        self.total += 1

    def quatile(self, qt):
        if qt <= 0:
            return self.left
        if qt >= 1:
            return self.right
        cnt = 0

        value = ceil(self.total * qt)

        for i in range(0, len(self.buckets)):
            num = self.buckets[i][2]
            if value <= cnt + num:
                if num == 1:
                    return self.buckets[i][0]
                result = (value - cnt - 1)/(num - 1) * (self.buckets[i][1] - self.buckets[i][0]) + self.buckets[i][0]
                return result
            cnt += num
        return self.right

def do_aggregation(aggr_metric, invoke_id, path_id, metric):
    if path_id not in aggr_metric:
        aggr_metric[path_id] = {}

    if invoke_id not in aggr_metric[path_id]:
        aggr_metric[path_id][invoke_id] = [0, AggrMetric(), Bucket(0, 20000, 300, [], 0)]

    aggr_metric[path_id][invoke_id][1].srtt_us += metric.srtt_us
    aggr_metric[path_id][invoke_id][1].mdev_max_us += metric.mdev_max_us
    aggr_metric[path_id][invoke_id][1].rttvar_us += metric.rttvar_us
    aggr_metric[path_id][invoke_id][1].mdev_us += metric.mdev_us
    aggr_metric[path_id][invoke_id][1].snd_cwnd += metric.snd_cwnd
    aggr_metric[path_id][invoke_id][1].rtt_us += metric.rtt_us

    aggr_metric[path_id][invoke_id][1].bytes_sent = metric.bytes_sent
    aggr_metric[path_id][invoke_id][1].bytes_received = metric.bytes_received
    aggr_metric[path_id][invoke_id][1].bytes_acked = metric.bytes_acked
    aggr_metric[path_id][invoke_id][1].delivered = metric.delivered

    aggr_metric[path_id][invoke_id][2].add(metric.duration / 1000) # us

    aggr_metric[path_id][invoke_id][0] += 1

def retrieve_aggregated_metric(metric):
    ret_metric = AggrMetric()

    ret_metric.srtt_us = int(metric[1].srtt_us / metric[0])
    ret_metric.mdev_max_us = int(metric[1].mdev_max_us / metric[0])
    ret_metric.rttvar_us = int(metric[1].rttvar_us / metric[0])
    ret_metric.mdev_us = int(metric[1].mdev_us / metric[0])
    ret_metric.snd_cwnd = int(metric[1].snd_cwnd / metric[0])
    ret_metric.rtt_us = int(metric[1].rtt_us / metric[0])

    ret_metric.bytes_sent = metric[1].bytes_sent
    ret_metric.bytes_received = metric[1].bytes_received
    ret_metric.bytes_acked = metric[1].bytes_acked
    ret_metric.delivered = metric[1].delivered

    ret_metric.duration_45 = int(metric[2].quatile(0.45))
    ret_metric.duration_95 = int(metric[2].quatile(0.95))

    return ret_metric

def parse_args():
    parser = argparse.ArgumentParser(description='Gearbox manager server.')
    parser.add_argument('-f', type=str, required=True, help='Config file of gearbox.')
    parser.add_argument('-i', type=int, required=True, help='Index of this aggregator/manager/collector.')

    args = parser.parse_args()
    gearbox_config = parse_config(args.f)
    return gearbox_config, args.i

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

def save_data(data, path):
    """Append data to a JSON file."""
    with open(path, 'a') as file:
        json.dump(data, file)
        file.write('\n')
