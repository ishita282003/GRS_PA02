#!/bin/bash
# MT25026 - Part C: Fully Automated Measurement Script (Namespaces + Experiments)

set -e

# ==========================
# Configuration
# ==========================
PORT=9000
SERVER_NS="ns_server"
CLIENT_NS="ns_client"
SERVER_IP="10.0.0.1"
CLIENT_IP="10.0.0.2"
DURATION=10
SERVER_TIMEOUT=$((DURATION + 3))

MSG_SIZES=(64 256 1024 4096)
THREADS=(1 2 4 8)

PERF_EVENTS="cycles,instructions,L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses,context-switches"

# ==========================
# Network Namespace Setup
# ==========================
echo "[INFO] Setting up network namespaces..."

# Clean old namespaces (safe re-run)
ip netns del ${SERVER_NS} 2>/dev/null || true
ip netns del ${CLIENT_NS} 2>/dev/null || true

# Create namespaces
ip netns add ${SERVER_NS}
ip netns add ${CLIENT_NS}

# Create veth pair
ip link add veth_server type veth peer name veth_client

# Move interfaces into namespaces
ip link set veth_server netns ${SERVER_NS}
ip link set veth_client netns ${CLIENT_NS}

# Assign IP addresses
ip netns exec ${SERVER_NS} ip addr add ${SERVER_IP}/24 dev veth_server
ip netns exec ${CLIENT_NS} ip addr add ${CLIENT_IP}/24 dev veth_client

# Bring interfaces up
ip netns exec ${SERVER_NS} ip link set veth_server up
ip netns exec ${CLIENT_NS} ip link set veth_client up
ip netns exec ${SERVER_NS} ip link set lo up
ip netns exec ${CLIENT_NS} ip link set lo up

echo "[INFO] Network namespaces ready."

# ==========================
# Compile everything
# ==========================
echo "[INFO] Compiling all binaries..."
make clean
make

# ==========================
# Initialize MASTER CSV files
# ==========================
MASTER_TP="MT25026_Part_C_throughput.csv"
MASTER_LAT="MT25026_Part_C_latency.csv"
MASTER_PERF="MT25026_Part_C_perf.csv"

echo "impl,msg_size,threads,total_Gb,duration_sec,throughput_Gbps" > "${MASTER_TP}"
echo "impl,msg_size,threads,avg_us,min_us,max_us" > "${MASTER_LAT}"
echo "impl,msg_size,threads,cycles,instructions,l1_loads,l1_misses,llc_loads,llc_misses,context_switches" > "${MASTER_PERF}"

# ==========================
# Helper: run clients safely (in ns_client)
# ==========================
run_clients_to_file () {
    local client=$1
    local threads=$2
    local msg=$3
    local outfile=$4

    rm -f "${outfile}"

    for ((i=1;i<=threads;i++)); do
        ip netns exec ${CLIENT_NS} \
            ./${client} ${SERVER_IP} ${PORT} ${msg} ${DURATION} >> "${outfile}" &
    done
    wait
}

# ==========================
# Main experiment loop
# ==========================
for IMPL in a1 a2 a3; do
  for MSG in "${MSG_SIZES[@]}"; do
    for T in "${THREADS[@]}"; do

      echo "[INFO] ${IMPL} | MSG=${MSG} | THREADS=${T}"

      SERVER=./${IMPL}_server
      CLIENT_TP=${IMPL}_client_tp
      CLIENT_LAT=${IMPL}_client_lat
      CLIENT_PERF=${IMPL}_client_perf

      # ==================================================
      # THROUGHPUT (Gb / Gbps)
      # ==================================================
      ip netns exec ${SERVER_NS} \
        timeout ${SERVER_TIMEOUT} ${SERVER} ${PORT} ${MSG} ${T} &
      sleep 2

      TP_TMP="tp_tmp.txt"
      run_clients_to_file ${CLIENT_TP} ${T} ${MSG} "${TP_TMP}"
      wait || true

      TOTAL_GB=$(grep "Data=.*Gb" tp_tmp.txt | sed 's/Data=//; s/ Gb//' | awk '{sum+=$1} END {print sum}')
      TOTAL_GBPS=$(grep "Throughput=" "${TP_TMP}" | awk -F'[= ]' '{sum+=$2} END {print sum+0}')

      echo "${IMPL},${MSG},${T},${TOTAL_GB},${DURATION},${TOTAL_GBPS}" >> "${MASTER_TP}"

      # ==================================================
      # LATENCY
      # ==================================================
      ip netns exec ${SERVER_NS} \
        timeout ${SERVER_TIMEOUT} ${SERVER} ${PORT} ${MSG} ${T} &
      sleep 2

      LAT_TMP="lat_tmp.txt"
      run_clients_to_file ${CLIENT_LAT} ${T} ${MSG} "${LAT_TMP}"
      wait || true

      AVG=$(grep "Avg:" "${LAT_TMP}" | awk '{sum+=$2; n++} END {if(n>0) print sum/n; else print 0}')
      MIN=$(grep "Min:" "${LAT_TMP}" | awk '{print $2}' | sort -n | head -1)
      MAX=$(grep "Max:" "${LAT_TMP}" | awk '{print $2}' | sort -n | tail -1)

      echo "${IMPL},${MSG},${T},${AVG},${MIN},${MAX}" >> "${MASTER_LAT}"

      # ==================================================
      # PERF (server-side, in ns_server)
      # ==================================================
      ip netns exec ${SERVER_NS} \
        timeout -s INT ${SERVER_TIMEOUT} perf stat -e ${PERF_EVENTS} \
        ${SERVER} ${PORT} ${MSG} ${T} 2> perf_tmp.txt &
      sleep 2

      run_clients_to_file ${CLIENT_PERF} ${T} ${MSG} perf_client_tmp.txt
      wait || true

      CYCLES=$(grep -m1 " cycles" perf_tmp.txt | awk '{print $1}' | tr -d ',')
      INST=$(grep -m1 " instructions" perf_tmp.txt | awk '{print $1}' | tr -d ',')
      L1L=$(grep -m1 "L1-dcache-loads" perf_tmp.txt | awk '{print $1}' | tr -d ',')
      L1M=$(grep -m1 "L1-dcache-load-misses" perf_tmp.txt | awk '{print $1}' | tr -d ',')
      LLCL=$(grep -m1 "LLC-loads" perf_tmp.txt | awk '{print $1}' | tr -d ',')
      LLCM=$(grep -m1 "LLC-load-misses" perf_tmp.txt | awk '{print $1}' | tr -d ',')
      CS=$(grep -m1 "context-switches" perf_tmp.txt | awk '{print $1}' | tr -d ',')

      echo "${IMPL},${MSG},${T},${CYCLES},${INST},${L1L},${L1M},${LLCL},${LLCM},${CS}" >> "${MASTER_PERF}"

      echo "[DONE] ${IMPL} MSG=${MSG} T=${T}"
      echo

    done
  done
done

# ==========================
# Cleanup intermediate files
# ==========================
echo "[INFO] Cleaning up intermediate files..."
rm -f tp_tmp.txt lat_tmp.txt perf_tmp.txt perf_client_tmp.txt

# ==========================
# Network Namespace Cleanup
# ==========================
echo "[INFO] Cleaning up network namespaces..."
ip netns del ${SERVER_NS}
ip netns del ${CLIENT_NS}

echo "[SUCCESS] All experiments completed successfully."

