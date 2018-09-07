
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
#include "bhttp_client.h"
#include "butil.h"
#include "core/io/json.h"
#include "modules/regex/regex.h"
#include <assert.h>

URL::URL(const String& p_url) {
    url = p_url;
    port = 80;
    protocol = "http";

    if (p_url.length() <= 0)
        return;

    /*
    RegEx e("(https?:\/\/)([^:^\/]*)(:\\d*)?(.*)?");

    Ref<RegExMatch> p = e.search(url);

    Array strings = p->get_strings();

    for (int i = 0; i < strings.size(); ++i) {
        print_line(strings[i]);
    }

    if (strings.size() > 1)
        protocol = (strings[1] == "https://") ? "http" : "https";

    if (strings.size() > 2)
        host = strings[2];

    if (strings.size() > 3)
        host = strings[3];
*/

    protocol = p_url.begins_with("https://") ? "https" : "http";
    String remaining = p_url.replace("https://", "");
    remaining = remaining.replace("http://", "");

    int start_port = remaining.find(":");
    int start_slash = remaining.find("/");

    if (start_port != -1 && start_port < start_slash) {
        String port_str = remaining.substr(start_port + 1, start_slash);
        port = port_str.to_int();
        host = remaining.substr(0, start_port);
    }
    else {
        host = remaining.substr(0, start_slash);
    }

    uri = remaining.substr(start_slash, remaining.length());

    /*
    Array results = e.search_all(url);

    Ref<RegExMatch> p = results[0];

    print_line(p.get_string("1"));
*/
    //protocol = results[0].

    int nothing = 0;
    ++nothing;
}

HTTPClient::ResponseCode BHttpClient::blocking_request(HTTPClient::Method method, const String& endpoint, const String& body, String &response_body) {

  poll();

  if (
      get_status() == HTTPClient::Status::STATUS_CONNECTION_ERROR ||
      get_status() == HTTPClient::Status::STATUS_DISCONNECTED ||
      get_status() == HTTPClient::Status::STATUS_CONNECTION_ERROR ||
      get_status() == HTTPClient::Status::STATUS_SSL_HANDSHAKE_ERROR
      ) {
    connect_to_host();
  }

  while (get_status()!=HTTPClient::Status::STATUS_CONNECTED) {
    poll();
  }

  Vector<String> headers = Vector<String>();
  headers.insert(0, "Content-Type: application/json");
  headers.insert(0, "Accept: application/json");

  String::num_int64((int)HTTPClient::request(method, endpoint, headers, body));
  while (get_status()==HTTPClient::Status::STATUS_REQUESTING) {
    poll();
  }
  PoolByteArray response_body_array = PoolByteArray();

  while (get_status()==HTTPClient::Status::STATUS_BODY) {
    poll();
    response_body_array.append_array(read_response_body_chunk());
  }

  if (response_body_array.size() >= 0) {
    PoolByteArray::Read r = response_body_array.read();
    response_body.parse_utf8((const char *)r.ptr(), response_body_array.size());
  }

  print_line(endpoint);
  print_line(response_body);

  bool keep_alive = false;
  List<String> response_headers;
  get_response_headers(&response_headers);
  for (int i = 0; i < response_headers.size(); ++i) {
      print_line(response_headers[i]);
      if (response_headers[i] == "Connection: Keep-Alive") {
        keep_alive = true;
      }
  }

  HTTPClient::ResponseCode result = (HTTPClient::ResponseCode)get_response_code();
  if (!keep_alive) {
      close();
  }

  return result;
}

HTTPClient::ResponseCode BHttpClient::blocking_request(HTTPClient::Method method, const String& endpoint, const String& body) {
  String response;
  return blocking_request(method, endpoint, body, response);
}

HTTPClient::ResponseCode BHttpClient::blocking_request_json(HTTPClient::Method method, const String& endpoint, const Dictionary& body, Variant &response_body) {
  String body_json;
  if (!body.empty()) {
    body_json = JSON::print(body);
  }
  String response_json;
  String e =  endpoint; //+(endpoint.find("?")==-1?"?":"&"); //+(auth?"access_token="+auth_token:String());

  print_line(e);
  print_line(body_json);

  HTTPClient::ResponseCode http_status = blocking_request(method, e, body_json, response_json);

  if (http_status == 200) {

    print_line(response_json);

    Variant response_variant = Variant();
    String r_err_str;
    int32_t r_err_line;
    Error parse_err = JSON::parse(response_json, response_variant, r_err_str, r_err_line);
    if (parse_err) { WARN_PRINT("JSON parsing error! "+parse_err); }
    response_body = response_variant;
  }

  return http_status;
}

HTTPClient::ResponseCode BHttpClient::blocking_request_json(HTTPClient::Method method, const String& endpoint, const Dictionary& body) {
  Variant response;
  return blocking_request_json(method, endpoint, body, response);
}

Error BHttpClient::connect_to_host(const String &p_host, int p_port, bool p_ssl, bool p_verify_host, uint64_t timeout_ms) {
    if (p_host != "") {
        host = p_host;
        port = p_port;
        ssl = p_ssl;
        verify_host = p_verify_host;
    }

    Error err = HTTPClient::connect_to_host(host, port, ssl, verify_host);
    if (err != OK)
        return err;

    uint64_t waitStartTime = OS::get_singleton()->get_ticks_msec();
    uint64_t now;
    while (get_status() == STATUS_RESOLVING) {
        poll();

        // timeout
        if (timeout_ms != -1) {
            now = OS::get_singleton()->get_ticks_msec();
            uint64_t waitTime = (now - waitStartTime);
            if (waitTime > timeout_ms) {
                return ERR_TIMEOUT;
            }
        }
    }

    switch (get_status()) {
    case STATUS_CANT_RESOLVE:
        return ERR_CANT_RESOLVE;
    case STATUS_CANT_CONNECT:
        return ERR_CANT_CONNECT;
    default:
        break;
    }

    return err;
}

void BHttpClient::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("add_message"),&BLog::_add_message);
}

BHttpClient::BHttpClient() {
}

BHttpClient::~BHttpClient() {

}
