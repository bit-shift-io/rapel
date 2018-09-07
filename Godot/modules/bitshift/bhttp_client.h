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
#ifndef BHTTP_CLIENT_H
#define BHTTP_CLIENT_H

#include "core/io/http_client.h"


class URL {
public:

    String protocol;
    String host;
    int port;
    String uri;

    String url;

    URL(const String& p_url = "");
};

/**
    @author Fabian Mathews <supagu@gmail.com>
*/

class BHttpClient : public HTTPClient {

    GDCLASS(BHttpClient, HTTPClient)

protected:

    String host;
    int port;
    bool ssl;
    bool verify_host;

    static void _bind_methods();

public:

    Error connect_to_host(const String &p_host = "", int p_port = -1, bool p_ssl = false, bool p_verify_host = true, uint64_t timeout_ms = -1);

    HTTPClient::ResponseCode blocking_request(HTTPClient::Method method, const String& endpoint, const String& body, String &response_body);
    HTTPClient::ResponseCode blocking_request(HTTPClient::Method method, const String& endpoint, const String& body);

    HTTPClient::ResponseCode blocking_request_json(HTTPClient::Method method, const String& endpoint, const Dictionary& body, Variant &response_body);
    HTTPClient::ResponseCode blocking_request_json(HTTPClient::Method method, const String& endpoint, const Dictionary& body);

    BHttpClient();
    ~BHttpClient();
};

#endif // BHTTP_CLIENT_H
