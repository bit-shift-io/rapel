import bpy

y_scale = 0.3

try:
    for fcu in bpy.context.object.animation_data.action.fcurves:
        if fcu.data_path.endswith("location"):
            print("location for: " + fcu.data_path)
            for kp in fcu.keyframe_points:
                kp.co.y *= y_scale
                kp.handle_left.y *= y_scale
                kp.handle_right.y *= y_scale
except TypeError:
    pass
