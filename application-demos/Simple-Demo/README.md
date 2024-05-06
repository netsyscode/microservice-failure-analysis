# Simple Microservices Example

This project demonstrates a basic microservices architecture where services built with Flask communicate with each other via environment variables.

## Architecture

This project includes three services:

1. **Service1** - Acts as the first service in the chain, calling Service2.
2. **Service2** - Receives requests from Service1 and calls Service3.
3. **Service3** - Receives requests from Service2 and as the final service in the chain, returns a static text response.

Each service is containerized and deployed on a Kubernetes cluster.

### Environment Variables

Note: all of the environment variables are automatically set by `simple-demo.yaml`.

- `PORT`: The port number on which the service listens.
- `NEXT_SERVICE_URL`: The URL of the next service in the chain; set to "none" for the final service in the chain.
- `SERVICE_NAME`: The service name, which will be printed in the response.

## Technologies Used

- **Flask**: For creating HTTP servers.
- **Requests**: For making HTTP requests.
- **Docker**: For containerizing services.
- **Kubernetes**: For deploying and managing containers.

## Quick Start

### Prerequisites

- Kubernetes cluster installed
- Docker installed

### Build and Deploy

1. **Build Docker Images**
   Place the Dockerfile in the root directory of each service and build the image:

   ```bash
   docker build -t <your-dockerhub-username>/simple-demo .
   docker push <your-dockerhub-username>/simple-demo
   ```

   If you build your own image, then you have to modify the docker image url in `simple-demo.yaml`.

2. **Deploy to Kubernetes**
   Deploy each service to your Kubernetes cluster with the following command:

   ```bash
   kubectl apply -f deployment.yaml
   ```

3. **Verify Deployment**
   Ensure all services are correctly deployed and running:

   ```bash
   kubectl get all
   ```

## Testing the Service

To test the deployed service, you can use the `test.sh` script. This script will attempt to retrieve the service URL and then perform a series of requests to check the service response.

### Usage of `test.sh`

1. **Set executable permission**:
   ```bash
   chmod +x test.sh
   ```

2. **Run the script**:
   You can specify the number of requests to send as a parameter. If no parameter is specified, the script defaults to 100 requests.

   ```bash
   ./test.sh 50  # Runs the test with 50 requests
   ```

   If no parameter is provided, the script will run with the default 100 requests:
   ```bash
   ./test.sh
   ```
