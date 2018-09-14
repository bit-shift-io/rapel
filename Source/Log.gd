#
# This file has been modified by bitshift (http://bit-shift.io)
# Modified version of GodotLogger by Spooner
# ======================
#
# Log.gd is a simple logging system. It allows for more formatted logging,
# logging levels and logging to a file.
#
# Installation
# ------------
#
#     Place this file somewhere (for example, 'res://root/logger.gd')
#     and autoload it (in project settings) to make it into a globally accessible singleton.
#
# Logger levels
# -------------
#
#     Level.DEBUG - Show all log messages
#     Level.INFO - Show info(), warning(), error() and critical() log messages [DEFAULT]
#     Level.WARNING - Show warning(), error() and critical() log messages
#     Level.ERROR - Show error() and critical() log messages
#     Level.CRITICAL - Show only critical() log messages
#
# Time formats
# ------------
#
#     TimeFormat.NONE
#     TimeFormat.DATETIME [YYYY-MM-DD HH:MM:SS.mmm]
#     TimeFormat.TIME [HH:MM:SS.mmm]
#     TimeFormat.ELAPSED [H:MM:SS.mmm]
#
# Examples
# --------
#
#     Getting a reference to the global logger object with:
#         var logger = get_node('/root/logger')
# 
#     Setting the logger level (default is Level.INFO):
#         logger.level = logger.Level.DEBUG
#
#     Setting whether to print() message (default is to print):
#         logger.print_std = false
#
#     Setting showing the current elapsed time (defaults to show TimeFormat.DATETIME):
#         logger.time_format = TimeFormat.ELAPSED
#
#     Setting time formatter to use your own function (which would normally be called as my_instance.time_formatter()):
#         logger.time_format_func = funcref(my_instance, "time_formatter")
#
#     Logging to a file (set to 'null' to close the file):
#         logger.filename = 'user://log.txt'
#        
#     Logging messages of various types (will use var2str() to output any non-string being logged):
#         logger.info("Creating a new fish object")
#         logger.debug([my_vector3, my_vector2, my_list])
#         logger.warning("Tried to take over the moon!")
#         logger.error("File doesn't exist, so I can't go on")
#         logger.critical("Divided by an ocelot error! Segfault immanent")
#

extends BLog

# Levels of debugging available
class Level:
	const DEBUG = 0
	const INFO = 1 # default
	const WARNING = 2
	const ERROR = 3
	const CRITICAL = 4
	const ASSERT = 5

# Built in time formatters
class TimeFormat:
    const NONE = 0
    const ELAPSED = 1
    const TIME = 2
    const DATETIME = 3 # default

const STRING_TYPE = typeof("") # TODO: Is there a nicer way to avoid creating a string every time we compare an object?

var break_on_assert = true;
var ignore_add_message = false;

# Print to stdout?
var print_stdout = true setget set_print_stdout, get_print_stdout
func get_print_stdout():
    return print_stdout
func set_print_stdout(value):
    assert(value in [true, false])
    print_stdout = value

# Logging level.
var fileLevel = Level.INFO setget set_filelevel, get_filelevel
func get_filelevel():
    return fileLevel
func set_filelevel(value):
    assert(fileLevel in [Level.DEBUG, Level.INFO, Level.WARNING, Level.ERROR, Level.CRITICAL])
    fileLevel = value

# Logging to file.
var file = null
var file_name = null setget set_filename, get_filename

func get_filename():
    return file_name
	
func set_filename(value):
    if file != null:
        info("Stopped logging to file: %s" % file_name)
        file.close()

    if value != null:
        file = File.new()
        file_name = value
        file.open(file_name, File.WRITE)
        info("Started logging to file: %s" % file_name)
    else:
        file = null
        file_name = null  

# Log timer
var time_format_func = funcref(self, "format_time_datetime") setget set_time_format_func
func set_time_format_func(value):
    time_format_func = value

var time_format = TimeFormat.DATETIME setget set_time_format
func set_time_format(value):
    if value == TimeFormat.NONE:
        self.time_format_func = funcref(self, "format_time_none")
    elif value == TimeFormat.DATETIME:
        self.time_format_func = funcref(self, "format_time_datetime")
    elif value == TimeFormat.TIME:
        self.time_format_func = funcref(self, "format_time_time")
    elif value == TimeFormat.ELAPSED:
        self.time_format_func = funcref(self, "format_time_elapsed")
    else:
        assert(false) # Bad time format used.

# --- Time formatters for use by the logger.

func format_time_none():
    return ""
    
func format_time_elapsed(time):
    return "[%s] " % _format_elapsed(time)

func format_time_time():
    return "[%s] " % _format_time()

func format_time_datetime():
    return "[%s %s] " % [_format_date(), _format_time()]

# --- General time formatting functions

func _format_time():
    """Not used directly, but might come in useful"""
    var time = OS.get_time()
    # This is not "correct", but gives impression of ms moving on!
    var ms = OS.get_ticks_msec() % 1000

    return "%02d:%02d:%02d.%03d" % [time["hour"], time["minute"], time["second"], ms]

func _format_elapsed(time):
    """Not used directly, but might come in useful"""
    #var time = OS.get_ticks_msec()
    var ms = time % 1000
    var s = int(time / 1000) % 60
    var m = int(time / 60000) % 60
    var h = int(time / 3600000)

    return "%d:%02d:%02d.%03d " % [h, m, s, ms]
func _format_date():
    """Not used directly, but might come in useful"""
    var date = OS.get_date()

    return "%d-%02d-%02d" % [date["year"], date["month"], date["day"]]

# --- Message writing methods

func debug(data):
	#var frame = get_stack()[1];
	#var data_str = "%30s:%-4d %s" % [frame.source.get_file(), frame.line, data];
	var data_str = data;
	_write('DEBUG:', data_str, "aqua", Level.DEBUG)
	   
func info(data):
	_write('INFO:', data, "white", Level.INFO)

func warning(data):
	_write('WARN:', data, "yellow", Level.WARNING)

func error(data):
	_write('ERROR:', data, "red", Level.ERROR)

func assert(condition, data):
	if (condition != null):
		if (typeof(condition) == TYPE_OBJECT || typeof(condition) == TYPE_STRING):
			return;
	
	if (condition == null || bool(condition) == false):
		_write('ASSERT:', "Assert: " + data , "red", Level.ASSERT) # + "\n" + get_stack()
		if (break_on_assert):
			assert(false);
	
func critical(data):
	_write('CRIT:', data, "red",  Level.CRITICAL)

func _write(type, data, colour, level):	
	"""Actually write out the message string"""
	if typeof(data) != STRING_TYPE:
	    data = var2str(data)

	if print_stdout:
		ignore_add_message = true; # stop _add_message doing anything
		print(data) 
		ignore_add_message = false;
		
	_write_to_console(type, data, colour, level);
	_write_to_log(type, data, colour, level);
	
	
func _write_to_console(type, data, colour, level):
	if (Console != null):
		Console.append_bbcode("[color=" + colour + "]" + data + "[/color]\n")
	
func _write_to_log(type, data, colour, level):
	if file != null && self.fileLevel < level:
		var message = '%s%5s %s' % [time_format_func.call_func(), type, data]
		file.store_line(message)

func _init():
	set_filename("res://log.txt")
	
func _ready():
	Console.register_command("logdebug", {
		method = funcref(self, "logDebug"),
		description = "log debug messages to log",
		args = "",
		num_args = 0
	})
	
# console command to enable logging debug info to file
func logDebug():
	fileLevel = Level.DEBUG
	debug("Logging debug\n")
	
# message received from C++ print_line or print_error
func _add_message(p_string, p_error):
	if (ignore_add_message):
		return;
		
	var p = print_stdout; # stop the print occuring in _write
	print_stdout = false;
		
	if (p_error):
		_write("NATIVE ERROR:", p_string, "aqua", Level.ERROR);
	else:
		_write("NATIVE:", p_string, "aqua", Level.DEBUG);
		
	print_stdout = p;
