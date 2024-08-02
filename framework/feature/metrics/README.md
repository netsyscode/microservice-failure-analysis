# Tags

| Category               | Attributes                                      | Status |
|------------------------|-------------------------------------------------|--------|
| **Entity Identifier**  | Host IPs                                        | ✔      |
|                        | Service IDs                                     | ✘      |
|                        | Container IDs                                   | ✔      |
|                        | Process IDs                                     | ✔      |
|                        | Component IDs                                   | ✘      |
|                        | Kubernetes Labels                               | ✘      |
| **Service Annotations**| Service Type: Frontend, Backend, Cache, ...     | ✘      |
|                        | Environment: Dev, QA, Production                | ✘      |
|                        | Release: Stable, Canary                         | ✘      |
|                        | Versions                                        | ✘      |
| **Request Features**   | Invocation Path                                 | ✘      |
|                        | Flow tuple                                      | ✔      |
|                        | Client IDs                                      | ✘      |
|                        | Operation Names                                 | ✘      |
|                        | API Endpoints                                   | ✘      |
| **Monitoring Attributes** | Observation Points                           | ✘      |
|                        | Monitoring Attributes                           | ✘      |



# metric

| Category               | Attributes                                      | Status |
|------------------------|-------------------------------------------------|--------|
| **Execution**          | Execution Duration                              | ✘      |
|                        | CPU Utilization                                 | ✘      |
|                        | Instructions per Cycle                          | ✘      |
| **Caches**             | LLC Misses                                      | ✘      |
|                        | LLC Hit Rate                                    | ✘      |
|                        | LLC Occupancy                                   | ✘      |
| **Memory**             | Memory Usage                                    | ✘      |
|                        | Memory Bandwidth                                | ✘      |
|                        | Page Faults                                     | ✘      |
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