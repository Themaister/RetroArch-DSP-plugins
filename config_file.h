
#ifndef __CONFIG_FILE_H
#define __CONFIG_FILE_H

#include <stdint.h>
#include <stdio.h>

typedef struct config_file config_file_t;

/////
// Config file format
// - # are treated as comments. Rest of the line is ignored.
// - Format is: key = value. There can be as many spaces as you like in-between.
// - Value can be wrapped inside "" for multiword strings. (foo = "hai u")

// Loads a config file. Returns NULL if file doesn't exist. NULL path will create an empty config file.
config_file_t *config_file_new(const char *path);
// Frees config file.
void config_file_free(config_file_t *conf);

// All extract functions return true when value is valid and exists. Returns false otherwise.

// Extracts a double from config file.
bool config_get_double(config_file_t *conf, const char *entry, double *in);
// Extracts an int from config file.
bool config_get_int(config_file_t *conf, const char *entry, int *in);
// Extracts a single char. If value consists of several chars, this is an error.
bool config_get_char(config_file_t *conf, const char *entry, char *in);
// Extracts an allocated string in *in. This must be free()-d if this function succeeds.
bool config_get_string(config_file_t *conf, const char *entry, char **in);
// Extracts a boolean from config. Valid boolean true are "true" and "1". Valid false are "false" and "0". Other values will be treated as an error.
bool config_get_bool(config_file_t *conf, const char *entry, bool *in);

// Setters. Similiar to the getters.
void config_set_double(config_file_t *conf, const char *entry, double value);
void config_set_int(config_file_t *conf, const char *entry, int val);
void config_set_char(config_file_t *conf, const char *entry, char val);
void config_set_string(config_file_t *conf, const char *entry, const char *val);
void config_set_bool(config_file_t *conf, const char *entry, bool val);

// Write the current config to a file.
bool config_file_write(config_file_t *conf, const char *path);

// Dump the current config to an already opened file. Does not close the file.
void config_file_dump(config_file_t *conf, FILE *file);



#endif
