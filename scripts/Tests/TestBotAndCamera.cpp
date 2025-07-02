#include "DeltaBot.h"
#include <iostream>
#include <thread>
#include <termios.h>
#include <chrono>

using namespace std;

// A simple test uses the keyboard to control the robot's forward, backward and steering, and to regulate speed (using polling)
// Test the camera at the same time

/**
 * Reads individual character input directly from the keyboard without the need for a carriage return.
 * @return {char}  : input character
 */
char getch()
{
    struct termios oldt, newt; // Save settings of old and new terminal
    char ch;
    tcgetattr(STDIN_FILENO, &oldt); // Get current terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);        // Close canonical mode and echo (Do not need to press enter and the character will not be displayed on the screen)
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings
    ch = getchar();                          // Get one character
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
    return ch;
}

int main(int argc, char *argv[])
{
    DeltaBot deltabot;
    bool running = true;

    cout << "Use the keyborad to control the bot: W(forward) S(backward) A(turn left) D(trun right) R(speed up) F(declerate) X(stop) Z(quit)" << endl;
    cout << "Start Camera" << endl;
    deltabot.StartCamera(); // Start the camera feed in a separate thread
    int speed = 0;

    while (running)
    {
        if (true)
        {
            char cmd = getch();
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