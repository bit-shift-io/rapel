extends Node

enum Phase {
	Execute,
	Plan
}

var phase = Phase.Execute;

signal change_phase(p_phase);

func _ready():
	pass # Replace with function body.
	
func set_phase(p_phase):
	phase = p_phase;
	emit_signal("change_phase", phase);
	
func get_phase():
	return phase;
