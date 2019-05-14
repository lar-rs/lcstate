/*
 *  ultra-can-api.c
 * Copyright (C) LAR
 *
 */

#include <fcntl.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "can4linux.h"
#include "ultra-can-api.h"


static  gint can_fd = 0;


static GRecMutex init_rmutex;
#define MUTEX_LOCK()   g_rec_mutex_lock   (&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock (&init_rmutex)

struct _NodeValue {
    guint    fd;
    guint    node;
    guint    index;
    guint    subindex;
    GError  *error;
    GValue   value;
    guint    toggle_byte;
    guint    len;
};

enum {
    NODE_INDEX_READ  = 1 << 0,
    NODE_INDEX_WRITE = 1 << 0,
};


// const char *logdmy_now() {
//     mktTime_t time = mktNow();
//     struct tm * timeinfo;
//     static char buf[20];
//     timeinfo = localtime(&time.tv_sec);
//     strftime(buf, sizeof(buf), "%Y-%m-%d", timeinfo);
//     return (buf);
// }
// static void logmessage(const gchar *message, canmsg_t *rxmsg) {
//     gchar *filename = g_strdup_printf("%s-can.log",logdmy_now());
//     gchar *path = g_build_path("/", g_get_home_dir(), filename, NULL);
//     FILE * f    = fopen(path, "a+");
//     if (f != NULL) {
//         fprintf(f, "%s,%lx,[%lx,%x,%x,%x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x],%s\n",mktTimeStr(NULL), rxmsg->id & 0x07f, rxmsg->id, rxmsg->length,
//             rxmsg->flags, rxmsg->cob, rxmsg->data[0], rxmsg->data[1], rxmsg->data[2], rxmsg->data[3], rxmsg->data[4], rxmsg->data[5], rxmsg->data[6], rxmsg->data[7],message);
//         fflush(f);
//         fclose(f);
//     }
//     g_free(filename);
//     g_free(path);
// }

#define CAN_PATH "/dev/can0"

static CanErrors merror = CAN_ERROR_NONE;


#define OPEN_ERROR do{merror= CAN_ERROR_OPEN}while(0)
static guint
can_open() {
    if(can_fd>0) return can_fd;
    can_fd = open (CAN_PATH, O_RDWR | O_NONBLOCK);
    if(can_fd<=0 ) {
        //TODO: can0 open.
        merror = CAN_ERROR_OPEN;
    }
    return can_fd;
}

static void
can_close() {
    if(can_fd>0)close(can_fd);
    can_fd=0;
}

guint can_read_uint (gint id,guint index,guint subindex ) {
    MUTEX_LOCK();
    can_open();

    can_close();
    MUTEX_UNLOCK();
    return 0;
}


gboolean can_check(){
    return merror == CAN_ERROR_NONE;
}



guint  analog2_get_in01(){
    guint ret = can_read_uint(2,6101,1);
    return ret;
}
guint analog2_get_in02(){
    return 0.0;
}
guint analog2_get_in03(){
    return 0.0;
}
guint analog2_get_in04(){
    return 0.0;
}
guint analog2_get_in05(){
    return 0.0;
}
guint analog2_get_out(){
    return 0.0;
}
void  analog2_set_out(gdouble value){

}
