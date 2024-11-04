### Simple TCP client to test for data from Pi on local network.
### Will fail to connect if Pi has not recieved any data

import socket
import msvcrt  # Windows-specific module

# Define the server address (hostname and port)
host = 'nmea2000pi.local'
port = 2598

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the server
client_socket.connect((host, port))
print(f"Connected to {host}:{port}")

# Function to check if a key is pressed (Windows-specific)
def kbhit():
    return msvcrt.kbhit()

while True:
    # Receive and print data from the server
    data = client_socket.recv(1024).decode('utf-8')
    print(f"Received data from server: {data}")

    # Check if the user pressed a key
    if kbhit():
        break

# Close the connection
client_socket.close()
