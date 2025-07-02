#include "DeltaBot.h"
#include <iostream>
#include <thread>
#include <termios.h>
#include <chrono>

using namespace std;

// A simple test uses the keyboard to control the robot's forward, backward and steering, and to regulate speed (using polling)
// Test the camera at the same time

struct termios orig_termios;

void DisableRawMode()
{
    tcsetattr(STDIN_FILENO,TCSANOW, &orig_termios);
}

void EnableRawMode()
{
    tcgetattr(STDIN_FILENO,&orig_termios);
    atexit(DisableRawMode);

    struct termios raw = orig_termios;
    raw.c_cflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// /**
//  * Reads individual character input directly from the keyboard without the need for a carriage return.
//  * @return {char}  : input character
//  */
// char getch()
// {
//     struct termios oldt, newt; // Save settings of old and new terminal
//     char ch;
//     tcgetattr(STDIN_FILENO, &oldt); // Get current terminal settings
//     newt = oldt;
//     newt.c_lflag &= ~(ICANON | ECHO);        // Close canonical mode and echo (Do not need to press enter and the character will not be displayed on the screen)
//     tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings
//     ch = getchar();                          // Get one character
//     tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
//     return ch;
// }

int main(int argc, char *argv[])
{
    EnableRawMode();
    DeltaBot deltabot;
    bool running = true;

    cout << "Use the keyborad to control the bot: W(forward) S(backward) A(turn left) D(trun right) R(speed up) F(declerate) X(stop) Z(quit)" << endl;
    cout << "Start Camera" << endl;
    deltabot.StartCamera(); // Start the camera feed in a separate thread
    int speed = 0;

    while (running)
    {
        // Use select to wait for input from stdin with a timeout of 10 milliseconds
        // This allows the program to continue running without blocking on input
        // If there is input, it will read the character and execute the corresponding command
        // If there is no input, it will continue to the next iteration of the loop
        // This is a simple way to implement non-blocking input handling in a console application
        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(STDIN_FILENO, &fd); // Set the file descripter of stdin to 1

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        int activity = select(STDIN_FILENO + 1, &fd, NULL, NULL, &timeout); // Wait for input on stdin with a timeout of 10 milliseconds

        if (activity > 0)
        {
            // char cmd = getch();
            char cmd;
            read(STDIN_FILENO,&cmd,1);
            switch (cmd)
            {
            case 'w':
                deltabot.Forward(speed);
                cout << "Forward.....The speed is: " << speed << endl;
                break;
            case 's':
                deltabot.Backward(speed);
                cout << "Backward.....The speed is: " << speed << endl;
                break;
            case 'a':
                deltabot.TurnLeft(speed);
                cout << "Turn left.....The speed is: " << speed << endl;
                break;
            case 'd':
                deltabot.TurnRight(speed);
                cout << "Turn right.....The speed is: " << speed << endl;
                break;

            case 'r':
                speed++;
                if (speed >= 10)
                    speed = 10;
                cout << "Speed up.....The speed is: " << speed << endl;
                break;
            case 'f':
                speed--;
                if (speed <= 0)
                    speed = 0;
                cout << "Declerate.....The speed is: " << speed << endl;
                break;

            case 'x':
                deltabot.Stop();
                cout << "Stop....." << endl;
                break;

            case 'z':
                deltabot.Stop();
                cout << "Quit....." << endl;
                running = false;
                break;

            default:
                cout << "invalid command" << endl;
            }
        }
    }
    deltabot.StopCamera();
    return 0;
}