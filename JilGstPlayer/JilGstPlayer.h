#ifndef SIMPLE_GST_PLAYER_H
#define SIMPLE_GST_PLAYER_H

#include <gst/gst.h>
#include <string>
#include <functional>

#include "GstPlayerCommand.h"

class JilGstPlayer {

public:
    JilGstPlayer();
    virtual ~JilGstPlayer();

    bool Post(BASE_COMMAND* command);
    void SetOnGstMessage(std::function<void(GstMessage* message)> msg_callback);

private:
    bool Execute(BASE_COMMAND* command);

    void Init();
    bool Open(std::string pipeline);
    bool Play();
    bool Pause();
    bool Stop();
    bool SetProperty(std::string name, std::function<void(GstElement* element)> callback);

    GstElement* GetGstElementByName(std::string name);
    bool RegisterWatchBus();
    bool UnRegisterWatchBus();

    static gboolean callback_func(GstBus* bus, GstMessage* message, gpointer data);
    static gpointer command_loop(gpointer data);

    GstElement* pipeline_;
    GstBus* bus_;
    guint bus_signal_id_;
    

    GAsyncQueue* command_queue_;
    GThread* command_thread_;
    bool thread_running_;

    std::function<void(GstMessage* message)> msg_callback_;
        
};

#endif //SIMPLE_GST_PLAYER_H