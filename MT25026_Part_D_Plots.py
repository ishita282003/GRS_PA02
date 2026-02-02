import matplotlib.pyplot as plt

# ==========================
# System configuration text
# ==========================
SYS_CONFIG = (
    "System: Ubuntu Linux\n"
    "CPU: x86_64\n"
    "Transport: TCP\n"
    "Topology: Client–Server (Network Namespaces)\n"
    "Duration: 10 seconds"
)

    
# ==========================
# 1. Throughput vs Message Size
# ==========================
msg_sizes = [64, 256, 1024, 4096]
tp_a1 = [2.094, 8.197, 14.695, 26.937]
tp_a2 = [4.203, 14.882, 46.837, 87.664]
tp_a3 = [2.081, 7.921, 28.183, 59.125]

fig, ax = plt.subplots(figsize=(8, 5))
ax.plot(msg_sizes, tp_a1, marker='o', label='A1 (two-copy)')
ax.plot(msg_sizes, tp_a2, marker='s', label='A2 (one-copy)')
ax.plot(msg_sizes, tp_a3, marker='^', label='A3 (zero-copy)')

ax.set_xlabel("Message Size (bytes)")
ax.set_ylabel("Throughput (Gbps)")
ax.set_title("Throughput vs Message Size (Threads = 8)")
ax.legend()
ax.grid(True)
ax.text(
        0.98, 0.02,
        SYS_CONFIG,
        transform=ax.transAxes,
        fontsize=9,
        ha="right",
        va="bottom",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.8)
    )

plt.show()

# ==========================
# 2. Latency vs Thread Count
# ==========================
threads = [1, 2, 4, 8]
lat_a1 = [19.81, 25.855, 38.1325, 42.5787]
lat_a2 = [2.37, 2.995, 4.9525, 10.0062]
lat_a3 = [6.7, 7.665, 12.215, 17.8813]

fig, ax = plt.subplots(figsize=(8, 5))
ax.plot(threads, lat_a1, marker='o', label='A1 (two-copy)')
ax.plot(threads, lat_a2, marker='s', label='A2 (one-copy)')
ax.plot(threads, lat_a3, marker='^', label='A3 (zero-copy)')

ax.set_xlabel("Thread Count")
ax.set_ylabel("Average Latency (µs)")
ax.set_title("Latency vs Thread Count (Message Size = 1024 bytes)")
ax.legend()
ax.grid(True)
ax.text(
    0.5, 0.5,               
    SYS_CONFIG,
    transform=ax.transAxes,
    fontsize=9,
    ha="center",
    va="center",
    bbox=dict(
        boxstyle="round",
        facecolor="white",
        edgecolor="black",
        alpha=0.85
    )
)
plt.show()

# ==========================
# 3. Cache Misses vs Message Size
# ==========================
l1_miss_a1 = [418908149, 927880771, 1272117176, 1897313140]
l1_miss_a2 = [193395214, 682731315, 2390065876, 5776038566]
l1_miss_a3 = [1020294305, 1446446362, 1977927797, 4046909689]

fig, ax = plt.subplots(figsize=(8, 5))
ax.plot(msg_sizes, l1_miss_a1, marker='o', label='A1 (two-copy)')
ax.plot(msg_sizes, l1_miss_a2, marker='s', label='A2 (one-copy)')
ax.plot(msg_sizes, l1_miss_a3, marker='^', label='A3 (zero-copy)')

ax.set_xlabel("Message Size (bytes)")
ax.set_ylabel("L1 Cache Load Misses")
ax.set_title("L1 Cache Misses vs Message Size (Threads = 4)")
ax.legend()
ax.grid(True)
ax.text(
        0.98, 0.02,
        SYS_CONFIG,
        transform=ax.transAxes,
        fontsize=9,
        ha="right",
        va="bottom",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.8)
    )
plt.show()

# ==========================
# 4. CPU Cycles per Byte
# ==========================
cycles_per_byte_a1 = [5.968, 2.716, 1.553, 0.786]
cycles_per_byte_a2 = [0.913, 0.275, 0.116, 0.093]
cycles_per_byte_a3 = [4.503, 2.009, 0.546, 0.212]

fig, ax = plt.subplots(figsize=(8, 5))
ax.plot(msg_sizes, cycles_per_byte_a1, marker='o', label='A1 (two-copy)')
ax.plot(msg_sizes, cycles_per_byte_a2, marker='s', label='A2 (one-copy)')
ax.plot(msg_sizes, cycles_per_byte_a3, marker='^', label='A3 (zero-copy)')

ax.set_xlabel("Message Size (bytes)")
ax.set_ylabel("CPU Cycles per Byte")
ax.set_title("CPU Cycles per Byte vs Message Size (Threads = 4)")
ax.legend()
ax.grid(True)

ax.text(
    0.5, 0.97,                 
    SYS_CONFIG,
    transform=ax.transAxes,
    fontsize=9,
    ha="center",
    va="top",
    bbox=dict(
        boxstyle="round",
        facecolor="white",
        edgecolor="black",
        alpha=0.9
    )
)


plt.tight_layout()
plt.show()


plt.show()

