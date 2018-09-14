#
# This file has been modified by bitshift (http://bit-shift.io)
# Original copyright notice:
#
# Godot Console main script
# Copyright (c) 2016 Hugo Locurcio and contributors - MIT license
#

extends CanvasLayer

const MAX_LINES = 100 # controls how much the console displays

signal set_console_open(open);

onready var console_text = get_node("Panel/Container/ConsoleText")
var line_history = [] # a list of lengths between \n characters
var last_line_complete = true
var cmd_history = []
var cmd_history_count = 0
var cmd_history_up = 0
# All recognized commands
var commands = {}
# All recognized cvars
var cvars = {}

var g_player = null

func _ready():
	# Allow selecting console text
	console_text.set_selection_enabled(true)
	# Follow console output (for scrolling)
	console_text.set_scroll_follow(true)
	# Don't allow focusing on the console text itself
	console_text.set_focus_mode(console_text.FOCUS_NONE)

	set_process_input(true)
	set_console_open(false);
	
	# By default we show help
	append_bbcode("[color=red]" + BConfig.get_setting("application/name", "") + "[/color] - [color=yellow]cmdlist[/color] to get a list of all commands\n")

	# Register built-in commands
	register_command("echo", {
		description = "Prints a string in console",
		args = "<string>",
		num_args = 1
	})
	
	register_command("history", {
		description = "Print all previous cmd used during the session",
		args = "",
		num_args = 0
	})

	register_command("cmdlist", {
		description = "Lists all available commands",
		args = "",
		num_args = 0
	})

	register_command("cvarlist", {
		description = "Lists all available cvars",
		args = "",
		num_args = 0
	})

	register_command("help", {
		description = "Outputs usage instructions",
		args = "",
		num_args = 0
	})

	register_command("quit", {
		description = "Exits the application",
		args = "",
		num_args = 0
	})

	register_command("clear", {
		description = "clear the terminal",
		args = "",
		num_args = 0
	})
	
	# Register built-in cvars
	register_cvar("client_max_fps", {
		description = "The maximal framerate at which the application can run",
		type = "int",
		default_value = 61,
		min_value = 10,
		max_value = 1000
	})

func _input(event):
	if (Engine.is_editor_hint()):
		return;
		
	if event.is_action_pressed("console_toggle"):
		set_console_open(!is_console_open());

	if event.is_action_pressed("console_up"):
		if (cmd_history_up > 0 and cmd_history_up <= cmd_history.size()):
			cmd_history_up-=1
			get_node("Panel/Container/LineEdit").set_text(cmd_history[cmd_history_up])

	if event.is_action_pressed("console_down"):
		if (cmd_history_up > -1 and cmd_history_up + 1 < cmd_history.size()):
			cmd_history_up +=1
			get_node("Panel/Container/LineEdit").set_text(cmd_history[cmd_history_up])

	if get_node("Panel/Container/LineEdit").get_text() != "" and get_node("Panel/Container/LineEdit").has_focus() and Input.is_key_pressed(KEY_TAB):
		complete()
		
	return;

func complete():
	var text = get_node("Panel/Container/LineEdit").get_text()
	var matches = 0
	# If there are no matches found yet, try to complete for a command or cvar
	if matches == 0:
		for command in commands:
			if command.begins_with(text):
				describe_command(command)
				get_node("Panel/Container/LineEdit").set_text(command + " ")
				get_node("Panel/Container/LineEdit").set_cursor_position(100)
				matches += 1
		for cvar in cvars:
			if cvar.begins_with(text):
				describe_cvar(cvar)
				get_node("Panel/Container/LineEdit").set_text(cvar + " ")
				get_node("Panel/Container/LineEdit").set_cursor_position(100)
				matches += 1

# This function is called from scripts/console_commands.gd to avoid the
# "Cannot access self without instance." error
func quit():
	get_tree().quit()

# This function is called from scripts/console_commands.gd because get_node()
# can't be used there
func set_label_text(value):
	get_node("/root/Test/ExampleLabel").set_text(value)

func set_console_open(open):
	# Close the console
	if open == false:
		get_node("Panel").set_visible(false);
		
	# Open the console
	elif open == true:
		get_node("Panel").set_visible(true);
		get_node("Panel/Container/LineEdit").grab_focus();
		get_node("Panel/Container/LineEdit").call_deferred("clear");
		
	emit_signal("set_console_open", open);

# use this to determine if the console is open and has focus
func is_console_open():
	return get_node("Panel").is_visible()

# Called when the user presses Enter in the console
func _on_LineEdit_text_entered(text):
	# used to manage cmd history
	if cmd_history.size() > 0:
		if (text != cmd_history[cmd_history_count - 1]):
			cmd_history.append(text)
			cmd_history_count+=1
	else:
		cmd_history.append(text)
		cmd_history_count+=1
	cmd_history_up = cmd_history_count
	var text_splitted = text.split(" ", true)
	# Don't do anything if the LineEdit contains only spaces
	if not text.empty() and text_splitted[0]:
		handle_command(text)
	else:
		# Clear the LineEdit but do nothing
		get_node("Panel/Container/LineEdit").clear()

# deregister commands that are belonged to a "owner" object
func deregister_commands(owner):
	var removeList = []
	for c in commands:
		var cArgs = commands[c]
		if (cArgs.has("owner") && cArgs["owner"] == owner):
			removeList.append(c)
				
	for c in removeList:
		commands.erase(c)

# Registers a new command
func register_command(name, args):
	commands[name] = args

# Registers a new cvar (control variable)
func register_cvar(name, args):
	cvars[name] = args
	cvars[name].value = cvars[name].default_value

func append_bbcode(bbcode):
	if (bbcode == null || bbcode.length() <= 0 || !console_text):
		return
		
	var lines = bbcode.split("\n")
	var linesIdx = 0
	
	if (!last_line_complete):
		line_history[line_history.size() - 1] += lines[0].length() # dont need to account for \n here as its done below
		linesIdx += 1
	
	for l in range(linesIdx, lines.size()):
		var line = lines[l]
		if (line.length()):
			line_history.append(line.length() + 1)	# + 1 to account for \n	
		
	last_line_complete = bbcode.ends_with("\n")
	
	# chop off old lines
	var text = console_text.get_bbcode() + bbcode
	var charsToChop = 0
	while (line_history.size() > MAX_LINES):
		charsToChop += line_history.pop_front()
	
	if (charsToChop > 0):
		text = text.substr(charsToChop, text.length())
	
	console_text.set_bbcode(text)

func get_history_str():
	var strOut = ""
	var count = 0
	for i in cmd_history:
		strOut += "[color=#ffff66]" + str(count) + ".[/color] " + i + "\n"
		count+=1
	
	return strOut

func clear():
	console_text.set_bbcode("")
	line_history = []
	last_line_complete = true

# Describes a command, user by the "cmdlist" command and when the user enters a command name without any arguments (if it requires at least 1 argument)
func describe_command(cmd):
	var command = commands[cmd]
	var description = command.description
	var args = command.args
	var num_args = command.num_args
	if num_args >= 1:
		append_bbcode("[color=#ffff66]" + cmd + ":[/color] " + description + " [color=#88ffff](usage: " + cmd + " " + args + ")[/color]\n")
	else:
		append_bbcode("[color=#ffff66]" + cmd + ":[/color] " + description + " [color=#88ffff](usage: " + cmd + ")[/color]\n")

# Describes a cvar, used by the "cvarlist" command and when the user enters a cvar name without any arguments
func describe_cvar(cvar):
	var cvariable = cvars[cvar]
	var description = cvariable.description
	var type = cvariable.type
	var default_value = cvariable.default_value
	var value = cvariable.value
	if type == "str":
		append_bbcode("[color=#88ff88]" + str(cvar) + ":[/color] [color=#9999ff]\"" + str(value) + "\"[/color]  " + str(description) + " [color=#ff88ff](default: \"" + str(default_value) + "\")[/color]\n")
	else:
		var min_value = cvariable.min_value
		var max_value = cvariable.max_value
		append_bbcode("[color=#88ff88]" + str(cvar) + ":[/color] [color=#9999ff]" + str(value) + "[/color]  " + str(description) + " [color=#ff88ff](" + str(min_value) + ".." + str(max_value) + ", default: " + str(default_value) + ")[/color]\n")

func handle_command(text):
	# special case to execute gdscript using /c
	if (text.begins_with("/c")):
		var source = text.substr(2, text.length()); 
		evaluate(source);
		return;
	
	# split incase multiple commands are supplied
	var commands = text.split(";", false);
	for cmd in commands:
		_handle_command(cmd.strip_edges());
	
func _handle_command(text):
	# The current console text, splitted by spaces (for arguments)
	var cmd = text.split(" ", true)
	# Check if the first word is a valid command
	if commands.has(cmd[0]):
		var command = commands[cmd[0]]
		print("> " + text)
		append_bbcode("[b]> " + text + "[/b]\n")
		# If no argument is supplied, then show command description and usage, but only if command has at least 1 argument required
		if cmd.size() == 1 and not command.num_args == 0:
			describe_command(cmd[0])
		else:
			# Run the command! If there are no arguments, don't pass any to the other script.
			if command.num_args == 0:
				if (command.has("method")):
					command.method.call_func()
				else:
					call(cmd[0].replace(".",""))
			else:
				if (command.has("method")):
					command.method.call_func(text)
				else:
					call(cmd[0].replace(".",""), text)
	# Check if the first word is a valid cvar
	elif cvars.has(cmd[0]):
		var cvar = cvars[cmd[0]]
		print("> " + text)
		append_bbcode("[b]> " + text + "[/b]\n")
		# If no argument is supplied, then show cvar description and usage
		if cmd.size() == 1:
			describe_cvar(cmd[0])
		else:
			# Let the cvar change values!
			if cvar.type == "str":
				for word in range(1, cmd.size()):
					if word == 1:
						cvar.value = str(cmd[word])
					else:
						cvar.value += str(" " + cmd[word])
			elif cvar.type == "int":
				cvar.value = int(cmd[1])
			elif cvar.type == "float":
				cvar.value = float(cmd[1])

			# Call setter code
			call(cmd[0], cvar.value)
	else:
		# Treat unknown commands as unknown
		append_bbcode("[b]> " + text + "[/b]\n")
		append_bbcode("[i][color=#ff8888]Unknown command or cvar: " + cmd[0] + "[/color][/i]\n")
	get_node("Panel/Container/LineEdit").clear()
	
	
# https://godotengine.org/qa/339/does-gdscript-have-method-to-execute-string-code-exec-python
func evaluate(text):
	var script = GDScript.new();
	script.set_source_code("func eval():\n\t" + text);
	script.reload();
	
	var obj = Reference.new();
	obj.set_script(script);
	
	return obj.eval();
	

# Prints a string in console
static func echo(text):
	# Erase "echo" from the output
	text.erase(0, 5)
	Console.append_bbcode(text + "\n")

# Lists all available commands
static func cmdlist():
	var commands = Console.commands
	for command in commands:
		Console.describe_command(command)

static func history():
	Console.append_bbcode(Console.get_history_str())

# Lists all available cvars
static func cvarlist():
	var cvars = Console.cvars
	for cvar in cvars:
		Console.describe_cvar(cvar)

# Prints some help
static func help():
	var help_text = """Type [color=#ffff66]cmdlist[/color] to get a list of commands.
Type [color=#ffff66]quit[/color] to exit the application."""
	Console.append_bbcode(help_text + "\n")
	
# The maximal framerate at which the application can run
static func client_max_fps(value):
	OS.set_target_fps(int(value))




func _on_LineEdit_text_changed( text ):
	pass # replace with function body
