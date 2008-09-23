#ifndef CPPIFCE_H
#define CPPIFCE_H

#ifdef __cplusplus
extern "C" {
#endif

int cpp_plugin_start(void);
void cpp_plugin_stop(void);
int cpp_plugin_enable(void);
void cpp_plugin_disable(void);
void cpp_plugin_recvmsg(int from, long msg, void* param);

#ifdef __cplusplus
}
#endif

#endif /* CPPIFCE_H */

