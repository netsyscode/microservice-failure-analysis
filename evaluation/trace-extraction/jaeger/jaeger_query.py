import sys

sys.path.append("../../../deps/jaeger/jaeger-idl/proto-gen-python/")

import argparse
import grpc
from datetime import datetime
import query_pb2 as jaeger_pb2
import query_pb2_grpc as jaeger_pb2_grpc
from google.protobuf.timestamp_pb2 import Timestamp
from google.protobuf.json_format import MessageToDict
from dateutil.relativedelta import relativedelta
import re
import base64
import json

def b64_to_hex(b64_str):
    decoded_bytes = base64.b64decode(b64_str)
    hex_str = "0x" + decoded_bytes.hex()
    return hex_str

def parse_timestamp(time_str):
    """Parse a datetime string or a relative time expression to a protobuf Timestamp."""
    if ' ago' in time_str:
        return parse_relative_timestamp(time_str)
    else:
        return parse_absolute_timestamp(time_str)

def parse_absolute_timestamp(date_str):
    """Parse an absolute datetime string to a protobuf Timestamp."""
    try:
        dt = datetime.strptime(date_str, "%Y-%m-%d %H:%M:%S")
        ts = Timestamp()
        ts.FromDatetime(dt)
        return ts
    except ValueError as e:
        sys.exit(f"Error: Invalid date format: {e}")

def parse_relative_timestamp(relative_str):
    """Parse relative time expressions like '1h ago' into a protobuf Timestamp."""
    now = datetime.now()
    match = re.match(r'(\d+)([hmd]) ago', relative_str)
    if not match:
        sys.exit(f"Error: Invalid relative time format: '{relative_str}'.")

    amount, unit = match.groups()
    amount = int(amount)

    if unit == 'h':
        delta = relativedelta(hours=-amount)
    elif unit == 'd':
        delta = relativedelta(days=-amount)
    elif unit == 'm':
        delta = relativedelta(minutes=-amount)
    else:
        sys.exit(f"Error: Unsupported time unit in relative time: '{unit}'.")

    target_time = now + delta
    ts = Timestamp()
    ts.FromDatetime(target_time)
    return ts

def create_trace_query_parameters(args):
    """Create TraceQueryParameters from command line arguments."""
    query_params = jaeger_pb2.TraceQueryParameters(
        service_name=args.service_name,
        search_depth=int(args.max_results) if args.max_results else 100
    )

    if args.start_time:
        start_timestamp = parse_timestamp(args.start_time)
        query_params.start_time_min.CopyFrom(start_timestamp)
        
        if args.end_time:
            end_timestamp = parse_timestamp(args.end_time)
        else:
            now = datetime.now()
            end_timestamp = Timestamp()
            end_timestamp.FromDatetime(now)
        
        if start_timestamp.ToSeconds() >= end_timestamp.ToSeconds():
            sys.exit("Error: Start time must be earlier than end time.") 

        query_params.start_time_max.CopyFrom(end_timestamp)

    return query_params

def process_trace(trace):
    """Process trace to convert IDs and serialize to JSON."""
    generated_trace = []
    for span in trace.spans:
        span_dict = MessageToDict(span, preserving_proto_field_name=True)

        span_dict['trace_id'] = b64_to_hex(span_dict['trace_id'])
        span_dict['span_id'] = b64_to_hex(span_dict['span_id'])

        if "references" in span_dict:
            try:
                if len(span_dict['references']) != 1:
                    raise ValueError("Expected exactly one reference in 'span_dict'.")
            except ValueError as e:
                print(span_dict['references'])
                print(e)

            span_dict['references'][0]['trace_id'] = b64_to_hex(span_dict['references'][0]['trace_id'])
            span_dict['references'][0]['span_id'] = b64_to_hex(span_dict['references'][0]['span_id'])
        
        generated_trace.append(span_dict)

    # Convert the entire trace message to JSON
    return generated_trace

def query_jaeger(args):
    """Query Jaeger server for traces and save results in JSON format."""
    with grpc.insecure_channel(f'{args.jaeger_address}:{args.jaeger_port}') as channel:
        stub = jaeger_pb2_grpc.QueryServiceStub(channel)
        results = []

        if args.service_name:
            query_params = create_trace_query_parameters(args)
            request = jaeger_pb2.FindTracesRequest(query=query_params)
            responses = stub.FindTraces(request)
            
            # with open(args.output_file, 'w') as f:
            #     for response in responses:
            #         f.write(str(response) + '\n')

            for response in responses:
                processed_trace = process_trace(response)
                results.append(processed_trace)
            
            with open(args.output_file, "w") as file:
                json.dump(results, file, indent=4)            
            
            print(f"{len(results)} trace results written to {args.output_file}")
        else:
            response = stub.GetServices(jaeger_pb2.GetServicesRequest())
            print("Available services:", ', '.join(response.services))

def main():
    parser = argparse.ArgumentParser(description="Query Jaeger for traces or list services in a user-friendly manner.")
    parser.add_argument("--jaeger-address", required=True, help="The gRPC address of the Jaeger server (e.g., 'localhost').")
    parser.add_argument("--jaeger-port", required=True, type=int, help="The gRPC port of the Jaeger server (e.g., 14250).")
    parser.add_argument("--service-name", help="The name of the service to query traces for. Without this, the script lists available services.")
    parser.add_argument("--max-results", type=int, help="The maximum number of traces to return. Only applicable if --service-name is specified.")
    parser.add_argument("--start-time", required="--service-name" in sys.argv, help="The start time for the trace query in 'YYYY-MM-DD HH:MM:SS' format or a relative time expression like '1h ago'. Required if --service-name is specified.")
    parser.add_argument("--end-time", help="The end time for the trace query in 'YYYY-MM-DD HH:MM:SS' format or a relative time expression like '1h ago'. Optional, defaults to the current time if --start-time is provided.")
    parser.add_argument("--output-file", default="jaeger_query_results.json", help="File to write query results to. Defaults to 'jaeger_query_results.txt'.")

    args = parser.parse_args()

    # Validate the combination of start-time and end-time
    if args.end_time and not args.start_time:
        parser.error("The --end-time option is specified without --start-time. Please provide a start time.")

    query_jaeger(args)

if __name__ == "__main__":
    main()
