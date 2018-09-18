/*************************************************************************/
/*                    This file is part of:                              */
/*                    BITSHIFT GODOT PLUGIN                              */
/*                    http://bit-shift.io                                */
/*************************************************************************/
/* Copyright (c) 2017   Fabian Mathews.                                  */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef BMATRIX_MASTER_SERVER_H
#define BMATRIX_MASTER_SERVER_H

#include "core/reference.h"
#include "modules/matrix/matrix.h"
#include "scene/main/timer.h"
#include "bhttp_client.h"

/**
        @author Fabian Mathews <supagu@gmail.com>
*/

class BMasterServer : public Reference {

    GDCLASS(BMasterServer,Reference)

public:

    enum ConnectionStatus {
        CS_DISCONNECTED = 0,
        CS_CONNECTING_1 = 1, // connecting in thread
        CS_CONNECTING_2 = 2, // connecting thread done, so now stuff that needs to happen on main thread
        CS_CONNECTED = 3
    };

    enum ServerStatus {
        SS_DISCONNECTED = 0,
        SS_CONNECTED = 1
    };

protected:

    static BMasterServer *singleton;

    URL master_server_url;
    BHttpClient master_server_client;

    MatrixClient matrix_client;
    Ref<MatrixRoom> matrix_room;

    Thread* connect_thread;
    Thread* refresh_servers_thread;
    Thread* update_server_thread;

    Vector<String> send_text_list;
    Mutex* send_text_list_mutex;
    Thread* send_text_thread;

    uint32_t server_id;
    Dictionary server_content;
    Mutex *server_mutex;
    ServerStatus server_status;

    //Map<String, Dictionary> member_data_map;
    Array servers;
    //Map<String, Dictionary> server_data_map;


    //bool need_more_host_data;
    bool guest;

    String room_id;

    Ref<MatrixUser> me;

    //Timer* poll_timer;
    //ConnectionStatus connection_status_poll_prev;
    ConnectionStatus connection_status;
    //List<Variant> connected_member_keys_list;

    Dictionary power_levels;

protected:

    static void _bind_methods();
    static void _connect_thread_function(void *self);
    static void _send_text_message_thread_function(void *self);
    static void _refresh_servers_thread_function(void *self);
    static void _update_server_thread_function(void *self);

    void _connect_1();
    void _connect_2();
    //void _poll();


    // here _blah_event occurs in a thread, but we are emitting a signal to trigger gdscript
    // now, lots if not all the stuff we do in gdscript is messing with UI code and needs to
    // occur in the main thread, else we get unexplained crashes!
    //
    void _timeline_event(Dictionary event);
    void _old_timeline_event(Dictionary event);
    void _ephemeral_event(Dictionary event);
    void _state_event(Dictionary event);

    void _timeline_event_deferred(Dictionary event);
/*
    void _old_timeline_event_deferred(Dictionary event);
    void _ephemeral_event_deferred(Dictionary event);
    void _state_event_deferred(Dictionary event);
*/
    //void _handle_server_data_event(Dictionary event);

    void _update_server_thread();

    void _refresh_servers();
    void _refresh_servers_deferred();

public:

    void set_power_levels();

    String get_room_id();
    String get_user_id();

    bool has_existing_login();
    String login(const String& username, const String& password, bool guest, bool remember);

    void connect_to_master_server();
    ConnectionStatus get_connection_status();

    // user info
    Error set_display_name(String display_name);
    String get_display_name(bool sync=false);

    // client hosting server information
    void update_server(const Dictionary &p_data);
    void delete_server();

    // get hosting information for available hosts
    Array get_servers();

    void refresh_servers();

    // room methods
    Error send_text_message(String text);
    String get_member_display_name(String id, bool sync=false);

    Dictionary get_members(bool sync=false);


    BMasterServer();
    ~BMasterServer();

    static BMasterServer *get_singleton() { return singleton; }
};

VARIANT_ENUM_CAST(BMasterServer::ConnectionStatus);

#endif // BMATRIX_MASTER_SERVER_H
