extends Spatial

export(float) var damage = 10;

export(float) var attack_recover_time = 0; # how long before we can attack again
export(float) var attack_time = 0; # how long the cast/swing takes

export(float) var range_min = 0;
export(float) var range_max = 0;

export(float) var weight = 0;

func _ready():
	pass # Replace with function body.
	
