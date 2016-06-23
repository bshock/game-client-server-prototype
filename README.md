# game-client-server-prototype

Game Networking Client/Server Prototype:


Introduction

This is a very bare-bones set of code for implementing turn-based games on a network.

If you were looking for a finished game, you won't find it here. This code only gives you
a starting point for implementing your own turn-based games on a network (preferably a
local network, to keep things simple).


Use Instructions

1) Compile both the GameServer and GameClient2 applications in Qt Creator (Qt Libraries
version 5.6 or above). They should work on both Linux and Windows.

2) Start the server, which is just a console app. Give it a listening port number that's
not being used on your current machine (netstat -a to check). Something like 1234 is
probably fine.

3) Start a client, which is a windowed app. Hit the "Connect to server" button. The first
time you do this, you'll be prompted for an IP and a port number. If you're testing this
on the same machine as the server, 127.0.0.1 will of course work. Otherwise, use the
local network IP of the machine running your server. (Open up whatever firewalls you have.)

4) The client will also prompt you for a username and password. Security isn't implemented
in this prototype, so just enter whatever you like. This login will identify you to the
server for game-sharing purposes.

5) If all goes well, you should get an acknowledgement from the server that you're logged
in as whatever name you gave it.

6) Now open a second client (same machine, different machine, doesn't matter) and do the
same thing, except use a different username/password.

7) When you have two clients up and connected to the server, choose one of them and hit the
"Create new game" button. This will automatically connect that client to the "game" session
started on the server. The other client will see a notice that a new game is open. Click on
that notice once and the client will join that game session.

8) About all you can do at this point is trade the "turn" back and forth by hitting the
"Complete turn" button on either client. The button will enable and disable itself based on
whose turn it is. In a real game client, you'd be making changes to your side of the game
state and then sending them off to the server and the other player by hitting "Complete turn."

9) Hit "Quit" to end the game. You should immediately get a notice that the game is
resumable. Clicking this notice will put you back in the same game.


Extension Instructions

Obviously, you need to build out the visual and UI for your turn-based game in the client
window. You can pass game state around by deriving something new from the GameState class.
Remember to override the read and write functions to serialize and deserialize the nformation
you want to send.

The server doesn't do anything in terms of login authentication right now. You can add this
in the checkLogin function however you like.

The server also has a huge glass jaw when it comes to interactive functionality: a big
honking lambda that's sent into all PlayerCommunicator objects (PlayerCommunicator being the
class the holds individual sockets and handles basic communication functions.) This lambda is
meant primarily for demo purposes. It's ugly and hard to read, and the if/then/else
arrangement can get mixed up very fast. I would recommend moving functionality into smaller
functions and probably decoding the communications strings into enums with a map, so you can
use a switch on them.

The GameContainer class handles game sessions. Right now it's written to depend on two and
only two players in a game session. You might want to open this up for two or more players,
depending on your turn-based game.

While you're at it, adding a logging system would also be a great idea. Servers that are
going to be up and running any amount of time need logs.