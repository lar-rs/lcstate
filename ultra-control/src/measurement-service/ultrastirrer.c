
#include "ultrastirrer.h"
#include "mkt-can-manager-client.h"
#include "tera-control-client.h"

static NodesObject *node = NULL;
static guint COUNT = 1;
static guint DELAY_MOD1 = 50;
static guint DELAY_MOD2 = 50;
static guint CURRENT_MOD1 = 70;
static guint CURRENT_MOD2 = 200;

NodesObject*
ultra_stirrer_get_node(){
    if(node!= NULL)return node;
	node = NODES_OBJECT(g_dbus_object_manager_get_object(mkt_can_manager_client_nodes(),"/com/lar/nodes/Doppelmotor1"));
    return node;

}

static GSettings*
stirrer_settings(){
    static GSettings *settings = NULL;
    if(settings==NULL) {
        settings = g_settings_new("ultra.stirrers");
    }
    return settings;
}

static TeraStirrer*
ultra_stirrer_bus() {
    static TeraStirrer * stirrer = NULL;
    if(stirrer==NULL){
        stirrer = tera_stirrer_skeleton_new();
        GError *error = NULL;
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(stirrer), tera_service_dbus_connection(), TERA_CONTROL_PATH, &error);
        if(stirrer)g_object_unref(stirrer);


    }
}
static void
ultra_stirrer_set_delay ()
{
    NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3( nodes_stirrer_get_node());
	if(doppelmotor) {
		nodes_doppelmotor3_call_set_stepper1_delay(doppelmotor,delay,NULL,NULL,NULL);
		stirrers_simple_set_delay(stirrers_object_get_simple(STIRRERS_OBJECT(stirrer_object)),delay);
	}
}

static void
ultra_stirrer_set_current_mod1 ()
{
    g_return_if_fail(STIRRER);
	NodesDoppelmotor3 *doppelmotor = nodes_object_get_doppelmotor3(stirrer_object->priv->node);
	if(doppelmotor) {
		guint count COUNT=>0?COUNT:1;
		nodes_doppelmotor3_call_set_stepper1_current(doppelmotor,COUNT*CURRENT_MOD1,NULL,NULL,NULL);
	}
}

void
ultra_stirrer_init(){
    if(STIRRER == NULL) {
        g_object_unref(STIRRER);
    }
    TeraStirrer *stirrer = ultra_stirrer_bus();

}
