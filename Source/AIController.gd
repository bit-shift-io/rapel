extends Spatial

const Actor = preload("Actor.gd");

export(int) var target_radius = 2;
export(bool) var enabled = true;

onready var actor = $"Actor";
onready var patrol_path = $"Path";

var target;
onready var target_radius_squared = target_radius * target_radius;

enum State {
	OnPatrolRoute,
	ApproachingTarget
}

var state;

func _ready():
	if (!enabled):
		set_process(false);
		actor.set_enabled(false);
		
	add_to_group("controllers");
	
	actor.show_path(false); # only for players
	
	# deparent the path from this actor and reparent to scene root so the follow 2D node can be used
	#if (patrol_path):
		#patrol_path.get_parent().remove_child(patrol_path);
		#get_tree().get_root().call_deferred("add_child", patrol_path);
		#patrol_path.update();
		
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
	#var dist_sqrd = actor.get_global_position().distance_squared_to(desired_target.get_global_position());
	#if (dist_sqrd > target_radius_squared):
#		return_to_patrol_route();
#		return;
	
	set_target(desired_target);

	var actor_pos = actor.get_global_position();
	var move_to_pos = target.get_global_position()
	var path_points = NavigationMgr.get_simple_path(actor_pos, move_to_pos);

	# if no path (path finding is broken atm) just go in a straight line for now
	#if (len(path_points) <= 0):
	#	path_points = [actor_pos, move_to_pos];

	#print(path_points)
	actor.set_path_points(path_points, false);
	state = State.ApproachingTarget;
	return;
	
func path_length_squared(path_points, max_dist_squared):
	var dist_squared = 0;
	for i in range(1, len(path_points)):
		var vec = (path_points[i] - path_points[i-1]);
		dist_squared += vec.length_squared();
		if (dist_squared > max_dist_squared):
			return max_dist_squared;
		
	return dist_squared;
		
	
func update_target():
	# is it better to sort enemies by distance incase we a targetting multiple? just make target a targeting list
	var dist_squared = target_radius_squared;
	var desired_target = null;
	for a in get_tree().get_nodes_in_group("actors"):
		if is_enemy(a):
			var dist_to_a_squared = actor.get_global_position().distance_squared_to(a.get_global_position());
			if (dist_to_a_squared < dist_squared):
				
				# okay so we are in range, do we have line of sight? - for now we just check the navigation distance
				var path_points = NavigationMgr.get_simple_path(actor.get_global_position(), a.get_global_position());
				var real_dist_to_a = path_length_squared(path_points, target_radius_squared);
				if (real_dist_to_a > dist_squared):
					continue;
				
				dist_squared = dist_to_a_squared;
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