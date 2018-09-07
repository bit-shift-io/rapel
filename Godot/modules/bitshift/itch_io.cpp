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
#include "itch_io.h"
#include "core/io/http_client.h"
#include "core/bind/core_bind.h"
#include "core/ustring.h"
#include <assert.h>

/*
 * 
 * If itch authenticates, we are returned a JSON string like so:
 
	{
	"user": {
		"gamer": false,
		"id": 507713,
		"url": "https:\/\/bitshift.itch.io",
		"username": "bitshift",
		"developer": true,
		"press_user": false
	}
	}

	we grab the username for our game, and maybe an email address?
 * 
 * */

void ItchIo::_bind_methods() {
    
        ClassDB::bind_method(D_METHOD("get_api_key"),&ItchIo::get_api_key);
        ClassDB::bind_method(D_METHOD("auth"),&ItchIo::auth);
        ClassDB::bind_method(D_METHOD("get_username"),&ItchIo::get_username);
}

String ItchIo::get_api_key() {
    _OS os;
    return os.get_environment("ITCHIO_API_KEY");
}

String ItchIo::auth() {
	
#if 0
	_OS os;
	
	HTTPClient http;
	Error err = http.connect("https://itch.io", 443, true, false); //true);
	if (err != OK) {
            print_line("Could not connect to https://itch.io");
            return "Could not connect to https://itch.io";
	}
	
	while (http.get_status()==HTTPClient::STATUS_CONNECTING or http.get_status()==HTTPClient::STATUS_RESOLVING) {
            http.poll();
            os.delay_msec(500);
	}
	
	assert(http.get_status() == HTTPClient::STATUS_CONNECTED);
	
	print_line("itch key: " + get_api_key());
	Vector<String> headers;
	headers.push_back("Authorization: " + get_api_key());
	
	err = http.request(HTTPClient::METHOD_GET,"/api/1/jwt/me", headers);
        if (err != OK) {
            print_line("Failed to request /api/1/jwt/me");
            return "Failed to request /api/1/jwt/me";
        }
	
        while (http.get_status() == HTTPClient::STATUS_REQUESTING) {
            http.poll();
            os.delay_msec(500);
	}
	
        if (http.get_status() != HTTPClient::STATUS_BODY and http.get_status() != HTTPClient::STATUS_CONNECTED) {
            print_line("Status error:" + String::num(http.get_status()));
            return "Status error";
        }

        int downloaded = 0;
        http.poll();
        ByteArray chunk = http.read_response_body_chunk();
        downloaded+=chunk.size();
        
        ByteArray* ba = reinterpret_cast<ByteArray*>(&chunk);
        String s;
        if (ba->size()>=0) {
                ByteArray::Read r = ba->read();
                s.parse_utf8((const char*)r.ptr(),ba->size());
        }

        Dictionary dict; 
        dict.parse_json(s);
        if (!dict.has("user")) {
            return "Bad response, not running through itch";
        }
        
        Dictionary user = dict.get_valid("user");
        String username = user.get_valid("username");
        
        authenticated = true;
        print_line("Client running through itch logged in as: " + username);
	return "Client running through itch logged in as: " + username;
#endif
        return "FAIL";
}

String ItchIo::get_username()
{
    return username;
}

ItchIo::ItchIo() {
    authenticated = false;
}


ItchIo::~ItchIo() {

}
