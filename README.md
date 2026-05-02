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

## 🧹 Clean Build Files
make clean

---

## 📂 Input Format
Example (graph.txt):

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

## 👥 Task Distribution – Milestone 1 & 2

### Shahid Hassan Haj – Main & Output
- Integrated modules, handled edge cases, printed results
- Designed the GUI layout
- Implemented animation control logic (Play/Stop)
- Integrated animation with Dijkstra results and GUI
- Managed entity movement across the path

#### Miral Agha – Animation Logic
- Implemented movement logic (step-based animation)
- Handled 300ms step timing
- Implemented 1-second delay at nodes

#### Ghada Bader – Path Handling
- Extracted shortest path from Dijkstra output
- Converted path into sequence of nodes/edges
- Managed transitions between nodes

#### Shahd Julani – Drawing & Visualization
- Implemented moving entity rendering
- Updated visuals during animation
- Displayed arrival message at destination



## 📝 Notes
- Uses adjacency list
- Memory handled properly
- Max nodes: 15