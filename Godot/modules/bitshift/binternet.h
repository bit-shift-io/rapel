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
#ifndef BINTERNET_H
#define BINTERNET_H

#include "core/reference.h"

class Thread;

/**
        @author Fabian Mathews <supagu@gmail.com>

        Util class to obtain internet ip address info
*/

class BInternet : public Reference {

    GDCLASS(BInternet,Reference)

    static BInternet *singleton;

protected:

    enum InternetStatus {
        IS_DISCONNECTED = 0,
        IS_CHECKING = 1,
        IS_AVAILABLE = 2,
        IS_UNAVILABLE = 3,
    };

    Dictionary ip_api;
    IP_Address ip;
    InternetStatus internet_status;

    Thread* init_thread;

    static void _init_thread_func(void* arg);
    static void _bind_methods();

    void _init();

public:

    IP_Address get_internet_ip();
    String get_country();
    String get_city();
    String get_continent();

    void init();

    BInternet();
    ~BInternet();

    static BInternet *get_singleton() { return singleton; }

};

#endif // BINTERNET_IP_H
