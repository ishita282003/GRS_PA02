# MT25026

CC = gcc
CFLAGS = -O2 -pthread

# ========================
# Executable names
# ========================

# Part A1
A1_SERVER = a1_server
A1_CLIENT_TP   = a1_client_tp
A1_CLIENT_LAT  = a1_client_lat
A1_CLIENT_PERF = a1_client_perf

# Part A2
A2_SERVER = a2_server
A2_CLIENT_TP   = a2_client_tp
A2_CLIENT_LAT  = a2_client_lat
A2_CLIENT_PERF = a2_client_perf

# Part A3
A3_SERVER = a3_server
A3_CLIENT_TP   = a3_client_tp
A3_CLIENT_LAT  = a3_client_lat
A3_CLIENT_PERF = a3_client_perf

# ========================
# Default target
# ========================
all: \
	$(A1_SERVER) $(A1_CLIENT_TP) $(A1_CLIENT_LAT) $(A1_CLIENT_PERF) \
	$(A2_SERVER) $(A2_CLIENT_TP) $(A2_CLIENT_LAT) $(A2_CLIENT_PERF) \
	$(A3_SERVER) $(A3_CLIENT_TP) $(A3_CLIENT_LAT) $(A3_CLIENT_PERF)

# ========================
# Part A1
# ========================
$(A1_SERVER):
	$(CC) $(CFLAGS) MT25026_Part_A1_Server.c -o $(A1_SERVER)

$(A1_CLIENT_TP):
	$(CC) $(CFLAGS) MT25026_Part_A1_Client_TP.c -o $(A1_CLIENT_TP)

$(A1_CLIENT_LAT):
	$(CC) $(CFLAGS) MT25026_Part_A1_Client_LAT.c -o $(A1_CLIENT_LAT)

$(A1_CLIENT_PERF):
	$(CC) $(CFLAGS) MT25026_Part_A1_Client_PERF.c -o $(A1_CLIENT_PERF)

# ========================
# Part A2
# ========================
$(A2_SERVER):
	$(CC) $(CFLAGS) MT25026_Part_A2_Server.c -o $(A2_SERVER)

$(A2_CLIENT_TP):
	$(CC) $(CFLAGS) MT25026_Part_A2_Client_TP.c -o $(A2_CLIENT_TP)

$(A2_CLIENT_LAT):
	$(CC) $(CFLAGS) MT25026_Part_A2_Client_LAT.c -o $(A2_CLIENT_LAT)

$(A2_CLIENT_PERF):
	$(CC) $(CFLAGS) MT25026_Part_A2_Client_PERF.c -o $(A2_CLIENT_PERF)

# ========================
# Part A3
# ========================
$(A3_SERVER):
	$(CC) $(CFLAGS) MT25026_Part_A3_Server.c -o $(A3_SERVER)

$(A3_CLIENT_TP):
	$(CC) $(CFLAGS) MT25026_Part_A3_Client_TP.c -o $(A3_CLIENT_TP)

$(A3_CLIENT_LAT):
	$(CC) $(CFLAGS) MT25026_Part_A3_Client_LAT.c -o $(A3_CLIENT_LAT)

$(A3_CLIENT_PERF):
	$(CC) $(CFLAGS) MT25026_Part_A3_Client_PERF.c -o $(A3_CLIENT_PERF)

# ========================
# Clean
# ========================
clean:
	rm -f \
	$(A1_SERVER) $(A1_CLIENT_TP) $(A1_CLIENT_LAT) $(A1_CLIENT_PERF) \
	$(A2_SERVER) $(A2_CLIENT_TP) $(A2_CLIENT_LAT) $(A2_CLIENT_PERF) \
	$(A3_SERVER) $(A3_CLIENT_TP) $(A3_CLIENT_LAT) $(A3_CLIENT_PERF)

