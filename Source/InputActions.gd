#
# Copyright (C) 2017 bitshift
# http://bit-shift.io 
# 
# View LICENSE at:
# https://github.com/bit-shift-io/trains-and-things/blob/master/LICENSE.md
#

#
# Using was_pressed and was_released in the game loop is not recommended
# especially for mouse related events
# a mouse wheel up might trigger multiple times in a single frame hence
# using was_pressed and was_released returns incorrect results
#
extends Node

enum State {
	STATE_UP,
	STATE_DOWN
};

# A note on godot _input and _unhandled_input
# first godot calls _input to see if input is handled by a specific class
# if nothing consumes the event, it gets passed to _unhandled_input
# when a text field has focus for example, the event is still passed to _input
# to allow intercepting it before the text field, so most often
# if you dont want to steal focus, use _unhandled_input by supplying HM_UNHANDLED
#
# It seems mouse events ignore this mechanic and require HM_HANDLED
enum HandledMode {
	HM_HANDLED,
	HM_UNHANDLED,
	HM_ALL
};


class Action:
	signal is_pressed();
	signal is_released();
	
	var name;
	var state = State.STATE_UP;
	var prev_state = State.STATE_UP;
	var handled_mode = HandledMode.HM_ALL;
	
	func _init():
		return;
		
	# return true if there is a state change
	func set_state(p_state):
		#Log.debug("Action: " + name + " set_state: " + str(p_state));
		if (p_state == state):
			return false;
			
		prev_state = p_state;
		state = p_state;
		return true;
		
	func input(event):
		if (handled_mode == HandledMode.HM_ALL || handled_mode == HandledMode.HM_HANDLED):
			_process_input(event);
		return;
		
	func _process_input(event):
		if (event.is_action_pressed(name)):
			if (set_state(State.STATE_DOWN)):
				emit_signal("is_pressed");
			
		if (event.is_action_released(name)):
			if (set_state(State.STATE_UP)):
				emit_signal("is_released");
			
		return;
		
	func unhandled_input(event):
		if (handled_mode == HandledMode.HM_ALL || handled_mode == HandledMode.HM_UNHANDLED):
			_process_input(event);
		return;
		
	func process(delta):
		prev_state = state;
		return;
		
	func is_down():
		return (state == State.STATE_DOWN);
		
	func is_up():
		return (state == State.STATE_UP);
		
	func was_pressed():
		return state == State.STATE_DOWN && prev_state == State.STATE_UP;
		
	func was_released():
		return state == State.STATE_UP && prev_state == State.STATE_DOWN;
		
		
# end class Action
	
var actions = {};
var enabled = true;

func _ready():
	set_process(true);
	set_process_input(true);
	set_process_unhandled_input(true);
	return;
	
func _process(delta):
	for action_name in actions:
		actions[action_name].process(delta);
		
	return;
	
func _unhandled_input(event):
	if (!enabled):
		return;
		
	for action_name in actions:
		if (event.is_action(action_name)):
			actions[action_name].unhandled_input(event);
			
	return;
	
func _input(event):
	if (!enabled):
		return;
		
	for action_name in actions:
		if (event.is_action(action_name)):
			actions[action_name].input(event);
			
	return;
	
func add_action(action_name, handled_mode = HandledMode.HM_ALL):
	var action = Action.new();
	action.name = action_name;
	action.handled_mode = handled_mode;
	actions[action_name] = action;
	return action;
	
func add_actions(p_actions, handled_mode = HandledMode.HM_ALL):
	var results = [];
	for action_name in p_actions:
		results.push_back(add_action(action_name, handled_mode));
		
	return results;
		
func is_down(p_action_name):
	var action = actions[p_action_name];
	return action.is_down();
	
func was_pressed(p_action_name):
	var action = actions[p_action_name];
	return action.was_pressed();
	
func was_released(p_action_name):
	var action = actions[p_action_name];
	return action.was_released();
	
func is_enabled():
	return enabled;
	
func set_enabled(enable):
	enabled = enable;
