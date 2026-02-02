Assignment: PA02 - Analysis of Network I/O Primitives using perf
Roll No: MT25026


Purpose:-
    This assignment implements and evaluates a TCP-based client-server application to study the impact of data-copy mechanisms and concurrency on network performance.
    Three server implementations are compared:
        -> A1 - Two-copy (send())
        -> A2 - One-copy (sendmsg() with pre-registered buffer)
        -> A3 - Zero-copy (sendmsg() with MSG_ZEROCOPY)
    Performance is analyzed using:
    	-> Throughput
    	-> Latency
    	-> Cache behavior
    	-> CPU efficiency (cycles per byte)
    Client and server run in separate Linux network namespaces to emulate a realistic distributed setup.
    

System Setup:-
    -> Operating System: Ubuntu Linux
    -> Architecture: x86_64
    -> Transport Protocol: TCP
    -> Topology: Client-Server via Linux Network Namespaces
    -> Measurement Duration: 10 seconds per experiment

Common Header File:-
    -> MT25026_common.h
    All implementations (A1, A2, and A3) share this common header file, which defines constants and data structures used consistently across servers and clients


Source Files:-
    Server Implementations
    	-> MT25026_Part_A1_Server.c - TCP server using send() (two-copy)
    	-> MT25026_Part_A2_Server.c - TCP server using sendmsg() (one-copy)
    	-> MT25026_Part_A3_Server.c - TCP server using sendmsg() + MSG_ZEROCOPY (zero-copy)
    	
    Client Implementations
    	-> MT25026_Part_A1_Client_TP.c
    	-> MT25026_Part_A2_Client_TP.c
    	-> MT25026_Part_A3_Client_TP.c
    	(Throughput measurement clients - multi-client, time-based)
    	
        -> MT25026_Part_A1_Client_LAT.c
    	-> MT25026_Part_A2_Client_LAT.c
    	-> MT25026_Part_A3_Client_LAT.c
    	(Latency measurement clients)
    	
        -> MT25026_Part_A1_Client_PERF.c
	    -> MT25026_Part_A2_Client_PERF.c
	    -> MT25026_Part_A3_Client_PERF.c
	    (Lightweight clients for server-side perf analysis)
    
    Automation & Analysis
    	-> MT25026_Part_C_Run_Experiments.sh (Fully automated experiment runner - namespaces + measurements)
    	-> MT25026_Part_D_Plots.py (Matplotlib-based plotting script - hardcoded values)
    	-> Makefile - Builds all servers and clients


Compilation:-
    Compile all programs using:
        make

    This generates executables:
        -> a1_server, a2_server, a3_server
        -> a1_client_tp, a2_client_tp, a3_client_tp
        -> a1_client_lat, a2_client_lat, a3_client_lat
        -> a1_client_perf, a2_client_perf, a3_client_perf


Part B - Program Execution (Manual):-
    1. Create network namespaces:
        sudo ip netns add ns_server
        sudo ip netns add ns_client

    2. Create and configure virtual Ethernet pair:
        sudo ip link add veth_server type veth peer name veth_client
        sudo ip link set veth_server netns ns_server
        sudo ip link set veth_client netns ns_client

    3. Assign IP addresses:
        sudo ip netns exec ns_server ip addr add 10.0.0.1/24 dev veth_server
        sudo ip netns exec ns_client ip addr add 10.0.0.2/24 dev veth_client

    4. Bring network interfaces up:
        sudo ip netns exec ns_server ip link set veth_server up
        sudo ip netns exec ns_client ip link set veth_client up
        sudo ip netns exec ns_server ip link set lo up
        sudo ip netns exec ns_client ip link set lo up

    5. Run server in namespace:
        sudo ip netns exec ns_server ./server <port> <msg_size> <thread_count>

        Example:
            sudo ip netns exec ns_server ./a1_server 9000 64 4
            This launches the server on port 9000, using a message size of 64 bytes and allowing up to 4 concurrent client connections

    6. Run client in namespace:
        sudo ip netns exec ns_client ./client 10.0.0.1 <port> <msg_size> <duration>

        Example:
            sudo ip netns exec ns_client ./a1_client_tp 10.0.0.1 9000 64 10
            This connects to the server at 10.0.0.1:9000, receives 64-byte messages for 10 seconds
        
        To generate concurrent load, multiple clients were launched in parallel:
            for i in {1..4}; do
                sudo ip netns exec ns_client ./client 10.0.0.1 9000 64 10 &
            done

    7. Using perf for profiling:
        sudo perf stat -e cycles,instructions,cache-misses,LLC-load-misses,context-switches \
            sudo ip netns exec ns_server ./server <port> <msg_size> <thread_count>


Part C - Automated Measurement:-
    Purpose
        Automates all experiments across:
            -> Message sizes: 64, 256, 1024, 4096 bytes
            -> Thread counts: 1, 2, 4, 8
            -> Implementations: A1, A2, A3
        Measures:
            -> Throughput (Gbps)
            -> Latency (μs)
            -> CPU cycles
            -> Cache misses
            -> Context switches

    Command
        sudo ./MT25026_Part_C_Run_Experiments.sh

        What thes script does:
            1. Creates client and server network namespaces
            2. Connects namespaces using a veth pair
            3. Compiles all binaries
            4. Run server and clients in isolated namespaces
            5. Measures:
                -> Throughput & latency from client output
                -> Server-side hardware counters using perf
            6. Stores results in CSV files
            7. Cleans up namespaces and temporary files
    
        Output:
            -> MT25026_Part_C_throughput.csv
            -> MT25026_Part_C_latency.csv
            -> MT25026_Part_C_perf.csv


Part D - Plotting & Analysis:-
    Purpose
        Visualizes performance trends using hardcoded values derived from experimental CSV outputs
        Generated plots:
            1. Throughput vs Message Size
            2. Latency vs Thread Count
            3. Cache Misses vs Message Size
            4. CPU Cycles per Byte vs Message Size

    Command
        python3 MT25026_Part_D_Plots.py

        Plots include:
            -> Clearly labeled axes
            -> Legends for A1, A2, A3
            -> System configuration annotation on each plot


Measurement Tools Used:-
    -> ip netns - Network namespace isolation
    -> perf stat - Hardware performance counters
    -> pthread - Multithreaded server handling
    -> matplotlib - Visualization


Units of Measurement:-
    -> Throughput - Gbps
    -> Latency - microseconds (μs)
    -> Cache Misses - Count
    -> CPU Cycles - Cycles
    -> CPU Efficiency - Cycles per Byte


Notes:-
    -> Duration applies to clients, not servers
    -> Server runs continuously until interrupted
    -> perf is always attached to the server process
    -> Zero-copy (MSG_ZEROCOPY) requires kernel support
