Full Name: Siyu Liu.
Student ID: 9062861443.
What I have done in the assignment: Completed the basic part.
Code files:
	ServerM.cpp: Check for valid client input and deliver the name to the corresponding server
	to query for result. Finally return the result to client.

	ServerA.cpp: Stores a list of names and their valid time intervals. Receive UDP call from 
	serverM and returns a intersection of time intervals when received a query.

	ServerB.cpp: Stores a list of names and their valid time intervals. Receive UDP call from 
	serverM and returns a intersection of time intervals when received a query.

	client.cpp: Send names to serverM request for valid time section(intersection).

Format of messages exchanged: 
	- usernames are concatenated and delimited by a comma and a space.
	- time intervals are int the format of a 2-dimensional array.

Idoiosyncrasy: None.