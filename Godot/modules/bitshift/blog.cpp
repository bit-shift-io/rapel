 
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
#include "blog.h"
#include "butil.h"
#include <assert.h>

BLog *BLog::singleton=NULL;

void BLog::_print_handler(void *p_this, const String &p_string, bool p_error) {
    BLog *log = (BLog *)p_this;
    log->_add_message(p_string, p_error);
}

void BLog::print(const String &p_string) {
    print_line(p_string);
    BUtil::get_singleton()->log_editor_message(p_string);
}

void BLog::_add_message(const String &p_string, bool p_error) {
    if (get_script_instance() && get_script_instance()->has_method("_add_message")) {
        get_script_instance()->call("_add_message", p_string, p_error);
    }
}

void BLog::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("add_message"),&BLog::_add_message);
    ClassDB::bind_method(D_METHOD("print"),&BLog::print);

    BIND_VMETHOD(MethodInfo(Variant::REAL, "_add_message", PropertyInfo(Variant::STRING, "string"), PropertyInfo(Variant::BOOL, "error")));
}
/*
void BConsole::log_editor_message(const String& msg) {
#ifdef TOOLS_ENABLED
    if (EditorNode::get_singleton() && EditorNode::get_log())
        EditorNode::get_log()->add_message(msg);
#endif
}
*/
BLog::BLog() {
    singleton = this;

    print_handler.printfunc = _print_handler;
    print_handler.userdata = this;
    add_print_handler(&print_handler);
}

BLog::~BLog() {
    remove_print_handler(&print_handler);
}
