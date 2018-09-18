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
#ifndef BLOG_H
#define BLOG_H

#include "core/reference.h"
#include "scene/main/node.h"
#include "core/print_string.h"

/**
	@author Fabian Mathews <supagu@gmail.com>
*/

class BLog : public Node {

    GDCLASS(BLog, Node)

    static BLog *singleton;

    PrintHandlerList print_handler;
    static void _print_handler(void *p_this, const String &p_string, bool p_error);

protected:

	static void _bind_methods();

public:

    BLog();
    ~BLog();

    virtual void _add_message(const String &p_string, bool p_error);

    void print(const String &p_string);

    static BLog *get_singleton() { return singleton; }
};

#endif // BLOG_H
