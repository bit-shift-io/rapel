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
#ifndef BRSS_H
#define BRSS_H

#include "core/reference.h"

class Thread;
class XMLParser;

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BRSS : public Reference {

    GDCLASS(BRSS,Reference)

protected:

    static void _bind_methods();

    Dictionary _parse_entry(XMLParser &parser);

    static void _parse_url_thread_function(void *self);

    Thread* parse_url_thread;

    String domain;
    String url;
    int port;
    bool useSSL;

public:

    void parse_url(String p_domain, String p_url, int p_port, bool p_useSSL);

    BRSS();
    ~BRSS();

};

#endif // BRSS_H
