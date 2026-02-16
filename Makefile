both client.c server.c:
	clang client.c -o client
	clang server.c -o server

run client server:
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP_Server' && ./server\""
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP_Server' && ./client\""
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP_Server' && ./client\""
	osascript -e "tell application \"Terminal\" to do script \"cd '/Users/zakreashlaibah/Documents/Code Files/C/TCP_Server' && ./client\""
