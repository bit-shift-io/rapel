 
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
#include "brss.h"
#include "core/io/xml_parser.h"
#include "core/io/http_client.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include "core/version.h"
#include <assert.h>

#define INTERNET_TIMEOUT_MS 1000

// XML parse
// https://godotengine.org/qa/10000/how-do-xmlparser-work-rss-example

// Threaded http:
// https://godotengine.org/qa/9902/how-to-download-a-file-from-the-internet-using-gdscript

void BRSS::parse_url(String p_domain, String p_url, int p_port, bool p_useSSL) {

    assert(parse_url_thread == NULL);

    domain = p_domain;
    url = p_url;
    port = p_port;
    useSSL = p_useSSL;

    parse_url_thread = Thread::create(_parse_url_thread_function, this);
}

void BRSS::_parse_url_thread_function(void *self) {
    BRSS *rss = (BRSS *)self;

    HTTPClient client;
    Error err = client.connect_to_host(rss->domain, rss->port, rss->useSSL, true);
    if (err != OK) {
        print_line("Could not connect to: " + rss->url);
        //return "Could not connect to: " + p_url;
        return;
    }

    uint64_t waitStartTime = OS::get_singleton()->get_ticks_msec();

    while (client.get_status()==HTTPClient::STATUS_CONNECTING or client.get_status()==HTTPClient::STATUS_RESOLVING) {
        client.poll();

        // timeout
        uint64_t now = OS::get_singleton()->get_ticks_msec();
        uint64_t waitTime = (now - waitStartTime);
        if (waitTime > INTERNET_TIMEOUT_MS) {
            return;
        }
    }

    HTTPClient::Status status = client.get_status();
    //assert(client.get_status() == HTTPClient::STATUS_CONNECTED);

    Vector<String> headers;
    headers.push_back("User-Agent: GodotEngine/" + String(VERSION_FULL_BUILD) + " (" + OS::get_singleton()->get_name() + ")");
    headers.push_back("Accept: */*");

    err = client.request(HTTPClient::METHOD_GET, rss->url, headers);
    if (err != OK) {
        print_line("Failed to request: " + rss->url);
        //return "Failed to request /api/1/jwt/me";
        return;
    }

    while (client.get_status() == HTTPClient::STATUS_REQUESTING) {
        client.poll();
    }

    if (client.get_status() != HTTPClient::STATUS_BODY and client.get_status() != HTTPClient::STATUS_CONNECTED) {
        print_line("Status error:" + String::num(client.get_status()));
        return; // "Status error";
    }

    String response_body;
    while (client.get_status()==HTTPClient::Status::STATUS_REQUESTING) {
      client.poll();
    }
    PoolByteArray response_body_array = PoolByteArray();

    while (client.get_status()==HTTPClient::Status::STATUS_BODY) {
      client.poll();
      response_body_array.append_array(client.read_response_body_chunk());
    }

    if (response_body_array.size() >= 0) {
      PoolByteArray::Read r = response_body_array.read();
      response_body.parse_utf8((const char *)r.ptr(), response_body_array.size());
    }

    _File f;
    f.open(String("res://rss.xml"), _File::WRITE);
    f.store_string(response_body);
    f.close();
       /*

    int downloaded = 0;
    http.poll();
    PoolByteArray chunk = http.read_response_body_chunk();
    downloaded+=chunk.size();

    ByteArray* ba = reinterpret_cast<ByteArray*>(&chunk);
    String s;
    if (ba->size()>=0) {
        ByteArray::Read r = ba->read();
        s.parse_utf8((const char*)r.ptr(),ba->size());
    }
*/

    PoolByteArray::Read r = response_body_array.read();
    XMLParser parser;
    //xml.open_buffer(r.ptr()); //response_body);
    err = parser.open(String("res://rss.xml"));
    if (err != OK) {
        print_line("Could not open: res://rss.xml");
        return;
    }

    //Vector<Dictionary> entries;
    Array entries;
    while (parser.read() == OK) {
        if (parser.get_node_type() == XMLParser::NODE_ELEMENT) {
            String name = parser.get_node_name();
            if (name == "entry") {
                entries.push_back(rss->_parse_entry(parser));
            }
        }
    }

    rss->emit_signal("parse_url_complete", entries);
}

Dictionary BRSS::_parse_entry(XMLParser &parser) {
    Dictionary entry;
    while (parser.read() == OK) {

        if (parser.get_node_type() == XMLParser::NODE_ELEMENT) {
            String name = parser.get_node_name();

            if (name == "link") {
                if (parser.has_attribute("rel")) {
                    String rel_value = parser.get_attribute_value("rel");
                    String type_value = parser.get_attribute_value("type");
                    if (rel_value == "alternate") {
                        String href = parser.get_attribute_value("href");
                        entry["link"] = href;
                    }
                }
            } else if (name == "title") {
                parser.read();
                String node_data = parser.get_node_data();
                entry["title"] = node_data;
            } else if (name == "published") {
                parser.read();
                String node_data = parser.get_node_data();
                entry["published"] = node_data;
            } else if (name == "content") {
                parser.read();
                String node_data = parser.get_node_data();
                entry["content"] = node_data;
            }

        } else if (parser.get_node_type() == XMLParser::NODE_ELEMENT_END && parser.get_node_name() == "entry") {
            break; //end of <asset>
        }
    }

    return entry;
}

void BRSS::_bind_methods() {
    ClassDB::bind_method(D_METHOD("parse_url"),&BRSS::parse_url);

    ADD_SIGNAL(MethodInfo("parse_url_complete"));
}

BRSS::BRSS() {
    parse_url_thread = NULL;
}

BRSS::~BRSS() {
    if (parse_url_thread) {
        Thread::wait_to_finish(parse_url_thread);
        memdelete(parse_url_thread);
        parse_url_thread = NULL;
    }
}
