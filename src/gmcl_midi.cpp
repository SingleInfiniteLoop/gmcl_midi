#include <time.h>
#include <stdio.h>
#include <RtMidi.h>
#include "Interface.h"

using namespace GarrysMod::Lua;

#if defined(_WIN32)
int clock_gettime(clockid_t clock_id, struct timespec *tp);
#endif

double gameStartTime = 0;
RtMidiIn *midiIn = 0;

/*
    Struct that defines a single MIDI events that occurred at a given time
*/
struct TimedMIDIEvent
{
    double time;
    std::vector<unsigned char> message; // command and parameters of the message
};

std::vector<TimedMIDIEvent> messageList;

static inline double systime(void)
{
    struct timespec tm;
    return (clock_gettime(CLOCK_MONOTONIC, &tm) == 0) ?
           (double)tm.tv_sec + (double)tm.tv_nsec * 1.0E-9 : 0.0;
}

/*
    Helper function: get the system time (equal to SysTime() in Lua)
    Used to time the MIDI events exactly.
*/
double getSysTime()
{
    return gameStartTime + systime();
}

/*
    Called when a MIDI event occurs
*/
void onMidiCallback(double timestamp, std::vector<unsigned char> *message, void * /*userData*/)
{
    if (message->size() > 0)
    {
        TimedMIDIEvent ev;
        ev.time = getSysTime();
        ev.message = std::vector<unsigned char>(*message);
        messageList.push_back(ev);
    }
}

/*
    Get the available MIDI ports
*/
LUA_FUNCTION(MidiGetPorts)
{
    unsigned int portCount = midiIn->getPortCount();

    LUA->CreateTable();
    for (unsigned int i = 0; i < portCount; i++)
    {
        LUA->PushNumber(i);
        LUA->PushString(midiIn->getPortName(i).c_str());
        LUA->SetTable(-3);
    }
    return 1;
}

/*
    Open the MIDI device
*/
LUA_FUNCTION(MidiOpen)
{
    int result = 0;
    int port = 0;

    if (LUA->IsType(1, Type::Number))
    {
        port = (int)LUA->GetNumber(1);
    }
    if (midiIn->getPortCount() > 0)
    {
        try
        {
            midiIn->openPort(port);
            LUA->PushString(midiIn->getPortName().c_str());
            result = 1;
        }
        catch (RtMidiError &error)
        {
            LUA->ThrowError(error.getMessage().c_str());
        }
    }
    else
    {
        LUA->ThrowError("No input ports available!");
        return 0;
    }
    return result;
}

/*
    Close the connection in the MIDI device
*/
LUA_FUNCTION(MidiClose)
{
    int result = 0;

    if (midiIn->isPortOpen())
    {
        midiIn->closePort();
        result = 1;
    }
    else
    {
        LUA->ThrowError("There aren't any ports open");
    }
    return result;
}

/*
    Whether a port has been opened
*/
LUA_FUNCTION(MidiIsOpened)
{
    LUA->PushBool(midiIn->isPortOpen());
    return 1;
}

/*
    This function is called every frame.
    It's used for syncing the MIDI events with Lua
*/
LUA_FUNCTION(MidiPoll)
{
    unsigned int messagesNumber = messageList.size();

    if (messagesNumber > 0)
    {
        LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
            LUA->GetField(-1, "hook");
                for (unsigned int id = 0; id < messagesNumber; id++)
                {
                    std::vector<unsigned char> message = messageList[id].message;
                    unsigned int msgSize = message.size();
                    LUA->GetField(-1, "Call");
                    LUA->PushString("MIDI");
                    LUA->PushNil();
                    LUA->PushNumber(messageList.at(id).time);
                    for (unsigned int byte = 0; byte < msgSize; byte++)
                    {
                        LUA->PushNumber(message[byte]);
                    }
                    LUA->Call(3 + msgSize, 0);
                }
            LUA->Pop();
        LUA->Pop();
        messageList.clear();
    }
    return 0;
}

//
// MIDI helper functions
//

// Get the code of a command
LUA_FUNCTION(MidiCommandGetCode)
{
    if (!LUA->CheckNumber(1))
    {
        return 0;
    }
    unsigned int code = (unsigned int) LUA->GetNumber(1);

    LUA->PushNumber(code & 0xF0); // strip last four bits
    return 1;
}

LUA_FUNCTION(MidiCommandGetChannel)
{
    if (!LUA->CheckNumber(1))
    {
        return 0;
    }
    unsigned int code = (unsigned int) LUA->GetNumber(1);

    LUA->PushNumber(code & 0x0F); // strip first four bits
    return 1;
}

LUA_FUNCTION(MidiCommandGetName)
{
    if (!LUA->CheckNumber(1))
    {
        return 0;
    }
    unsigned int command = (unsigned int)LUA->GetNumber(1);

    command = command & 0xF0;
    if (command == 0x80)
        LUA->PushString("NOTE_OFF");
    else if (command == 0x90)
        LUA->PushString("NOTE_ON");
    else if (command == 0xA0)
        LUA->PushString("AFTERTOUCH");
    else if (command == 0xB0)
        LUA->PushString("CONTINUOUS_CONTROLLER");
    else if (command == 0xC0)
        LUA->PushString("PATCH_CHANGE");
    else if (command == 0xD0)
        LUA->PushString("CHANNEL_PRESSURE");
    else if (command == 0xE0)
        LUA->PushString("PITCH_BEND");
    else if (command == 0xF0)
        LUA->PushString("SYSEX");
    else
        LUA->PushNil();
    return 1;
}

//
// Called when module is opened
//
GMOD_MODULE_OPEN()
{
    try
    {
        midiIn = new RtMidiIn();
    }
    catch (RtMidiError &e)
    {
        e.printMessage();
        return 1;
    }
    midiIn->setCallback(&onMidiCallback);
    // Get the SysTime at the start of the program
    double startTime = systime(); // time since loading of module
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
        LUA->GetField(-1, "SysTime");
        LUA->Call(0, 1);
        gameStartTime = LUA->GetNumber() - startTime; // substract clock to compensate for delay since module start
    LUA->Pop();
    // Add the polling hook
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
    LUA->GetField(-1, "hook");
        LUA->GetField(-1, "Add");
        LUA->PushString("Think");
        LUA->PushString("MIDI polling");
        LUA->PushCFunction(MidiPoll);
        LUA->Call(3, 0);
    LUA->Pop();
    // Create the midi table
    LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
        LUA->CreateTable();
            LUA->PushCFunction(MidiGetPorts); LUA->SetField(-2, "GetPorts");
            LUA->PushCFunction(MidiOpen); LUA->SetField(-2, "Open");
            LUA->PushCFunction(MidiClose); LUA->SetField(-2, "Close");
            LUA->PushCFunction(MidiIsOpened); LUA->SetField(-2, "IsOpened");
            // MIDI command helper functions
            LUA->PushCFunction(MidiCommandGetCode); LUA->SetField(-2, "GetCommandCode");
            LUA->PushCFunction(MidiCommandGetChannel); LUA->SetField(-2, "GetCommandChannel");
            LUA->PushCFunction(MidiCommandGetName); LUA->SetField(-2, "GetCommandName");
        LUA->SetField(-2, "midi");
    LUA->Pop();
    return 0;
}

//
// Called when your module is closed
//
GMOD_MODULE_CLOSE()
{
    delete midiIn;
    return 0;
}

