#ifndef GST_PLAYER_COMMAND_H
#define GST_PLAYER_COMMAND_H

#include <string>

enum COMMAND_TYPE
{
    OPEN,
    STOP,
    PLAY,
    PAUSE,
    SET_PROPERTY,
    LAST
};
    
enum PLAY_STATE
{
    STOPPED,
    PLAYING,
    PAUSED,
};

struct BASE_COMMAND {

    BASE_COMMAND(){}
    virtual ~BASE_COMMAND(){}

    virtual COMMAND_TYPE getType() = 0;
};

struct OPEN_CMD : BASE_COMMAND{

    std::string pipeline_;

    OPEN_CMD(std::string pipeline) : pipeline_(pipeline)
    {}

    COMMAND_TYPE getType(){return COMMAND_TYPE::OPEN;}
};

struct STOP_CMD : BASE_COMMAND{

    STOP_CMD()
    {}

    COMMAND_TYPE getType(){return COMMAND_TYPE::STOP;}
};

struct PLAY_CMD : BASE_COMMAND{

    PLAY_CMD()
    {}

    COMMAND_TYPE getType(){return COMMAND_TYPE::PLAY;}
};

struct PAUSE_CMD : BASE_COMMAND{

    PAUSE_CMD()
    {}

    COMMAND_TYPE getType(){return COMMAND_TYPE::PAUSE;}
};

struct SET_PROPERTY_CMD : BASE_COMMAND{

    std::string name_;
    std::function<void(GstElement* element)> callback_;

    SET_PROPERTY_CMD(std::string name, std::function<void(GstElement* element)> callback)
     : name_(name), callback_(callback)
    {}

    COMMAND_TYPE getType(){return COMMAND_TYPE::SET_PROPERTY;}
};

struct LAST_CMD : BASE_COMMAND{

    LAST_CMD()
    {}

    COMMAND_TYPE getType(){return COMMAND_TYPE::LAST;}
};

#endif //GST_PLAYER_COMMAND_H