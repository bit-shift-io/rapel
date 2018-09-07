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
#include "register_types.h"
#include "itch_io.h"
#include "butil.h"
#include "binternet.h"
#include "bterrain.h"
#include "bwater.h"
#include "bclutter_map.h"
#include "bboundary_map.h"
#include "bnormal_map.h"
#include "bspline.h"
#include "bcurve_mesh.h"
#include "bcurve_mesh_node.h"
#include "barc_line_curve.h"
#include "barc_line_curve_node.h"
#include "bmaterial.h"
#include "bterrain_material.h"
#include "bdialogs.h"
#include "blog.h"
#include "bpanel.h"
#include "bpanel_box_container.h"
#include "bsurface_tool.h"
#include "bmaster_server.h"
#include "bconfig.h"
#include "brss.h"
#include "btheme.h"

#ifdef TOOLS_ENABLED
#include "editor/beditor.h"
#endif

#include "bplugin_manager.h"

static BConfig* _bconfig = NULL;
static BMasterServer* _bmaster_server = NULL;
static BUtil* _butil = NULL;
static BInternet* _binternet = NULL;
//static BTheme* _btheme = NULL;
static Ref<BTheme> _btheme;

#ifdef TOOLS_ENABLED
static BEditor* _beditor;
#endif

static BPluginManager* _bplugin_manager = NULL;

uint32_t start_allocs = 0;
uint32_t end_allocs = 0;

const int EDSCALE = 1;

static Ref<StyleBoxFlat> make_flat_stylebox(Color p_color, float p_margin_left = -1, float p_margin_top = -1, float p_margin_right = -1, float p_margin_bottom = -1) {
    Ref<StyleBoxFlat> style(memnew(StyleBoxFlat));
    style->set_bg_color(p_color);
    style->set_default_margin(MARGIN_LEFT, p_margin_left * EDSCALE);
    style->set_default_margin(MARGIN_RIGHT, p_margin_right * EDSCALE);
    style->set_default_margin(MARGIN_BOTTOM, p_margin_bottom * EDSCALE);
    style->set_default_margin(MARGIN_TOP, p_margin_top * EDSCALE);
    return style;
}

void register_bitshift_types() {
    start_allocs = MemoryPool::allocs_used;

    ClassDB::register_class<BLog>();
    ClassDB::register_class<BSurfaceTool>();
    //ObjectTypeDB::register_type<ItchIo>();
    //ObjectTypeDB::register_type<BUtil>();
    ClassDB::register_class<BTerrain>();
    ClassDB::register_class<BWater>();
    ClassDB::register_class<BClutterMap>();
    ClassDB::register_class<BBoundaryMap>();
    ClassDB::register_class<BNormalMap>();
    ClassDB::register_class<BSpline>();
    ClassDB::register_class<BCurveMesh>();
    ClassDB::register_class<BCurveMeshNode>();
    ClassDB::register_class<BArcLineCurve>();
    ClassDB::register_class<BWindowDialog>();
    ClassDB::register_class<BPanel>();
    ClassDB::register_class<HPanelBoxContainer>();
    ClassDB::register_class<VPanelBoxContainer>();
    //ClassDB::register_class<BArcLineCurveNode>();
    ClassDB::register_class<BSpatialMaterial>();
    ClassDB::register_class<BTerrainMaterial>();
    ClassDB::register_class<BRSS>();
	
    SceneTree::add_idle_callback(BSpatialMaterial::flush_changes);
    BSpatialMaterial::init_shaders();
    BTerrainMaterial::init_shaders();
	
    //_blog=memnew(BLog);
    _bconfig=memnew(BConfig);
    _bmaster_server=memnew(BMasterServer);
    _butil=memnew(BUtil);
    _binternet=memnew(BInternet);
    //_btheme=memnew(BTheme);
    _btheme.instance();

#ifdef TOOLS_ENABLED
    _beditor=memnew(BEditor);
#endif

    _bplugin_manager=memnew(BPluginManager);

    Engine::get_singleton()->add_singleton( Engine::Singleton("BInternet",BInternet::get_singleton() )  );
    Engine::get_singleton()->add_singleton( Engine::Singleton("BUtil",BUtil::get_singleton() )  );
    Engine::get_singleton()->add_singleton( Engine::Singleton("BConfig",BConfig::get_singleton() )  );
    Engine::get_singleton()->add_singleton( Engine::Singleton("BMasterServer",BMasterServer::get_singleton() )  );
    Engine::get_singleton()->add_singleton( Engine::Singleton("BTheme",BTheme::get_singleton() )  );
    Engine::get_singleton()->add_singleton( Engine::Singleton("BPluginManager",BPluginManager::get_singleton() )  );
    //Engine::get_singleton()->add_singleton( Engine::Singleton("BTerrain",BTerrain::get_singleton() )  );
    //Engine::get_singleton()->add_singleton( Engine::Singleton("BLog",BLog::get_singleton() )  );

    end_allocs = MemoryPool::allocs_used;
}

void unregister_bitshift_types() {

    //ERR_EXPLAINC("There are still MemoryPool allocs in use at exit!");
    uint32_t allocs = MemoryPool::allocs_used;
    ERR_FAIL_COND(allocs != end_allocs);

    memdelete(_bplugin_manager);
    _btheme->shutdown();
    _btheme = Ref<BTheme>();
    //memdelete(_beditor); - cleaned up by EditorNode
    memdelete(_bconfig);
    memdelete(_bmaster_server);
    memdelete(_butil);
    memdelete(_binternet);

    BTerrainMaterial::finish_shaders();
    BSpatialMaterial::finish_shaders();

    allocs = MemoryPool::allocs_used;
    ERR_FAIL_COND(allocs != start_allocs);
}
