#include <iostream>
#include "JilGstPlayer.h"

JilGstPlayer::JilGstPlayer() :
    pipeline_(nullptr),
    bus_(nullptr),
    bus_signal_id_(0),
    command_queue_(g_async_queue_new()),
    command_thread_(g_thread_new("command_loop", command_loop, this)),
    thread_running_(true),
    msg_callback_(nullptr)
{
}

JilGstPlayer::~JilGstPlayer() {

    Post(new LAST_CMD());

    if(command_thread_){
        g_thread_join (command_thread_);
        command_thread_ = nullptr;
    }
    
    if(command_queue_){
        g_async_queue_unref(command_queue_);
        command_queue_ = nullptr;
    }
}


bool JilGstPlayer::Post(BASE_COMMAND* command){
    bool result = false;

    if(command_queue_ && command){
        g_async_queue_push(command_queue_, command);
        result = true;
    }

    return result;
}

void JilGstPlayer::SetOnGstMessage(std::function<void(GstMessage* message)> msg_callback){
    msg_callback_ = msg_callback;
}

bool JilGstPlayer::Execute(BASE_COMMAND* command){
    bool result = false;

    switch(command->getType()){
        case COMMAND_TYPE::OPEN : {
            OPEN_CMD* cmd = static_cast<OPEN_CMD*>(command);
            Open(cmd->pipeline_);
            break;
        }
        case COMMAND_TYPE::PLAY : {
            PLAY_CMD* cmd = static_cast<PLAY_CMD*>(command);
            Play();
            break;
        }
        case COMMAND_TYPE::PAUSE : {
            PLAY_CMD* cmd = static_cast<PLAY_CMD*>(command);
            Pause();
            break;
        }
        case COMMAND_TYPE::STOP : {
            PLAY_CMD* cmd = static_cast<PLAY_CMD*>(command);
            Stop();
            break;
        }
        case COMMAND_TYPE::SET_PROPERTY : {
            SET_PROPERTY_CMD* cmd = static_cast<SET_PROPERTY_CMD*>(command);
            SetProperty(cmd->name_, cmd->callback_);
            break;
        }
        case COMMAND_TYPE::LAST : {
            thread_running_ = false;
            break;
        }
        default:
            result = false;
    }

    return result;
}

bool JilGstPlayer::Open(std::string pipeline){
    bool result = true;

    gchar* desc = nullptr;

    //desc = g_strdup_printf("%s%s", "videotestsrc ! ", "autovideosink name=sink");
    desc = g_strdup_printf("%s", pipeline.c_str());

    GError *error = nullptr;
    pipeline_ = gst_parse_launch(desc, &error);

    if(pipeline_){
        //sink = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");
        RegisterWatchBus();
    }else{
        result = false;
    }

    return result;
}

bool JilGstPlayer::Play(){
    bool result = true;
    GstStateChangeReturn ret_gst = GST_STATE_CHANGE_SUCCESS;
    ret_gst = gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_PLAYING);

    if(ret_gst == GST_STATE_CHANGE_ASYNC){
        ret_gst = gst_element_get_state(GST_ELEMENT(pipeline_), NULL, NULL, 2500 * GST_MSECOND);
        if(ret_gst == GST_STATE_CHANGE_FAILURE){
            //fail
            result = false;
        }
    }else if(ret_gst == GST_STATE_CHANGE_FAILURE){
        //fail
        result = false;
    }
    return result;
}

bool JilGstPlayer::Pause(){
    bool result = true;
    GstStateChangeReturn ret_gst = GST_STATE_CHANGE_SUCCESS;
    ret_gst = gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_PAUSED);

    if(ret_gst == GST_STATE_CHANGE_FAILURE){
        //fail
        result = false;
    }
    return result;
}

bool JilGstPlayer::Stop(){
    bool result = true;
    GstStateChangeReturn ret_gst = GST_STATE_CHANGE_SUCCESS;
    ret_gst = gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_NULL);

    if(ret_gst == GST_STATE_CHANGE_FAILURE){
        //fail
        result = false;
    }else{
        UnRegisterWatchBus();
        gst_object_unref(GST_OBJECT(pipeline_));
    }
    return result;
}

bool JilGstPlayer::SetProperty(std::string name, std::function<void(GstElement* element)> callback){
    bool result = true;

    GstElement* element = GetGstElementByName(name);
    if(element && callback){
        callback(element);
    }else{
        result = false;
    }

    return result;
}

GstElement* JilGstPlayer::GetGstElementByName(std::string name){
    GstElement* element = gst_bin_get_by_name(GST_BIN(pipeline_), name.c_str());
    return element;
}

bool JilGstPlayer::RegisterWatchBus(){
    bool result = false;

    if(bus_){
        UnRegisterWatchBus();
    }

    if(pipeline_){
        bus_ = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    }

    if(bus_){
        //gst_bus_add_signal_watch(bus_);
        //bus_signal_id_ = g_signal_connect(bus_, "message", G_CALLBACK(callback_func), static_cast<void*>(this));
        bus_signal_id_ = gst_bus_add_watch(bus_, callback_func, this);
        result = bus_signal_id_ > 0;
    }
    return result;
}

bool JilGstPlayer::UnRegisterWatchBus(){
    if(bus_){
        gst_bus_set_flushing(bus_, true);
    }

    if(bus_signal_id_){
        g_signal_handler_disconnect(bus_, bus_signal_id_);
    }

    gst_bus_remove_signal_watch(bus_);
    gst_object_unref(bus_);
    bus_ = nullptr;
}

gboolean JilGstPlayer::callback_func(GstBus* bus, GstMessage* message, gpointer data){
    JilGstPlayer* player = static_cast<JilGstPlayer*>(data);
    if(player && player->msg_callback_){
        player->msg_callback_(message);
    }else{
        g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));
        std::cout << GST_MESSAGE_TYPE_NAME (message);
    }
    
    return TRUE;
}

gpointer JilGstPlayer::command_loop(gpointer data){
    std::cout << "Start Command Loop\n";
    JilGstPlayer* player = static_cast<JilGstPlayer*>(data);
    BASE_COMMAND* command = nullptr;
    bool running = true;

    while(player->command_queue_ && running){
        command  = static_cast<BASE_COMMAND*>(g_async_queue_pop(player->command_queue_));
        player->Execute(command);

        delete command;
        command = nullptr;
    }
    return NULL;
}