---
summary: Overview of the ROS package layout and inter-node communication
read_when: You need to understand how the nodes talk to each other
---

# Architecture

This project contains four ROS packages:

- **interfaces** — shared message definitions
- **demo_cpp_node** — C++17 publisher/subscriber
- **demo_py_node** — Python publisher/subscriber
- **demo_bringup** — launch files only, no business logic

## Message Flow

```
demo_cpp_node  --(/cpp_to_py)-->  demo_py_node
demo_py_node   --(/py_to_cpp)-->  demo_cpp_node
```

Both nodes exchange `interfaces/msg/Greeting` messages.
