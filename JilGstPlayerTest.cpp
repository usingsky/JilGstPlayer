#include <iostream>
#include <unistd.h>
#include "JilGstPlayer.h"

using namespace std;

static GMainLoop *loop;

int main(){
    gst_init (nullptr, nullptr);

    JilGstPlayer* player = new JilGstPlayer;

    player->SetOnGstMessage([&](GstMessage* message) -> void {
        switch (GST_MESSAGE_TYPE (message)) {
            case GST_MESSAGE_ASYNC_DONE:{
                GstClockTime running_time;
                gst_message_parse_async_done (message, &running_time);
                std::cout << "Async Done :" << GST_TIME_AS_SECONDS(running_time) << endl;
                break;
            }
            case GST_MESSAGE_STATE_CHANGED: {
                GstState old_state, new_state, pending_state;
                gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
                g_print("Pipeline state changed from %s to %s [%s]\n", gst_element_state_get_name(old_state), 
                        gst_element_state_get_name(new_state), GST_OBJECT_NAME (message->src)); 
                break;
            }
            case GST_MESSAGE_TAG: {
                GstTagList* tags = NULL;
                gst_message_parse_tag (message, &tags);
                g_print ("[Tags from element] %s:\n", GST_OBJECT_NAME (message->src));
                gst_tag_list_free (tags);
                break;
            }
            case GST_MESSAGE_EOS: {
                g_print ("[EOS Message]\n");
                g_main_loop_quit(loop);
                break;
            }
            case GST_MESSAGE_ERROR: {
                GError *err = NULL;
                gchar *dbg_info = NULL;
    
                gst_message_parse_error (message, &err, &dbg_info);
                g_printerr ("[ERROR from element] %s: %s\n", GST_OBJECT_NAME (message->src), err->message);
                g_printerr ("[Debugging info] %s\n", (dbg_info) ? dbg_info : "none");
                g_error_free (err);
                g_free (dbg_info);
                break;
            }
            case GST_MESSAGE_ELEMENT:{
                g_print ("[Message from element] %s\n", GST_OBJECT_NAME (message->src));
            }
            default:{
                g_print ("[Message] %s\n", GST_MESSAGE_TYPE_NAME (message));
                break;
            }
        }
    });

    player->Post(new OPEN_CMD("videotestsrc name=src ! autovideosink name=sink"));
    player->Post(new SET_PROPERTY_CMD("src", [](GstElement* element) -> void {
            g_object_set(element, "pattern", 2, NULL);
        })
    );

    player->Post(new PLAY_CMD());

    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    player->Post(new LAST_CMD());
    
    return 1;
}