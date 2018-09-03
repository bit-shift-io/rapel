# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####


bl_info = {
    'name': "bitshift tools",
    'author': "Bronson & Fabian Mathews",
    'version': (1, 0, 0),
    'blender': (2, 8, 0),
    'location': "Toolshelf",
    'description': "bitshift tools",
    "warning": "", # used for warning icon and text in addons panel
    "wiki_url": "http://bitshift.io",
    'category': 'Tools'}


import bpy, bmesh, mathutils, logging, time, re
from bpy.props import BoolProperty, IntProperty, FloatProperty, EnumProperty
from collections import defaultdict
from math import radians, hypot
from mathutils import Vector, Color
import os
import sys


class bitshift_tools(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'TOOLS'
    bl_category = 'Tools'
    bl_label = "bitshift Tools"
    
  
    def draw(self, context):
        layout = self.layout
        col = layout.column(align=True)
        
        col.operator("bitshift.activate_hotkeys", text="Activate Hotkeys")
        col.operator("bitshift.purge_orphans", text="Purge Orphans")
        
        col.label("Export")
        col.operator("bitshift.export_scene", text="Export Scene")
        col.operator("bitshift.export_selected", text="Export Selected")
        col.operator("export_scene.glb", text="Export")
        
        col.label("Terrain")
        col.operator("bitshift.render_map", text="Render Map")
        col.operator("bitshift.import_map", text="Import Map")
           
    
#------------------- export ------------------------------   
def get_path():
    # get scene render path, if user has changed from /tmp/
    directory = bpy.path.abspath(bpy.context.scene.render.filepath)
    if directory == "/tmp/":
        directory = bpy.path.abspath(bpy.data.filepath)
    
    # find the root directory
    while True:
        gitdir = os.path.join(directory, '.git')
        if os.path.exists(gitdir):
            directory = os.path.join(directory, 'Source/Mesh')
            break
        elif directory == "/":
            directory = bpy.path.abspath(bpy.context.scene.render.filepath)
            break
        directory = os.path.dirname(os.path.dirname(directory))

    # get filename
    name = bpy.path.display_name_from_filepath(bpy.data.filepath)

    linux_filepath = str(directory + "/" + name + '.glb')
    windows_filepath = str(directory + "\\" + name + '.glb')

    if sys.platform.startswith('win'):
        path = windows_filepath
    
    elif sys.platform.startswith('linux'):
        path = linux_filepath 
    
    print(path)
    return path

	
class exportSelected(bpy.types.Operator):
    """Export Selected"""
    bl_label = "Export Selected"
    bl_idname = "bitshift.export_selected"
    
    def execute(self, context):
        print("Export Selected")
        path = get_path()
        bpy.ops.export_scene.glb(export_apply=True, filepath=path, filter_glob="*.glb", export_selected=True)
        return {"FINISHED"}
		
		
class exportScene(bpy.types.Operator):
    """Export All"""
    bl_label = "Export All"
    bl_idname = "bitshift.export_scene"
    
    def execute(self, context):
        print("Export All")
        path = get_path()
        bpy.ops.export_scene.glb(export_apply=True, filepath=path, filter_glob="*.glb")
        return {"FINISHED"}

#------------------- misc ------------------------------  
class remove_unused_data(bpy.types.Operator):
    bl_label = "Purge orphans"
    bl_idname = "bitshift.purge_orphans"
    # this is also hidden in the outliner > under orphan data
    
    def execute(self, context):
        #bpy.ops.outliner.orphans_purge()
                
        for block in bpy.data.meshes:
            if block.users == 0:
                bpy.data.meshes.remove(block)

        for block in bpy.data.materials:
            if block.users == 0:
                bpy.data.materials.remove(block)

        for block in bpy.data.textures:
            if block.users == 0:
                bpy.data.textures.remove(block)

        for block in bpy.data.images:
            if block.users == 0:
                bpy.data.images.remove(block)        
                
        for block in bpy.data.curves:
            if block.users == 0:
                bpy.data.images.remove(block)  
                
        for block in bpy.data.lamps:
            if block.users == 0:
                bpy.data.images.remove(block)  
                
        for block in bpy.data.cameras:
            if block.users == 0:
                bpy.data.images.remove(block)    
        
        return {"FINISHED"}
        
#------------------- terrain ------------------------------   

class terrain_render_map(bpy.types.Operator):
    bl_label = "Render Map"
    bl_idname = "bitshift.render_map"
    
    node_types = {
        'RL' : 'CompositorNodeRLayers',
        'OF' : 'CompositorNodeOutputFile',
        'OC' : 'CompositorNodeComposite'
    }
    
    def execute(self, context):
        print("Rendering map")
        self.render_image(context)
        return {"FINISHED"}
    

    def get_output( self, passout ):
        """ Find the renderlayer node's output that matches the current render
            pass """

        # Renderlayer pass names and renderlayer node output names do not match
        # which is why we're using this dictionary (and regular expressions)
        # to match the two
        # file name extension : blender extension
        output_dict = {
            'ambient_occlusion' : 'AO',
            'material_index'    : 'IndexMA',
            'object_index'      : 'IndexOB',
            'reflection'        : 'Reflect',
            'refraction'        : 'Refract',
            'combined'          : 'Image',
            'uv'                : 'UV',
            'z'                 : 'Depth'
        }

        output = ''
        if passout in output_dict.keys():
            output = output_dict[ passout ]
        elif "_" in passout:
            wl = passout.split("_") # Split to list of words
            # Capitalize first char in each word and rejoin with spaces
            output = " ".join([ s[0].capitalize() + s[1:] for s in wl ])
        else: # If one word, just capitlaize first letter
            output = passout[0].capitalize() + passout[1:]

        return output


    def save_output_passes(self,context):
        # compositing
        bpy.context.scene.use_nodes = True
        tree = bpy.context.scene.node_tree
        links = tree.links
        
        for node in tree.nodes:
            tree.nodes.remove(node)
            
        # get name of current file
        filename = bpy.path.basename(bpy.context.blend_data.filepath) 
        basename = os.path.splitext(filename)[0]
        pass_attr_str = 'use_pass_'

        rl_nodes_y   = 0
        node = ''  # Initialize node so that it would exist outside the loop

        # for each render layer create an entry in the output node
        for rl in context.scene.render.layers:
            # Create a new render layer node
            node = ''
            node = tree.nodes.new( type = self.node_types['RL'] )

            # Set node location, label and name
            node.location = 0, rl_nodes_y
            node.label    = rl.name
            node.name     = rl.name

            # Select the relevant render layer
            node.layer = rl.name

            # create nodes
            output_node = tree.nodes.new( type = self.node_types['OF'] )
            output_node.file_slots.remove(output_node.inputs[0]) # remove first entry

            # Set base path, location, label and name
            #filepath = bpy.data.filepath
            #directory = os.path.dirname(filepath)
            #output_node.base_path = directory # should be default render path!
            output_node.location  = 500, 0
            output_node.label     = 'file output'
            output_node.name      = 'file output'
            output_node.format.file_format = 'PNG'
            output_node.format.color_mode = 'RGB'
            output_node.format.compression = 100

            # for each pass in layer
            passes = [ p for p in dir(rl) if pass_attr_str in p ]
            for p in passes:
                # If render pass is active (True) - create output
                if getattr( rl, p ):
                    pass_name = p[len(pass_attr_str):]
                    output = self.get_output( pass_name )
                    file_path = basename + "-" + output
        
                    #if output == 'Image' and not output_node.inputs[ output ].links:
                    #    links.new( node.outputs[ output ], output_node.inputs[ output ])   
                        
                    if output:
                        # Add file output socket
                        output_node.file_slots.new( name = output )
                        socket = output_node.file_slots[-1]
                        socket.path = file_path
                        socket.use_node_format = False
                        
                        if output == 'Depth':
                            file_path = basename + "-Height"
                            socket.path = file_path
                            normalize_node = tree.nodes.new(type="CompositorNodeNormalize")
                            links.new(node.outputs[ output ],normalize_node.inputs[0])
                            
                            invert_node = tree.nodes.new(type="CompositorNodeInvert")
                            links.new(normalize_node.outputs[0],invert_node.inputs[1])
                            links.new(invert_node.outputs[0],output_node.inputs[-1])  
                            
                            # configure texture
                            socket.format.color_mode = 'BW'
                            socket.format.color_depth = '16'
                            socket.format.compression = 100
                            
                            # exr for testing
                            #socket.format.file_format = 'OPEN_EXR'
                            #socket.format.exr_codec = 'NONE'
                            
                        elif output == 'Normal':
                            seperate_node = tree.nodes.new(type="CompositorNodeSepRGBA")
                            links.new(node.outputs[ output ], seperate_node.inputs[0])
                            
                            r_normalize_node = tree.nodes.new(type="CompositorNodeNormalize")
                            links.new(seperate_node.outputs[0], r_normalize_node.inputs[0])
                            
                            g_normalize_node = tree.nodes.new(type="CompositorNodeNormalize")
                            links.new(seperate_node.outputs[1], g_normalize_node.inputs[0])
                            
                            combine_node = tree.nodes.new(type="CompositorNodeCombRGBA")
                            links.new(r_normalize_node.outputs[0], combine_node.inputs[0])  
                            links.new(g_normalize_node.outputs[0], combine_node.inputs[1]) 
                            links.new(seperate_node.outputs[2], combine_node.inputs[2]) 
                            links.new(combine_node.outputs[0], output_node.inputs[-1])    
                            
                            # configure texture
                            socket.format.color_mode = 'RGB'
                            socket.format.color_depth = '16'
                            socket.format.compression = 100                            
                            
                        else:
                            # Set up links
                            links.new( node.outputs[ output ], output_node.inputs[-1] )

            rl_nodes_y -= 300
        '''
        # Create composite node, just to enable rendering
        cnode = ''
        cnode = tree.nodes.new( type = self.node_types['OC'] )

        # Link composite node with the last render layer created
        links.new( node.outputs[ 'Image' ], cnode.inputs[0] )
        '''
        
        
    def fix_file_names(self,context):
        directory = bpy.path.abspath(bpy.context.scene.render.filepath)
        for f in os.listdir(directory):
            if f.endswith("0001.png"):
                fname = os.path.join(directory, f.replace("0001",""))
                if os.path.isfile(fname):
                    os.remove(fname)
                os.rename(os.path.join(directory, f), fname)



    def render_image(self,context):
        selected = bpy.context.scene.objects.active
        if selected == "":
            self.report({'ERROR'},"please select a mesh")
            return {"FINISHED"}
        

        # copy mesh
        bpy.ops.object.duplicate()
        copy = bpy.context.scene.objects.active
        bpy.ops.object.editmode_toggle()
        bpy.context.tool_settings.mesh_select_mode = (False, True, True) # edge mode
        bpy.ops.mesh.select_all(action='DESELECT')
        bpy.ops.mesh.select_non_manifold()
        bpy.ops.mesh.extrude_region()
        bpy.ops.mesh.select_non_manifold()
        bpy.ops.transform.resize(value=(2, 2, 2), constraint_axis=(True, True, False), constraint_orientation='GLOBAL')
        bpy.ops.object.editmode_toggle()       


        # setup image
        image = bpy.data.images.get("bake")
        if image != None:
            bpy.data.images.remove(image)
        image = bpy.data.images.new("bake", width=bpy.context.scene.render.resolution_x, height=bpy.context.scene.render.resolution_y)
        
        
        # setup cameras
        bbox_corners = [selected.matrix_world * Vector(corner) for corner in selected.bound_box]
        pos = max(z for (x,y,z) in bbox_corners) + 1  # python wizardry?
        bpy.ops.object.camera_add(view_align=False, enter_editmode=False, location=(0.0, 0.0, pos), rotation=(0.0, 0.0, 0.0),)
        camera = bpy.context.scene.objects.active
        camera.name = "bake"
        camera.data.type = 'ORTHO'
        camera.data.clip_start = 0.5 # needs to be less than 1 we add above
        camera.data.clip_end = 1 + selected.dimensions.z
        camera.data.ortho_scale = max(selected.dimensions)
        bpy.context.scene.camera = camera

        # setup render
        bpy.context.scene.render.engine = 'CYCLES'
        bpy.context.scene.sequencer_colorspace_settings.name = 'Raw'    
        bpy.context.scene.render.layers["RenderLayer"].use_pass_ambient_occlusion = True
        bpy.context.scene.render.layers["RenderLayer"].use_pass_normal = True
        bpy.context.scene.render.layers["RenderLayer"].use_pass_z = True
        bpy.context.scene.view_settings.view_transform = 'Raw'
    
        # render
        self.save_output_passes(context)
        bpy.ops.render.render()
        self.fix_file_names(context)

        # display 
        for area in bpy.context.screen.areas :
            if area.type == 'IMAGE_EDITOR' :
                area.spaces.active.image = bpy.data.images['Render Result']
                for region in area.regions:
                    if region.type == 'WINDOW':
                        ctx = bpy.context.copy()
                        ctx[ 'area'] = area
                        ctx['region'] = region
                        bpy.ops.image.view_all(ctx,fit_view=True)    
            
        # cleanup
        selected.hide_render = False
        bpy.ops.object.select_all(action='DESELECT')
        selected.select = True
        bpy.data.cameras.remove(camera.data)
        bpy.data.meshes.remove(copy.data)
        selected.select = True
        bpy.context.scene.objects.active = selected
    
 
    
class terrain_import_map(bpy.types.Operator):
    bl_label = "Import Map"
    bl_idname = "bitshift.import_map"
    
    filepath = bpy.props.StringProperty(subtype="FILE_PATH")
    
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}
    
    def execute(self, context):
        print("Import map")
        
        # setup render
        bpy.context.scene.render.engine = 'CYCLES'
        bpy.context.scene.sequencer_colorspace_settings.name = 'Raw'         
        
        # open image
        image = bpy.data.images.load(filepath=self.filepath, check_existing=False)
        image.colorspace_settings.name = 'Raw'
        texture = bpy.data.textures.new('Terrain', type = 'IMAGE')
        texture.image = image
        texture.extension = 'EXTEND'

        # create mesh
        divisions = image.size[0]/8 # divide into 4 levels
        size = image.size[0]/2
        bpy.ops.mesh.primitive_plane_add(radius=size, calc_uvs=True, view_align=False, enter_editmode=True, location=(0, 0, 0))
        plane = bpy.context.scene.objects.active
        bpy.ops.mesh.subdivide(number_cuts=divisions -2) # account for existing edges
        bpy.ops.object.editmode_toggle()  
        mesh = bpy.context.scene.objects.active
        mesh.name = "terrain"
        
        # subdiv 
        # apply 4 levels
        multires = mesh.modifiers.new("Multires", type='MULTIRES')
        multires.subdivision_type = 'SIMPLE'

        bpy.ops.object.multires_subdivide(modifier="Multires")
        bpy.ops.object.multires_subdivide(modifier="Multires")
        bpy.ops.object.multires_subdivide(modifier="Multires")
        bpy.ops.object.multires_subdivide(modifier="Multires")


        # displace
        displace = mesh.modifiers.new("Displace", type='DISPLACE')
        displace.strength = 100
        displace.mid_level = 0
        displace.texture_coords = 'UV'
        displace.uv_layer = "UVMap"
        displace.texture = texture

        bpy.ops.object.modifier_apply(apply_as='DATA', modifier="Displace")
        
        # add palette for painting terrain
        pal_array = [
            (1, 0, 0),
            (0, 1, 0),
            (0, 0, 1),
            (0, 0, 0)
        ]

        bpy.ops.palette.new()
        pal = bpy.data.palettes[-1]
        pal.name='terrain'
        for col in pal_array:
            bpy.ops.palette.color_add()
            pal.colors[-1].color = Color(col)
            
        # apply material
        material = self.create_material(context, mesh)
        
        # try and load terrain map, and boundary map if they exist
        #directory = os.path.dirname(image.filepath)
        terrain_path = image.filepath.replace("Height","Terrain")
        boundary_path = image.filepath.replace("Height","Boundary")

        if os.path.isfile(terrain_path):
            terrain_img = bpy.data.images.load(filepath=terrain_path, check_existing=True)
            nodes = material.node_tree.nodes
            links = material.node_tree.links
            terrain_node = nodes["terrain"]
            terrain_node.image = terrain_img
            terrain_node.color_space = 'NONE'
            terrain_node.extension = 'EXTEND'    
            # TODO: apply this to the terrain in the same way the shader works
            
        if os.path.isfile(boundary_path):
            boundary_img = bpy.data.images.load(filepath=boundary_path, check_existing=True)
            nodes = material.node_tree.nodes
            links = material.node_tree.links
            boundary_node = nodes.new('ShaderNodeTexImage')
            boundary_node.name = "boundary"
            boundary_node.image = boundary_img
            boundary_node.location = (-400,-900)         
            boundary_node.color_space = 'NONE'
            boundary_node.extension = 'EXTEND'
            # TODO: put some kind of value slider to view how much this affects
            
            

        return {"FINISHED"}    
    
    
    def create_material(self, context, mesh):
        material = bpy.data.materials.get("terrain")
        if material != None:
            bpy.data.materials.remove(material)
        material = bpy.data.materials.new("terrain")
        material.use_nodes = True
        mesh.data.materials.append(material)
        
        nodes = material.node_tree.nodes
        links = material.node_tree.links
        
        # delete old nodes
        #for n in nodes:
        #    n.delete()
        
        # create the mix/blender/layer group node
        layer_group = self.create_layer_group()
        group = nodes.new("ShaderNodeGroup")
        group.node_tree = layer_group
        #bpy.data.node_groups['layer_group'] # or call by name
        group.location = (0,0)
        
        # input nodes
        alpha = nodes.new('ShaderNodeTexImage')
        alpha.name = "alpha"
        alpha.location = (-400,900)
        
        red = nodes.new('ShaderNodeTexImage')
        red.name = "red"
        red.location = (-400,600)     
        
        green = nodes.new('ShaderNodeTexImage')
        green.name = "green"
        green.location = (-400,300)   
        
        blue = nodes.new('ShaderNodeTexImage')
        blue.name = "blue"
        blue.location = (-400,000)     
        
        base = nodes.new('ShaderNodeTexImage')
        base.name = "base"
        base.location = (-400,-300)    
        
        terrain = nodes.new('ShaderNodeTexImage')
        terrain.name = "terrain"
        terrain.location = (-400,-600)         
        terrain.color_space = 'NONE'
        terrain.extension = 'EXTEND'
        
        # output nodes
        dif = nodes['Diffuse BSDF'] # get existing nodes
        dif.location = (300,0) 
        
        mat = nodes['Material Output']
        mat.location = (500,0) 
        
        # link image nodes
        links.new(alpha.outputs['Color'], group.inputs['alpha layer'])
        links.new(blue.outputs['Color'], group.inputs['blue layer'])
        links.new(green.outputs['Color'], group.inputs['green layer'])
        links.new(red.outputs['Color'], group.inputs['red layer'])
        links.new(base.outputs['Color'], group.inputs['base layer'])
        links.new(terrain.outputs['Color'], group.inputs['terrain_rgb'])
        links.new(terrain.outputs['Alpha'], group.inputs['terrain_a'])
        
        # output nodes
        links.new(group.outputs['color'], dif.inputs['Color']) 
        links.new(dif.outputs['BSDF'], mat.inputs['Surface']) 
        
        return material
        
    def create_layer_group(self):
        # function adds a group to the group menu, the user still has to place it afterwards
        
        # check for old, and remove it
        if "layer_group" in bpy.data.node_groups:
            group = bpy.data.node_groups["layer_group"]
            bpy.data.node_groups.remove(group)
            
        group = bpy.data.node_groups.new("layer_group", 'ShaderNodeTree')
        
        # create group inputs
        group_inputs = group.nodes.new('NodeGroupInput')
        group_inputs.location = (-500,0)
        group.inputs.new('NodeSocketColor','alpha layer')
        group.inputs.new('NodeSocketColor','red layer')
        group.inputs.new('NodeSocketColor','green layer')
        group.inputs.new('NodeSocketColor','blue layer')
        group.inputs.new('NodeSocketColor','base layer')
        group.inputs.new('NodeSocketColor','terrain_rgb')
        group.inputs.new('NodeSocketColor','terrain_a')
        
        # create group outputs
        group_outputs = group.nodes.new('NodeGroupOutput')
        group_outputs.location = (500,0)
        group.outputs.new('NodeSocketColor','color')   

        # create mix nodes
        mix_0 = group.nodes.new('ShaderNodeMixRGB')
        mix_0.location = (0,0)

        mix_1 = group.nodes.new('ShaderNodeMixRGB')
        mix_1.location = (100,200)

        mix_2 = group.nodes.new('ShaderNodeMixRGB')
        mix_2.location = (200,400)
        
        mix_3 = group.nodes.new('ShaderNodeMixRGB')
        mix_3.location = (300,600) 
        
        # create seperate nodes
        rgb = group.nodes.new('ShaderNodeSeparateRGB')
        rgb.location = (-300,500)
        
        # create seperate nodes
        a = group.nodes.new('ShaderNodeInvert')
        a.location = (-300,300)        

        # link inputs
        group.links.new(group_inputs.outputs['base layer'], mix_0.inputs['Color1'])
        
        group.links.new(group_inputs.outputs['blue layer'], mix_0.inputs['Color2'])
        group.links.new(group_inputs.outputs['green layer'], mix_1.inputs['Color2'])
        group.links.new(group_inputs.outputs['red layer'], mix_2.inputs['Color2'])
        group.links.new(group_inputs.outputs['alpha layer'], mix_3.inputs['Color2'])
        
        group.links.new(group_inputs.outputs['terrain_rgb'], rgb.inputs['Image'])
        group.links.new(a.inputs['Color'], group_inputs.outputs['terrain_a'])
        
        # mix input nodes
        group.links.new(mix_1.inputs['Color1'], mix_0.outputs['Color'])
        group.links.new(mix_2.inputs['Color1'], mix_1.outputs['Color'])
        group.links.new(mix_3.inputs['Color1'], mix_2.outputs['Color'])
        
        group.links.new(mix_0.inputs['Fac'], rgb.outputs['B'])
        group.links.new(mix_1.inputs['Fac'], rgb.outputs['G'])
        group.links.new(mix_2.inputs['Fac'], rgb.outputs['R'])
        group.links.new(mix_3.inputs['Fac'], a.outputs['Color'])
        
        #link output
        group.links.new(mix_3.outputs['Color'], group_outputs.inputs['color'])     

        return group

        

#------------------- hotkeys ------------------------------  
# Originally from rSelection script
addon_keymaps = []

def kmi_props_setattr(kmi_props, attr, value):
    try:
        setattr(kmi_props, attr, value)
    except AttributeError:
        print("Warning: property '%s' not found in keymap item '%s'" %
            (attr, kmi_props.__class__.__name__))
    except Exception as e:
        print("Warning: %r" % e)

class hotkeys(bpy.types.Operator):
    bl_label = "Activate hotkeys"
    bl_idname = "bitshift.activate_hotkeys"
        
    def execute(self, context):   
        print("Activating hotkeys")
        # Set Preferences
        bpy.context.user_preferences.inputs.select_mouse = 'LEFT'

        #----KEYMAP----
        wm = bpy.context.window_manager

        if True:
            view3d_km_items = wm.keyconfigs.default.keymaps['3D View'].keymap_items
            for j in view3d_km_items:
                if j.name == 'Activate/Select':
                    j.active = False
    
        #----EDIT MODE----
        
        km = wm.keyconfigs.addon.keymaps.new('Mesh', space_type='EMPTY', region_type='WINDOW', modal=False)
        addon_keymaps.append(km)

        #Element Select Modes
        kmi = km.keymap_items.new('mesh.select_mode', 'ONE', 'PRESS')
        kmi_props_setattr(kmi.properties, 'type', 'VERT')
        kmi = km.keymap_items.new('mesh.select_mode', 'TWO', 'PRESS')
        kmi_props_setattr(kmi.properties, 'type', 'EDGE')
        kmi = km.keymap_items.new('mesh.select_mode', 'THREE', 'PRESS')
        kmi_props_setattr(kmi.properties, 'type', 'FACE')

        #Select Linked
        kmi = km.keymap_items.new('mesh.select_linked_pick', 'SELECTMOUSE', 'DOUBLE_CLICK')
        kmi_props_setattr(kmi.properties, 'limit', True)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi = km.keymap_items.new('mesh.select_linked_pick', 'SELECTMOUSE', 'DOUBLE_CLICK', shift=True)
        kmi_props_setattr(kmi.properties, 'limit', True)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi = km.keymap_items.new('mesh.select_linked_pick', 'SELECTMOUSE', 'DOUBLE_CLICK', ctrl=True)
        kmi_props_setattr(kmi.properties, 'limit', True)
        kmi_props_setattr(kmi.properties, 'deselect', True)
        
        #----OBJECT MODE----

        km = bpy.context.window_manager.keyconfigs.addon.keymaps.new('3D View', space_type='VIEW_3D', region_type='WINDOW', modal=False)
        addon_keymaps.append(km)

        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS')
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', False)
        kmi_props_setattr(kmi.properties, 'center', False)
        kmi_props_setattr(kmi.properties, 'enumerate', False)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', shift=True)
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', True)
        kmi_props_setattr(kmi.properties, 'center', False)
        kmi_props_setattr(kmi.properties, 'enumerate', False)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', ctrl=True)
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', True)
        kmi_props_setattr(kmi.properties, 'toggle', False)
        kmi_props_setattr(kmi.properties, 'center', False)
        kmi_props_setattr(kmi.properties, 'enumerate', False)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', alt=True)
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', False)
        kmi_props_setattr(kmi.properties, 'center', False)
        kmi_props_setattr(kmi.properties, 'enumerate', True)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', shift=True, ctrl=True)
        kmi_props_setattr(kmi.properties, 'extend', True)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', True)
        kmi_props_setattr(kmi.properties, 'center', True)
        kmi_props_setattr(kmi.properties, 'enumerate', False)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', ctrl=True, alt=True)
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', False)
        kmi_props_setattr(kmi.properties, 'center', True)
        kmi_props_setattr(kmi.properties, 'enumerate', True)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', shift=True, alt=True)
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', True)
        kmi_props_setattr(kmi.properties, 'center', False)
        kmi_props_setattr(kmi.properties, 'enumerate', True)
        kmi_props_setattr(kmi.properties, 'object', False)
        
        kmi = km.keymap_items.new('view3d.select_or_deselect_all', 'SELECTMOUSE', 'PRESS', shift=True, ctrl=True, alt=True)
        kmi_props_setattr(kmi.properties, 'extend', False)
        kmi_props_setattr(kmi.properties, 'deselect', False)
        kmi_props_setattr(kmi.properties, 'toggle', True)
        kmi_props_setattr(kmi.properties, 'center', True)
        kmi_props_setattr(kmi.properties, 'enumerate', True)
        kmi_props_setattr(kmi.properties, 'object', False)

        kmi = km.keymap_items.new('view3d.select_border', 'EVT_TWEAK_L', 'ANY')
        kmi_props_setattr(kmi.properties, 'extend', False)

        kmi = km.keymap_items.new('view3d.select_border', 'EVT_TWEAK_L', 'ANY', shift=True)
        kmi = km.keymap_items.new('view3d.select_border', 'EVT_TWEAK_L', 'ANY', ctrl=True)
        kmi_props_setattr(kmi.properties, 'extend', False)

        kmi = km.keymap_items.new('view3d.manipulator', 'SELECTMOUSE', 'PRESS', any=True)
        kmi_props_setattr(kmi.properties, 'release_confirm', True)

        # Map Gesture Border
        km = bpy.context.window_manager.keyconfigs.addon.keymaps.new('Gesture Border', space_type='EMPTY', region_type='WINDOW', modal=True)
        addon_keymaps.append(km)

        kmi = km.keymap_items.new_modal('CANCEL', 'ESC', 'PRESS', any=True)
        kmi = km.keymap_items.new_modal('BEGIN', 'LEFTMOUSE', 'PRESS')
        kmi = km.keymap_items.new_modal('SELECT', 'LEFTMOUSE', 'RELEASE')
        kmi = km.keymap_items.new_modal('SELECT', 'LEFTMOUSE', 'RELEASE', shift=True)
        kmi = km.keymap_items.new_modal('DESELECT', 'LEFTMOUSE', 'RELEASE', ctrl=True)
        
        return {"FINISHED"}

#------------------- blender ------------------------------   
def register():

    bpy.utils.register_module(__name__)    
    pass

def unregister():
   
    bpy.utils.unregister_module(__name__)    
    pass    
    try:
        del bpy.types.WindowManager.retopowindowtool
    except:
        pass

if __name__ == "__main__":
    register()
