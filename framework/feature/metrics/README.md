# Tags

| Category               | Attributes                                      | Status | Collected location |
|------------------------|-------------------------------------------------|--------|------------|
| **Entity Identifier**  | Host IPs                                        | ✔      |   user       |
|                        | Service IDs                                     | ✘      |     user       |
|                        | Container IDs                                   | ✔      |    user        |
|                        | Process IDs                                     | ✔      |     kernel       |
|                        | Component IDs                                   | ✘      |   kernel         |
|                        | Kubernetes Labels                               | ✘      |   user         |
| **Service Annotations**| Service Type: Frontend, Backend, Cache, ...     | ✘      |   user         |
|                        | Environment: Dev, QA, Production                | ✘      |  user          |
|                        | Release: Stable, Canary                         | ✘      |  user          |
|                        | Versions                                        | ✘      |   user         |
| **Request Features**   | Invocation Path                                 | ✘      |   kernel         |
|                        | Flow tuple                                      | ✔      |  kernel          |
|                        | Client IDs                                      | ✘      |    kernel        |
|                        | Operation Names                                 | ✘      |   unknown         |
|                        | API Endpoints                                   | ✘      |    unknown        |
| **Monitoring Attributes** | Observation Points                           | ✘      |    unknown        |
|                        | Monitoring Attributes                           | ✘      |   unknown         |



# metric

| Category               | Attributes                                      | Status |
|------------------------|-------------------------------------------------|--------|
| **Execution**          | Execution Duration                              | ✘      |
|                        | CPU Utilization                                 | ✘      |
|                        | Instructions                                    | ✔      |
| **Caches**             | LLC Misses                                      | ✔      |
|                        | LLC Hit Rate                                    | ✘      |
|                        | LLC Occupancy                                   | ✘      |
| **Memory**             | Memory Usage                                    | ✔      |
|                        | Memory Bandwidth                                | ✘      |
|                        | Page Faults                                     | ✔      |
| **Network**            | TX Bytes                                        | ✔      |
|                        | RX Bytes                                        | ✔      |
|                        | Dropped Packets                                 | ✔      |
|                        | RTT                                             | ✔      |
|                        | Retransmissions                                 | ✔      |
| **File System**        | File System Usage                               | ✘      |
|                        | File Write Speed                                | ✘      |
|                        | File Read Speed                                 | ✘      |
| **Others**             | Context Switches                                | ✘      |
|                        | Software Interrupts                             | ✘      |
|                        | Lock Contentions                                | ✘      |
|                        | Application Metrics                             | ✘      |