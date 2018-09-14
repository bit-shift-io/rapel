
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
#include "binternet.h"
#include "core/io/http_client.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include "core/io/json.h"
#include "core/vector.h"
#include <assert.h>

#define INTERNET_TIMEOUT_MS 1000

BInternet *BInternet::singleton=NULL;

void BInternet::init() {
    if (internet_status != IS_DISCONNECTED) {
        return;
    }

    internet_status = IS_CHECKING;
    init_thread = Thread::create(_init_thread_func, this);
}

void BInternet::_init_thread_func(void* arg) {
    BInternet *net = (BInternet *)arg;
    net->_init();
}

void BInternet::_init() {
    HTTPClient http;
    Error err = http.connect_to_host("ipapi.co", 443 /*80*/, true, true); // Connect to host/port
    if (err != OK) {
        print_line("Could not connect to: ipapi.co");
        internet_status = IS_UNAVILABLE;
        return;
    }

    uint64_t waitStartTime = OS::get_singleton()->get_ticks_msec();

    // Wait until resolved and connected
    while( http.get_status()==HTTPClient::STATUS_CONNECTING || http.get_status()==HTTPClient::STATUS_RESOLVING) {
        http.poll();

        // timeout
        uint64_t now = OS::get_singleton()->get_ticks_msec();
        uint64_t waitTime = (now - waitStartTime);
        if (waitTime > INTERNET_TIMEOUT_MS) {
            internet_status = IS_UNAVILABLE;
            return;
        }

        // shutdown by a call to quit
        if (internet_status == IS_DISCONNECTED) {
            return;
        }
    }

    if ( http.get_status() != HTTPClient::STATUS_CONNECTED ) { // Could not connect
        internet_status = IS_UNAVILABLE;
        return;
    }

    // Some headers
    Vector<String> headers;
    headers.insert(0, "Content-Type: application/json");
    headers.insert(0, "Accept: application/json");

    //headers.insert(0, "User-Agent: Pirulo/1.0 (Godot)");
    //headers.insert(0, "Accept: */*");

    err = http.request(HTTPClient::METHOD_GET,"/json",headers); // Request a page from the site (this one was chunked..)
    assert( err == OK ); // Make sure all is OK

    while (http.get_status() == HTTPClient::STATUS_REQUESTING) {
        // Keep polling until the request is going on
        http.poll();
    }

    assert( http.get_status() == HTTPClient::STATUS_BODY or http.get_status() == HTTPClient::STATUS_CONNECTED ); // Make sure request finished well.

    if (http.has_response()) {
        PoolByteArray response_body_array = PoolByteArray(); // Array that will hold the data
        while(http.get_status()==HTTPClient::STATUS_BODY) {
            // While there is body left to be read
            http.poll();
            PoolByteArray chunk = http.read_response_body_chunk(); // Get a chunk
            if (chunk.size()!=0) {
                response_body_array.append_array(chunk); // Append to read buffer
            }
        }

        String response_body;
        if (response_body_array.size() >= 0) {
          PoolByteArray::Read r = response_body_array.read();
          response_body.parse_utf8((const char *)r.ptr(), response_body_array.size());
        }

        Variant r_ret;
        String r_err_str;
        int r_err_line;
        err = JSON::parse(response_body, r_ret, r_err_str, r_err_line);
        assert( err == OK ); // Make sure all is OK

        ip_api = r_ret;

        String ip_str = ip_api["ip"];
        if (ip_str.is_valid_ip_address()) {
            ip = IP_Address(ip_str);
        }
    }
}

IP_Address BInternet::get_internet_ip() {
    init();
    Thread::wait_to_finish(init_thread);
    return ip;
}

String BInternet::get_country() {
    init();
    Thread::wait_to_finish(init_thread);
    return ip_api["country_name"];
}

String BInternet::get_city() {
    init();
    Thread::wait_to_finish(init_thread);
    return ip_api["city"];
}

String BInternet::get_continent()
{
    init();
    Thread::wait_to_finish(init_thread);

    String continent_code = ip_api["continent_code"];
    if (continent_code == "AF")
        return "Africa";
    if (continent_code == "AN")
        return "Antarctica";
    if (continent_code == "AS")
        return "Asia";
    if (continent_code == "EU")
        return "Europe";
    if (continent_code == "NA")
        return "North America";
    if (continent_code == "OC")
        return "Oceania";
    if (continent_code == "SA")
        return "South America";

    return "";
}

void BInternet::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init"),&BInternet::init);
    ClassDB::bind_method(D_METHOD("get_internet_ip"),&BInternet::get_internet_ip);
    ClassDB::bind_method(D_METHOD("get_country"),&BInternet::get_country);
    ClassDB::bind_method(D_METHOD("get_city"),&BInternet::get_city);
    ClassDB::bind_method(D_METHOD("get_continent"),&BInternet::get_continent);
}

BInternet::BInternet() {
    singleton = this;
    internet_status = IS_DISCONNECTED;
    init_thread = NULL;
}

BInternet::~BInternet() {
    internet_status = IS_DISCONNECTED;
    if (init_thread) {
        Thread::wait_to_finish(init_thread);
        memdelete(init_thread);
        init_thread = NULL;
    }
}
