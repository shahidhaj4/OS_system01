/* ... includes ... */
#include "sync.h" 

/* ... (rest of code) ... */

static void run_autonomous_child(
    int traveler_idx,
    int src,
    int dest,
    int write_fd,
    const GraphData* data
) {
    int dist[MAX_NODES];
    int prev[MAX_NODES];
    int path[MAX_NODES];
    int path_len = 0;

    dijkstra(&g_graph, src, dist, prev);

    if (!extractPath(prev, src, dest, path, &path_len)) {
        IPCOverPipeMessage msg;
        msg.traveler_index = traveler_idx;
        msg.arrived_node = src;
        msg.next_node = -1;
        msg.is_finished = 1;
        write(write_fd, &msg, sizeof(msg));
        close(write_fd);
        _exit(0);
    }

    reversePath(path, path_len);

    for (int i = 0; i < path_len; i++) {
        /* Lock the node to ensure mutual exclusion */
        lock_node(path[i]);

        IPCOverPipeMessage msg;
        msg.traveler_index = traveler_idx;
        msg.arrived_node = path[i];

        if (i == path_len - 1) {
            msg.next_node = -1;
            msg.is_finished = 1;
            write(write_fd, &msg, sizeof(msg));
            
            /* Stay in the destination node for 1 second */
            sleep(1);
            unlock_node(path[i]);
            break;
        }

        msg.next_node = path[i + 1];
        msg.is_finished = 0;
        write(write_fd, &msg, sizeof(msg));

        int weight = get_edge_weight_from_data(data, path[i], path[i + 1]);

        /* Simulated movement delay */
        struct timespec delay;
        delay.tv_sec = 0;
        delay.tv_nsec = weight * 300000000L;
        nanosleep(&delay, NULL);

        /* Wait in the node for 1 second */
        sleep(1);

        /* Release the node lock */
        unlock_node(path[i]);
    }

    close(write_fd);
    _exit(0);
}
/* ... (rest of functions) ... */