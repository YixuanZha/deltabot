#include "DeltaBot.h"

// Main function to test the DeltaBot camera functionality

int main(int argc, char* argv[])
{
    try
    {
        DeltaBot bot;
        bot.StartCamera();
    }
    catch(const std::exception& e)
    {
        std::cerr << "An expection occurred: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}