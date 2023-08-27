
from socket import *
def main():
    serverName = '127.0.0.2'
    serverPort = 12000
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((serverName,serverPort))
    sentence = b"Input lowercase sentence:"
    clientSocket.send(sentence)
    modifiedSentence = clientSocket.recv(1024)
    print ('From Server:', modifiedSentence)
    clientSocket.close()

if __name__ == "__main__":
    main()
