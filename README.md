# OS_system01

## 📌 Project Description
This project simulates a train network using graph algorithms, where each node represents a station and edges represent connections between stations.
It finds the shortest path between stations using Dijkstra’s algorithm and visualizes the network using a GUI (Raylib).

---

## 🚀 How to Run

### Milestone 1 (Console)
make milestone1

./dijkstra graph.txt

### Milestone 2 (GUI)
make milestone2

./sim graph.txt

### Milestone 3 (Animation)
make milestone3

./sim graph.txt

### Milestone 4 (Multiple Travelers + fork)
make milestone4

./sim graph.txt

### Milestone 5 (IPC)
make milestone5

./sim graph.txt

### Milestone 6 (Synchronization)
make milestone6

./sim graph.txt6

## 🧹 Clean Build Files
make clean

---

## 📂 Input Format

### Milestones 1–3 Format

Example:

0 3  
4 5  
0 1 10  
1 2 5  
2 3 7  
0 2 15  
1 3 12

- First line: source and destination
- Second line: number of nodes (N) and edges (M)
- Next lines: edges (src dst weight)

---

### Milestones 4–5 Format

Example:

5 7  
0 1 4  
0 2 2  
1 3 5  
2 1 1  
2 3 8  
3 4 2  
1 4 6  
2  
0 4  
2 3
### Milestones 6 Format
Example:
"""
5 6
0 2 1
1 2 1
3 2 1
2 4 2
0 4 5
1 4 5
3
0 4
1 4
3 4
"""
- First line: number of nodes (N) and edges (M)
- Next M lines: edges (src dst weight)
- Next line: number of travelers
- Next lines: traveler source and destination

## ⚙️ Milestones Overview

### Milestone 1
- Dijkstra algorithm
- Console output (path + total weight)
- Input validation

### Milestone 2
- GUI using Raylib
- Highlight shortest path
- Display nodes and edges

### Milestone 3
- Animated movement along the shortest path
- Step-based motion according to edge weights (300ms per step)
- 1-second delay at intermediate nodes
- Play/Stop control using SPACE key
- Arrival message at destination

### Milestone 4
- Multiple travelers moving at the same time
- Parent process calculates the shortest path for each traveler
- One child process is created for each traveler using fork()
- Each child process prints [PID] started and then sleeps
- Parent process manages the GUI
- Each traveler is displayed with a different color
- Parent sends a signal when a traveler reaches the destination
- Parent waits for all child processes before exiting

### Milestone 5
- IPC communication between child processes and the parent process.
- Each child process calculates its own Dijkstra shortest path independently.
- Each child sends its current node and next node to the parent through a pipe.
- The parent process receives pipe messages, updates the GUI, and prints the log.
- Terminal logs are printed only by the parent process.
- The chosen IPC method is pipes.

### Milestone 6
- Node synchronization between multiple traveler processes.
- Each graph node has its own shared lock.
- Only one traveler can be inside the same node at the same time.
- If more than one traveler reaches the same node, the others wait outside the node.
- Each traveler stays inside the node for one full second before leaving.
- The GUI clearly shows traveler states:
    - WAITING: traveler is waiting outside the node.
    - INSIDE_NODE: traveler is currently inside the node.
    - MOVING: traveler is moving between nodes.
    - FINISHED: traveler reached the destination.
- Synchronization is implemented using shared process mutexes.

## 🔗 IPC Choice - Milestone 5

For Milestone 5, we used pipes as the IPC mechanism.

Each traveler is represented by a child process.  
The child process calculates its own Dijkstra shortest path independently and sends updates to the parent process through a pipe.

Each message contains:
- Traveler index
- Arrived node
- Next node
- Finished status

The parent process receives these messages, updates the GUI, and prints the required terminal log.

Example log format:

[PID=1021] arrived at node 0 | next node: 2  
[PID=1021] arrived at node 4 | DESTINATION  
[PID=1021] finished

## 🔒 Synchronization Choice - Milestone 6

For Milestone 6, we used process-shared mutexes for node synchronization.

Each node has one mutex lock stored in shared memory.  
The shared memory is created using `mmap`, so all child processes can access the same locks after `fork()`.

When a traveler reaches a node:
1. The child sends a `WAITING` message to the parent.
2. The child tries to lock the node mutex.
3. After the lock is acquired, the child sends an `INSIDE_NODE` message.
4. The child stays inside the node for one full second.
5. The child unlocks the node and continues moving.

This guarantees that no more than one traveler can be inside the same node at the same time.
## 👥 Task Distribution

### Milestone 1 & 2

#### Shahid Hassan Haj – Main & Output
- Integrated modules, handled edge cases, and printed results.
- Designed the GUI layout.

#### Miral Agha – GUI Support
- Helped with GUI structure and visual organization.

#### Ghada Bader – Path Handling
- Worked on shortest path handling and Dijkstra output.

#### Shahd Julani – Drawing & Visualization
- Implemented graph drawing and visualization updates.

---

### Milestone 3

#### Shahid Hassan Haj – Main & Control
**Files:** `Main.c`  
Added Play/Stop control, managed animation state, and connected the animation to the existing GUI.

#### Miral Agha – Animation Logic
**Files:** `animation.c`, `animation.h`  
Implemented movement logic, including 300ms jump timing and 1-second waiting at nodes.

#### Ghada Bader – Path Handling
**Files:** `path.c`, `dijkstra.c`  
Extracted the shortest path from Dijkstra and converted it into a sequence of nodes/edges.

#### Shahd Julani – Drawing & Visualization
**Files:** `drawing.c`, `drawing.h`  
Drew the moving entity, updated the display during animation, and showed the arrival message.

---

### Milestone 4

#### Shahid Hassan Haj – Main & Control
**Files:** `Main.c`  
Handled the parent process, created child processes using `fork()`, and managed process termination with signals and `waitpid()`.

#### Miral Agha – Animation Logic
**Files:** `animation.c`, `animation.h`  
Implemented the movement logic for multiple travelers moving at the same time.

#### Ghada Bader – Path Handling
**Files:** `file_reader.c`, `file_reader.h`, `dijkstra.c`  
Updated the input reading and handled path calculation for each traveler.

#### Shahd Julani – Drawing & Visualization
**Files:** `drawing.c`, `drawing.h`  
Displayed the travelers on the graph with different colors and arrival messages.

### Milestone 5

#### Shahid Hassan Haj – Main, Build & Documentation
**Files:** `Main.c`, `Makefile`, `CMakeLists.txt`, `README.md`  
Connected the project modules for Milestone 5, added IPC integration through pipes, updated the build commands, and documented the IPC implementation.

#### Miral Agha – IPC & Animation Logic
**Files:** `animation.c`, `animation.h`  
Implemented child process behavior, pipe messages, traveler state updates, and movement timing for autonomous travelers.

#### Ghada Bader – File Reading & Path Handling
**Files:** `file_reader.c`, `file_reader.h`, `dijkstra.c`, `dijkstra.h`  
Updated input reading for multiple travelers and provided Dijkstra path extraction functions used by each child process.

#### Shahd Julani – Drawing & Visualization
**Files:** `drawing.c`, `drawing.h`, `graph.txt`  
Displayed travelers with different colors, drew the graph, and prepared the test input file for Milestone 5.

### Milestone 6

#### Ghadabader – Synchronization and Node Locks
**Files:** `sync.c`, `sync.h`, `animation.c`  
Implemented the synchronization mechanism for graph nodes using process-shared mutex locks.  
Each node has its own lock, so only one traveler can enter and stay inside the node for one full second before another traveler can enter.

#### Miral Agha – Traveler States and Movement Logic
**Files:** `animation.c`, `animation.h`  
Added clear traveler states: `MOVING`, `WAITING`, `INSIDE_NODE`, and `FINISHED`.  
Updated the traveler process logic so each traveler reports its state to the parent process through IPC while moving, waiting, entering a node, or finishing the route.

#### Shahd Julani – GUI and Waiting Visualization
**Files:** `drawing.c`, `drawing.h`, `Dijkstra_draw.c`, `Dijkstra_draw.h`  
Updated the GUI to clearly display traveler states using different colors and visual indicators.  
Waiting travelers are shown outside the node with a `WAIT` label and are slightly shifted so multiple waiting travelers can be seen clearly.

#### Shahid Hassan Haj – Integration, Build, and Main Flow
**Files:** `Main.c`, `Makefile`, `README.md`, `graph.txt`  
Connected the synchronization, IPC updates, traveler states, and GUI drawing into the main simulation flow.  
Updated the build commands for `make milestone6`, prepared the test input file, and documented the Milestone 6 synchronization mechanism in the README.
## 📝 Notes
- Uses adjacency list.
- Memory handled properly.
- Max nodes: 15.
- Milestone 4 uses `fork()` to create one child process for each traveler.
- In Milestone 4, the parent process calculates the paths and manages the GUI.
- Milestone 5 uses pipes for IPC.
- In Milestone 5, each child process calculates its own Dijkstra path independently.
- Child processes send node updates to the parent process through pipes.
- Only the parent process prints terminal logs.
- The parent process updates the GUI according to the messages received from the children.
- Travelers move simultaneously and are displayed with different colors.
- Milestone 6 uses shared mutex locks for node synchronization.
- Each node has one process-shared mutex.
- The mutexes are created in shared memory using `mmap`.
- The parent initializes the node locks before creating child processes.
- Child processes call `lock_node()` before entering a node and `unlock_node()` after staying inside for one second.
- The GUI displays waiting travelers outside the node and shows which traveler is inside.
