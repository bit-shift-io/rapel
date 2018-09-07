
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
#include "bmaster_server.h"
#include "bconfig.h"
#include "butil.h"
#include "scene/main/viewport.h"
#include "core/io/json.h"
#include <assert.h>

#define INTERNET_TIMEOUT_MS 1000

BMasterServer *BMasterServer::singleton=NULL;

/*
 * In riot.im you can type /devtools to see the room state
 * the room owner (me!) can call this function to allow users permission to use certain events
 * we need to call this once on any room we want to user as a master server
 *
 * to run this code, in the console of your game enter:
 * /c BMasterServer.set_power_levels()
 *
 * https://matrix.org/docs/spec/client_server/r0.3.0.html#m-room-power-levels
 */
void BMasterServer::set_power_levels() {
    // refresh room state
    _refresh_servers();

    Dictionary e = power_levels["events"];
    e["bs.room.server_data"] = 0;
    power_levels["events"] = e;

    print_line("Attempting to set power levels: " + JSON::print(power_levels));
    Error err = matrix_room->set_state("m.room.power_levels", "", power_levels);//TEST THIS BEFORE RUNNING AGAIN!!!!!
    switch (err)
    {
    case MATRIX_OK:
        print_line("m.room.power_levels set OK");
        break;
    default:
        print_error("m.room.power_levels failed, are you logged in as the right user?");
        break;
    }

}

bool BMasterServer::has_existing_login() {
    bool has = BConfig::get_singleton()->has_setting("matrix/auth_token");
    return has;
}

String BMasterServer::login(const String& username, const String& password, bool p_guest, bool remember) {
    Error err;
    guest = p_guest;
    if (guest) {
        err = matrix_client.register_account(Variant(), "secret_password", true);
    }
    else {
        err = matrix_client.login(username, password);
    }

    switch (err)
    {
    case MATRIX_NOT_IMPLEMENTED:
        return "Homeserver requires additional authentication information, not implemented yet";
    case MATRIX_INVALID_REQUEST:
        return "Invalid registration request";
    case MATRIX_RATELIMITED:
        return "Registration request was ratelimited";
    case MATRIX_UNAUTHORIZED:
        return "Login failed";
    case MATRIX_OK:
    default:
        break;
    }

    // store authentication info
    // TODO: save this in the private settings
    if (remember) {
        BConfig::get_singleton()->set_setting("matrix/auth_token", matrix_client.get_auth_token());
        BConfig::get_singleton()->set_setting("matrix/user_id", matrix_client.get_user_id());
        BConfig::get_singleton()->set_setting("matrix/sync_token", matrix_client.get_sync_token());
        BConfig::get_singleton()->set_setting("matrix/guest", guest);
        /*
        BConfig::get_singleton()->set_setting("matrix/username", username);
        BConfig::get_singleton()->set_setting("matrix/p", password);*/
    }

    // get users matrix username
    me = matrix_client.get_me();
    me->get_display_name(true);

    return "";
}

void BMasterServer::_connect_thread_function(void *self) {
    BMasterServer *ms = (BMasterServer *)self;
    ms->_connect_1();
}

void BMasterServer::_connect_1() {
    uint64_t us = OS::get_singleton()->get_ticks_usec();

    // Now connect to our master server
    PoolVector<String> urls = BConfig::get_singleton()->get_setting("bitshift/matrix_master_server/urls", PoolStringArray());
    int ucount = urls.size();
    PoolVector<String>::Read r = urls.read();
    bool valid_ms = false;
    for (int i = 0; i < ucount; i++) {
        master_server_url = URL(urls[i]);
        Error err = master_server_client.connect_to_host(master_server_url.host, master_server_url.port, master_server_url.protocol == "https", true, INTERNET_TIMEOUT_MS);
        if (err == OK) {
            valid_ms = true;
            break;
        }

        master_server_url = URL();
    }

    // no master server? no point trying matrix,
    // internet is probably down
    if (!valid_ms) {
        return;
    }

    // request to connect
    String auth_token = BConfig::get_singleton()->get_setting("matrix/auth_token", "");

    // user has already logged in
    if (matrix_client.get_auth_token() != "") {

    }
    // else there are some saved credentials
    else if (auth_token != "") {
/*
        String username = BConfig::get_singleton()->get_setting("matrix/username", "");
        String password = BConfig::get_singleton()->get_setting("matrix/p", "");
        guest = BConfig::get_singleton()->get_setting("matrix/guest", false);

        if (!guest) {
            login(username, password, guest,false);
        }
        else */{
            // use existing token to login
            matrix_client.set_auth_token(auth_token);
            String matrix_user_id = BConfig::get_singleton()->get_setting("matrix/user_id", "");
            matrix_client.set_user_id(matrix_user_id);
            String matrix_sync_token = BConfig::get_singleton()->get_setting("matrix/sync_token", "");
            matrix_client.set_sync_token(matrix_sync_token);
        }
    }
    else {
        // failure? or login as guest?
        ERR_PRINT("No login credentials for matrix to attempt to connect with");
        return;
    }

    /*
    // check is auth token exists in config

    if (auth_token == "") {
        // new login

        // anonymous user - should I display a login dialog?
        ms->matrix_client.register_account(NULL, "secret_password", true);

        // store authentication info
        BConfig::get_singleton()->set_setting("matrix/auth_token", ms->matrix_client.get_auth_token());
        BConfig::get_singleton()->set_setting("matrix/user_id", ms->matrix_client.get_user_id());
        BConfig::get_singleton()->set_setting("matrix/sync_token", ms->matrix_client.get_sync_token());
    }
    else {
        // use existing token to login
        ms->matrix_client.set_auth_token(auth_token);
        String matrix_user_id = BConfig::get_singleton()->get_setting("matrix/user_id", "");
        ms->matrix_client.set_user_id(matrix_user_id);
        String matrix_sync_token = BConfig::get_singleton()->get_setting("matrix/sync_token", "");
        ms->matrix_client.set_sync_token(matrix_sync_token);
    }*/

    // join room
    room_id = BConfig::get_singleton()->get_setting("bitshift/matrix_master_server/room_id", "#trains_and_things:matrix.org");
            //GLOBAL_DEF("bitshift/matrix_master_server/room_id", "#trains_and_things:matrix.org");
    matrix_room = matrix_client.join_room(room_id);
    //ms->emit_signal("room_joined", ms->matrix_room);

    if (!matrix_room.is_valid()) {
        ERR_PRINTS("Could not join room:" + room_id);
        return;
    }

    if (connection_status == CS_DISCONNECTED) {
        return;
    }


    {
        uint64_t us = OS::get_singleton()->get_ticks_usec();

        // as soon as we start listening, we will start receiving events
        matrix_client.start_listening();

        print_line("matrix_client.start_listening (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    }

    if (connection_status == CS_DISCONNECTED) {
        return;
    }

    if (me.is_null())
        me = matrix_client.get_me();

    get_display_name(true); // retreive name from online

    // we actually dont care about syncing the state except to get the list of users in the channel
    /*
    {
        uint64_t us = OS::get_singleton()->get_ticks_usec();

        Ref<_Thread> ss = matrix_room->state_sync();
        ss->wait_to_finish();
        print_line("matrix_room.state_sync (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    }*/

    // connect signals
    matrix_room->connect("timeline_event", this, "_timeline_event");
    matrix_room->connect("old_timeline_event", this, "_old_timeline_event");
    matrix_room->connect("ephemeral_event", this, "_ephemeral_event");
    matrix_room->connect("state_event", this, "_state_event");

    //matrix_room->state_sync();


    // start thread for sending messages so it doesn't block the game
    send_text_thread = Thread::create(_send_text_message_thread_function, this);

    if (connection_status == CS_DISCONNECTED) {
        return;
    }

    connection_status = CS_CONNECTING_2;

    //Dictionary members = get_members();
    //members.get_key_list(&connected_member_keys_list);

    print_line("BMasterServer::_connect_1 (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    //emit_signal("_connect_1_complete");


    call_deferred("_connect_2");

    // now go back through the old events looking for any servers
/*
    // not sure a loop here is healthy
    need_more_host_data = true;
    //while (need_more_host_data) {
        matrix_room->get_old_events(1000); // should be suitable for getting a list of existing master servers right?
    //}

    if (need_more_host_data)
        print_line("Need to increase get_old_events"); // better to get the servers to refresh the server every hour or so?*/
}

void BMasterServer::_connect_2() {
/*
    // send fake events to minimize godot script handling all the events in the room, we only want to handle the bare essentials for performance
    // reasons as godot is slow to create the templates, so we want to minimize the amount of creation it needs to do
    Dictionary members = get_members();
    List<Variant> member_keys;
    members.get_key_list(&member_keys);

    connected_member_keys_list = member_keys;
    /*
    for (List<Variant>::Element *E = member_keys.front(); E; E = E->next()) {
        String member = E->get();
        Dictionary content = members[member];
        emit_signal("member_joined", member, content);
    }*/

    if (connection_status == CS_DISCONNECTED) {
        return;
    }

    connection_status = CS_CONNECTED;

    // pull the list of servers
    //refresh_servers();

    emit_signal("connection_complete");
    print_line("Connected to matrix master server: " + room_id);
}

#if 0
void BMasterServer::_poll() {
    // change to connecting 2 state
    if (connection_status == CS_CONNECTING_2) {
        //_connect_2();
/*
        Dictionary members = get_members();
        for (int i = 0; i < MIN(connected_member_keys_list.size(), 10); ++i) {
            String member = connected_member_keys_list.back()->get();
            connected_member_keys_list.pop_back();
            Dictionary content = members[member];
            emit_signal("member_joined", member, content);
        }

        if (connected_member_keys_list.size() <= 0) */
        {
            // start thread for sending messages so it doesn't block the game
            send_text_thread = Thread::create(_send_text_message_thread_function, this);

            connection_status = CS_CONNECTED;

            emit_signal("connection_complete");
        }

    }

    connection_status_poll_prev = connection_status;
}
#endif

void BMasterServer::connect_to_master_server() {
    // why re-connect?
    if (connection_status != CS_DISCONNECTED)
        return;

    print_line("BMasterServer::connect_to_master_server");

    //connect("_connect_1_complete", this, "_connect_2");
#if 0
    connection_status_poll_prev = connection_status;

    poll_timer = memnew(Timer);

    // add time to scene root so we can poll
    MainLoop *ml = OS::get_singleton()->get_main_loop();
    SceneTree *sml = Object::cast_to<SceneTree>(ml);
    //sml->get_root()->add_child(poll_timer);
    sml->get_root()->call_deferred("add_child", poll_timer);

    poll_timer->connect("timeout", this, "_poll");
    poll_timer->set_one_shot(false);
    poll_timer->set_wait_time(0.1);
    poll_timer->start();
#endif

    connection_status = CS_CONNECTING_1;
    //_connect_thread_function(this); // for testing multithreading issues
    connect_thread = Thread::create(_connect_thread_function, this);
}

void BMasterServer::_timeline_event(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    call_deferred("_timeline_event_deferred", event);
}

#if 0
void BMasterServer::_handle_server_data_event(Dictionary event) {
    String event_type = event["type"];
    if (event_type == "bs.room.server_data") {
        String sender = event["sender"];

        /*
        // ignore games older than an hour
        uint max_age = 1000 * 60 * 60;
        uint age = event["age"];
        if (age > max_age) {
            need_more_host_data = false;
            return;
        }*/

        /*
        // ignore my own data
        if (sender == matrix_client.get_user_id()) {
            return;
        }*/

        print_line("bs.room.server_data: " + JSON::print(event));


        String state_key = event["state_key"];

        // add sender to host data
        Dictionary server_data = event["content"];

        server_data_map_mutex->lock();

        if (server_data.empty() || server_data.has("clear")) {
            if (server_data_map.has(state_key)) {
                print_line("Removing server_data from: " + sender + "\t" + JSON::print(event)); //JSON::print(host_data_map[sender]));
                server_data_map.erase(sender);
            }
        } else {
            server_data["sender"] = sender;
            server_data["state_key"] = state_key;
            server_data["age"] = event["age"];
            event["content"] = server_data;

            print_line("Updating server_data from: " + sender + "\t" + JSON::print(event)); //JSON::print(host_data));
            server_data_map[state_key] = server_data;
        }

        server_data_map_mutex->unlock();
    }
}
#endif

void BMasterServer::_timeline_event_deferred(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    String event_type = event["type"];
/*
    _handle_server_data_event(event);
    /*
    if (event_type == "bs.room.host_data") {
        String sender = event["sender"];

        // add sender to host data
        Dictionary host_data = event["content"];
        host_data["sender"] = sender;
        event["content"] = host_data;

        if (host_data.empty() || host_data.has("clear")) {
            if (host_data_map.has(sender))
                host_data_map.erase(sender);
        } else {
            host_data_map[sender] = host_data;
        }
    }*/

    // dont care about old messages
    if (connection_status == CS_CONNECTED && event_type == "m.room.message") {
        String member = event["sender"];
        Dictionary content = (Dictionary)event["content"];
        emit_signal("message_received", member, content);
    }
/*
    // dont care about old membership changes as we send fake events after
    if (connection_status == CS_CONNECTED && event_type == "m.room.member") {
      String member = event["state_key"];

      Dictionary content = (Dictionary)event["content"];
      emit_signal("member_changed", member, content);
      /*
      if (((Dictionary)event["content"])["membership"] == "join") {

      } else if (get_members().has(member)) {
        emit_signal("member_left", member, content);
      }* /
    }

    emit_signal("timeline_event", event);*/
}


void BMasterServer::_old_timeline_event(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    String event_type = event["type"];
    //_handle_server_data_event(event);

    //call_deferred("_old_timeline_event", event);

    int nothing = 0;
    ++nothing;
}

#if 0
void BMasterServer::_old_timeline_event_deferred(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    String event_type = event["type"];

    if (event_type == "bs.room.host_data") {
        int nothing = 0;
        ++nothing;
    }

    //emit_signal("old_timeline_event", event);
}
#endif

void BMasterServer::_ephemeral_event(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    //call_deferred("_ephemeral_event_deferred", event);

    int nothing = 0;
    ++nothing;
}

#if 0
void BMasterServer::_ephemeral_event_deferred(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    //emit_signal("ephemeral_event", event);
}
#endif


void BMasterServer::_state_event(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    String event_type = event["type"];
    //_handle_server_data_event(event);

    if (event_type == "m.room.power_levels") {
        power_levels = event["content"];
        print_line("Power levels set to: " + JSON::print(power_levels));
    }

     //call_deferred("_state_event_deferred", event);
}

#if 0
void BMasterServer::_state_event_deferred(Dictionary event) {
    if (connection_status == CS_DISCONNECTED)
        return;

    String event_type = event["type"];

    if (event_type == "bs.room.server_data") {
        int nothing = 0;
        ++nothing;
    }

    // dont care about old membership changes as we send fake events after
    if (connection_status == CS_CONNECTED && event_type == "m.room.member") {
      String member = event["state_key"];

      Dictionary content = (Dictionary)event["content"];
      if (((Dictionary)event["content"])["membership"] == "join") {
        emit_signal("member_joined", member, content);
      } else {
        emit_signal("member_left", member, content);
      }
    }

    emit_signal("state_event", event);
}

#endif


BMasterServer::ConnectionStatus BMasterServer::get_connection_status() {
    return connection_status;
}

Error BMasterServer::set_display_name(String display_name) {
    if (get_display_name() == display_name) {
        return Error::OK;
    }

    if (me.is_null()) {
        return Error::FAILED; // not yet connected to matrix
    }

    print_line("set_display_name:" + display_name);
    //Ref<MatrixUser> me = matrix_client.get_me();
    return me->set_display_name(display_name);
}

String BMasterServer::get_display_name(bool sync) {
    if (me.is_null()) {
        return "";
    }

    //Ref<MatrixUser> me = matrix_client.get_me();
    String display_name = me->get_display_name(sync);
    //print_line("get_display_name:" + display_name);
    return display_name;
}

String BMasterServer::get_user_id() {
    return matrix_client.get_user_id();
}

void BMasterServer::update_server(const Dictionary &p_data) {
    server_mutex->lock();

    server_content = p_data;

    if (server_status == SS_DISCONNECTED) {
        print_line("Server started");
        server_status = SS_CONNECTED;

        // let the previous server finish
        if (update_server_thread) {
            Thread::wait_to_finish(update_server_thread);
            memdelete(update_server_thread);
            update_server_thread = NULL;
        }

        update_server_thread = Thread::create(_update_server_thread_function, this);
    }

    server_mutex->unlock();
}

void BMasterServer::_update_server_thread_function(void *self) {
    BMasterServer *ms = (BMasterServer *)self;
    ms->_update_server_thread();
}

void BMasterServer::_update_server_thread() {
    String appname = ProjectSettings::get_singleton()->get("application/config/name");
    String endpoint = master_server_url.uri + "/api.php?action=server-update";
    Dictionary content;
    content["application"] = appname;

    while (server_status != SS_DISCONNECTED) {
        server_mutex->lock();

        Variant response;
        content["content"] = server_content;
        if (server_id != -1) {
            content["server_id"] = server_id;
        }

        HTTPClient::ResponseCode err = master_server_client.blocking_request_json(HTTPClient::METHOD_POST, endpoint, content, response);
        if (err != HTTPClient::RESPONSE_OK) {
            print_line("error of some sort updating servers");
        }
        else {
            String srv_id = ((Dictionary)response)["server_id"];
            server_id = srv_id.to_int();
        }

        // server shutdown
        if (server_id == -1) {
            print_error("Server shutdown?");
            return;
        }

        print_line("Server polled");
        server_mutex->unlock();

        // wait with periodic checks incase the server was shutdown
        uint64_t wait = 480000; // 8 minutes
        uint64_t time = OS::get_singleton()->get_ticks_msec();
        while (OS::get_singleton()->get_ticks_msec() - time < wait) {
            OS::get_singleton()->delay_usec(1000000); // 1000ms

            if (server_status == SS_DISCONNECTED)
                break;
        }
    }

    // delete the master server
    server_mutex->lock();
    {
        server_content = Dictionary();

        Variant response;
        String endpoint = master_server_url.uri + "/api.php?action=server-delete";
        Dictionary content;
        content["server_id"] = server_id;

        HTTPClient::ResponseCode err = master_server_client.blocking_request_json(HTTPClient::METHOD_POST, endpoint, content, response);
        if (err != HTTPClient::RESPONSE_OK) {
            print_line("error of some sort deleting servers");
        }
        else {
            //server_id = -1;
        }

        server_id = -1;
    }
    print_line("Server deleted");
    server_mutex->unlock();
}

void BMasterServer::delete_server() {
    if (server_status == SS_DISCONNECTED)
        return;

    server_mutex->lock();
    server_status = SS_DISCONNECTED; // tell thread to shutdown
    server_mutex->unlock();
}

Array BMasterServer::get_servers() {
    return servers;
}


void BMasterServer::refresh_servers() {
    // dont allow multiple refreshing
    if (refresh_servers_thread) {
        return;
    }

    refresh_servers_thread = Thread::create(_refresh_servers_thread_function, this);
}

void BMasterServer::_refresh_servers_thread_function(void *self) {
    BMasterServer *ms = (BMasterServer *)self;
    ms->_refresh_servers();
}

void BMasterServer::_refresh_servers() {
    uint64_t us = OS::get_singleton()->get_ticks_usec();

    String appname = ProjectSettings::get_singleton()->get("application/config/name");
    Variant response;
    String endpoint = master_server_url.uri + "/api.php?action=query";
    Dictionary content;
    content["application"] = appname;
    HTTPClient::ResponseCode err = master_server_client.blocking_request_json(HTTPClient::METHOD_POST, endpoint, content, response);
    if (err != HTTPClient::RESPONSE_OK) {
        print_line("error of some sort refreshing servers");
    }

    servers = response;

    print_line("BMasterServer::_refresh_servers (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
    call_deferred("_refresh_servers_deferred");
}

void BMasterServer::_refresh_servers_deferred() {
    if (refresh_servers_thread) {
        memdelete(refresh_servers_thread);
        refresh_servers_thread = NULL;
    }
    emit_signal("refresh_servers_complete");
}

void BMasterServer::_send_text_message_thread_function(void *self) {
    BMasterServer *ms = (BMasterServer *)self;

    // sit here polling to see if there is any text, if so, send it off!
    while (ms->connection_status != CS_DISCONNECTED) {
        if (ms->send_text_list.size() > 0) {
            ms->send_text_list_mutex->lock();
            String text = ms->send_text_list[0];
            ms->send_text_list.remove(0);
            ms->send_text_list_mutex->unlock();

            ms->matrix_room->send_text_message(text);
        }
    }
}

Error BMasterServer::send_text_message(String text) {
    send_text_list_mutex->lock();
    send_text_list.push_back(text);
    send_text_list_mutex->unlock();
    return OK;
}

String BMasterServer::get_member_display_name(String id, bool sync) {
    if (!matrix_room.is_valid()) {
        ERR_PRINTS("Not in the matrix room");
        return id;
    }

    return matrix_room->get_member_display_name(id, sync);
}

Dictionary BMasterServer::get_members(bool sync) {
    return matrix_room->get_members(sync);
}

String BMasterServer::get_room_id() {
    return room_id;
}

void BMasterServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_power_levels"),&BMasterServer::set_power_levels);

    ClassDB::bind_method(D_METHOD("get_room_id"),&BMasterServer::get_room_id);
    ClassDB::bind_method(D_METHOD("get_user_id"),&BMasterServer::get_user_id);

    ClassDB::bind_method(D_METHOD("has_existing_login"),&BMasterServer::has_existing_login);
    ClassDB::bind_method(D_METHOD("login"),&BMasterServer::login);

    ClassDB::bind_method(D_METHOD("connect_to_master_server"),&BMasterServer::connect_to_master_server);
    ClassDB::bind_method(D_METHOD("get_connection_status"),&BMasterServer::get_connection_status);

    ClassDB::bind_method(D_METHOD("get_display_name"),&BMasterServer::get_display_name, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("set_display_name"),&BMasterServer::set_display_name);

    ClassDB::bind_method(D_METHOD("update_server"),&BMasterServer::update_server);
    ClassDB::bind_method(D_METHOD("delete_server"),&BMasterServer::delete_server);

    ClassDB::bind_method(D_METHOD("get_servers"),&BMasterServer::get_servers);
    ClassDB::bind_method(D_METHOD("refresh_servers"),&BMasterServer::refresh_servers);
    ClassDB::bind_method(D_METHOD("_refresh_servers_deferred"),&BMasterServer::_refresh_servers_deferred);
    ADD_SIGNAL(MethodInfo("refresh_servers_complete"));

    ClassDB::bind_method(D_METHOD("send_text_message"),&BMasterServer::send_text_message);
    ClassDB::bind_method(D_METHOD("get_member_display_name"),&BMasterServer::get_member_display_name, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("get_members"),&BMasterServer::get_members, DEFVAL(false));


    ClassDB::bind_method(D_METHOD("_timeline_event"),&BMasterServer::_timeline_event);
    ClassDB::bind_method(D_METHOD("_old_timeline_event"),&BMasterServer::_old_timeline_event);
    ClassDB::bind_method(D_METHOD("_ephemeral_event"),&BMasterServer::_ephemeral_event);
    ClassDB::bind_method(D_METHOD("_state_event"),&BMasterServer::_state_event);

    ClassDB::bind_method(D_METHOD("_timeline_event_deferred"),&BMasterServer::_timeline_event_deferred);
 /*
    ClassDB::bind_method(D_METHOD("_old_timeline_event_deferred"),&BMasterServer::_old_timeline_event_deferred);
    ClassDB::bind_method(D_METHOD("_ephemeral_event_deferred"),&BMasterServer::_ephemeral_event_deferred);
    ClassDB::bind_method(D_METHOD("_state_event_deferred"),&BMasterServer::_state_event_deferred);
*/

    //ClassDB::bind_method(D_METHOD("_poll"),&BMasterServer::_poll);
    ClassDB::bind_method(D_METHOD("_connect_2"),&BMasterServer::_connect_2);

    ADD_SIGNAL(MethodInfo("room_joined"));
    ADD_SIGNAL(MethodInfo("connection_complete"));

    // room events
    //ADD_SIGNAL( MethodInfo("timeline_event") );     //new event inserted at most recent point in timeline
    //ADD_SIGNAL( MethodInfo("old_timeline_event") ); //old event inserted at beginning of timeline
    //ADD_SIGNAL( MethodInfo("ephemeral_event") );
    //ADD_SIGNAL( MethodInfo("state_event") );

    //ADD_SIGNAL( MethodInfo("member_joined") );
    //ADD_SIGNAL( MethodInfo("member_left") );
    //ADD_SIGNAL( MethodInfo("member_changed") );
    ADD_SIGNAL( MethodInfo("message_received") );

    BIND_ENUM_CONSTANT(CS_DISCONNECTED);
    BIND_ENUM_CONSTANT(CS_CONNECTING_1);
    BIND_ENUM_CONSTANT(CS_CONNECTING_2);
    BIND_ENUM_CONSTANT(CS_CONNECTED);
}

BMasterServer::BMasterServer() {
    server_id = -1;
    singleton = this;
    connect_thread = NULL;
    send_text_thread = NULL;
    refresh_servers_thread = NULL;
    update_server_thread = NULL;
    connection_status = CS_DISCONNECTED;
    server_status = SS_DISCONNECTED;
    guest = false;

    send_text_list_mutex = Mutex::create();
    server_mutex = Mutex::create();

    String hs_name = BConfig::get_singleton()->get_setting("bitshift/matrix_master_server/hs_name", "matrix.org");
    matrix_client.set_hs_name(hs_name);
}

BMasterServer::~BMasterServer() {
    uint64_t us = OS::get_singleton()->get_ticks_usec();

    // disconnect signals
    if (matrix_room.is_valid()) {
        matrix_room->disconnect("timeline_event", this, "_timeline_event");
        matrix_room->disconnect("old_timeline_event", this, "_old_timeline_event");
        matrix_room->disconnect("ephemeral_event", this, "_ephemeral_event");
        matrix_room->disconnect("state_event", this, "_state_event");
    }

    connection_status = CS_DISCONNECTED;

    if (refresh_servers_thread) {
        Thread::wait_to_finish(refresh_servers_thread);
    }

    // delete any running server
    if (server_id != -1) {
        delete_server();
    }

    if (update_server_thread) {
        Thread::wait_to_finish(update_server_thread);
        memdelete(update_server_thread);
        update_server_thread = NULL;
    }

    if (connect_thread) {
        Thread::wait_to_finish(connect_thread);
        memdelete(connect_thread);
        connect_thread = NULL;
    }

    if (send_text_thread) {
        Thread::wait_to_finish(send_text_thread);
        memdelete(send_text_thread);
        send_text_thread = NULL;
    }

    print_line("deleted threads");

    matrix_client.stop_listening();
/*
    print_line("~~~ 1");

    if (matrix_room.is_valid())
        matrix_room->leave_room();

    print_line("~~~ 2");

    //matrix_client.logout();

    print_line("closed down matrix");
*/
    if (send_text_list_mutex) {
        memdelete(send_text_list_mutex);
        send_text_list_mutex = NULL;
    }

    if (server_mutex) {
        memdelete(server_mutex);
        server_mutex = NULL;
    }

    print_line("BMasterServer::~BMasterServer (seconds): " + rtos((OS::get_singleton()->get_ticks_usec() - us) / 1000000.0));
}
