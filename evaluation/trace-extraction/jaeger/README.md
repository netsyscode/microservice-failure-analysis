# Jaeger Trace Query Tool

This tool provides a command-line interface to query Jaeger for traces or list available services. It supports querying by service name within a specified time range, allowing for both absolute and relative time expressions. This document provides guidance on how to set up and use the tool effectively.

## Prerequisites

- Python 3.6 or newer
- gRPC Python package
- Protobuf package for Python (submodule in `deps/jaeger/jaeger-idl`, check [here](../../../deps/jaeger/README.md))

Before running the script, ensure you have the necessary Python packages installed:

```sh
pip install grpcio
pip install protobuf==3.20.*  # Important: this is due to version limits cause by default generation method of jaeger-idl
```

## Usage

The tool supports several command-line arguments to specify the Jaeger server details, the service name to query, the time range for the query, and the maximum number of results to return.

### Command-Line Arguments

- `--jaeger-address`: **Required.** The address of the Jaeger gRPC server (e.g., 'localhost').
- `--jaeger-port`: **Required.** The port on which the Jaeger gRPC server is running (e.g., 14250).
- `--service-name`: The name of the service to query traces for. If not specified, the tool lists available services.
- `--max-results`: The maximum number of traces to return. Only applicable if `--service-name` is specified.
- `--start-time`: The start time for the trace query. Can be an absolute datetime in 'YYYY-MM-DD HH:MM:SS' format or a relative time expression like '1h ago'. Required if `--service-name` is specified, unless using `--end-time` for a recent period query.
- `--end-time`: The end time for the trace query. Can be an absolute datetime in 'YYYY-MM-DD HH:MM:SS' format or a relative time expression like '1h ago'. Optional and defaults to the current time if `--start-time` is provided.
- `--output-file`: Specifies the file path where the query results will be written. If not specified, the default is `jaeger_query_results.json`. This option allows you to direct the output of the script to a file of your choosing, making it easier to save, share, or process the results of your Jaeger queries. This option only works when `--service-name` is specified.

### Examples

To query traces for a specific service from 1 hour ago to now and write the results to a custom file:

```bash
python jaeger_query.py --jaeger-address localhost --jaeger-port 16685 --service-name my-service --start-time "1h ago" --output-file custom_output.txt
```

List available services:

```bash
python jaeger_query.py --jaeger-address localhost --jaeger-port 16685
```

## Relative Time Expressions

The tool supports the following units for relative time expressions:
- `h`: Hours
- `d`: Days
- `m`: Minutes

Example: "1h ago", "2d ago", "30m ago"

## Notes

- Ensure that the Jaeger server's gRPC endpoint is accessible from the location where you're running the script.
- The tool uses insecure gRPC channels for simplicity. Consider using secure channels for production environments.