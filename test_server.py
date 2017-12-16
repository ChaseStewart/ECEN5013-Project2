# Echo server program
import socket, time

# packet types
DATA_PKT    = 0
NO_DATA_PKT = 1
ALRT_PKT    = 2
LOG_PKT     = 3
ERR_PKT     = 4

# defines
HOST = '10.0.0.3'
PORT = 50021 # The same port as used by the server

#code 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST,PORT))
s.listen(1)

print("Listening on host %s:%d" % (HOST, PORT))

(conn, addr) = s.accept()
print("Connected by %d" % (addr[1]))

#while True:
#input = conn.recv(1024)
#print("Received %s" % (input))
#time.sleep(1)
#
#time.sleep(1)
##conn.send("%d:%s\0" % (NO_DATA_PKT, "Don't look at me!"))
##input = conn.recv(1024)
##print("Received %s" % (input))
#
#time.sleep(1)
#conn.send("%d:%s\0" % (ALRT_PKT, "Alert from humidity sensor"))
##input = conn.recv(1024)
##print("Received %s" % (input))
#
#time.sleep(1)
#conn.send("%d:%s\0" % (LOG_PKT, "Nice to meet you!"))
##input = conn.recv(1024)
##print("Received %s" % (input))
#
#time.sleep(1)
#conn.send("%d:%s\0" % (ERR_PKT, "Everything went great"))
##input = conn.recv(1024)
##print("Received %s" % (input))
#time.sleep(1)

try:
	while 1:
	    data = conn.recv(1024)
	    print("Received %s" % (data))
	    #break
except:
	conn.close()

