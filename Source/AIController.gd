extends Node2D

const Actor = preload("Actor.gd");

export(int) var target_radius = 100;

onready var actor = $"Actor";
onready var patrol_path = $"Path2D";

var target;
onready var target_radius_squared = target_radius * target_radius;

enum State {
	OnPatrolRoute,
	ApproachingTarget
}

var state;

func _ready():
	add_to_group("controllers");
	
	actor.show_path(false); # only for players
	
	# deparent the path from this actor and reparent to scene root so the follow 2D node can be used
	if (patrol_path):
		#patrol_path.get_parent().remove_child(patrol_path);
		#get_tree().get_root().call_deferred("add_child", patrol_path);
		patrol_path.update();
		
	actor.connect("command_list_completed", self, "_command_list_completed");
	actor.connect("state_changed", self, "_actor_state_changed");
	
func _actor_state_changed(p_state):
	if (p_state == Actor.State.Dying):
		set_target(null);
	return;
	
func return_to_patrol_route():
	if (state == State.OnPatrolRoute):
		return;
		
	# set patrol route
	actor.set_path_points(patrol_path.get_curve().get_baked_points(), true);
	state = State.OnPatrolRoute;
	
func _process(delta):
	if (GameState.get_phase() == GameState.Phase.Plan):
		return;
		
	if (!actor.is_alive()):
		return;
		
	# pick closest target
	var desired_target = update_target();
	if (!desired_target):
		return_to_patrol_route();
		return;
		
	# move towards?
	var dist_sqrd = actor.get_global_position().distance_squared_to(desired_target.get_global_position());
	if (dist_sqrd > target_radius_squared):
		return_to_patrol_route();
		return;
	
	set_target(desired_target);

	var actor_pos = actor.get_global_position();
	var move_to_pos = target.get_global_position()
	var path_points = actor.navigation.get_simple_path(actor_pos, move_to_pos);
	
	# if no path (path finding is broken atm) just go in a straight line for now
	if (len(path_points) <= 0):
		path_points = [actor_pos, move_to_pos];
		
	#print(path_points)
	actor.set_path_points(path_points, false);
	state = State.ApproachingTarget;
	
func update_target():
	# is it better to sort enemies by distance incase we a targetting multiple? just make target a targeting list
	var dist = -1;
	var desired_target = null;
	for a in get_tree().get_nodes_in_group("actors"):
		if is_enemy(a):
			var dist_to_a = actor.get_global_position().distance_squared_to(a.get_global_position());
			if (dist == -1 || dist_to_a < dist):
				dist = dist_to_a;
				desired_target = a;
				
	return desired_target;
				
func set_target(p_new_target):
	# no change in target
	if (p_new_target == target):
		return;
		
	var old_target = target;
	target = p_new_target;
	
	if (old_target):
		old_target.set_targetted(actor, false);
		
	if (target):
		target.set_targetted(actor, true);
	
	
func is_enemy(p_other_actor):
	if (p_other_actor != actor):
		return true;
	
	return false;