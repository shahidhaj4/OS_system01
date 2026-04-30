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

---

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

---

## 👥 Task Distribution – Milestone 1 & 2

### Shahid Hassan Haj – Main & Output
- Integrated modules, handled edge cases, printed results
- Designed the GUI layout

### Miral Agha – File Reader
- Parsed input file, validated data, managed memory

### Ghada Bader – Graph Structure
- Built adjacency list, managed edges and memory

### Shahd Julani – Dijkstra Algorithm
- Implemented shortest path logic
- Contributed to drawing (visualization)

---

## 📝 Notes
- Uses adjacency list
- Memory handled properly
- Max nodes: 15