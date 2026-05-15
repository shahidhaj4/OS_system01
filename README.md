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

### Milestone 4 Format

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

## 📝 Notes
- Uses adjacency list
- Memory handled properly
- Max nodes: 15
- Milestone 4 uses fork() to create one child process for each traveler
- The parent process manages the GUI and calculates the paths
- Each child process prints [PID] started and then sleeps
- Travelers move simultaneously and are displayed with different colors