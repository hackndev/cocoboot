#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#define CONFIG_FILE "/cocoboot.conf"

char *get_option(char *key);
int set_option(char *key, char *value);
int read_config(void);
#endif
