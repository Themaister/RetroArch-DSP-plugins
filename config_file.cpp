
#include "config_file.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

struct entry_list
{
   char *key;
   char *value;
   struct entry_list *next;
};

struct config_file
{
   struct entry_list *entries;
};

static char *getaline(FILE *file)
{
   char *newline = (char*)malloc(9);
   size_t cur_size = 8;
   size_t index = 0;

   int in = getc(file);
   while (in != EOF && in != '\n')
   {
      if (index == cur_size)
      {
         cur_size *= 2;
         newline = (char*)realloc(newline, cur_size + 1);
      }

      newline[index++] = in;
      in = getc(file);
   }
   newline[index] = '\0';
   return newline; 
}

static bool parse_line(struct entry_list *list, char *line)
{
   // Remove everything after comment.
   char *comment = strchr(line, '#');
   if (comment)
      *comment = '\0';

   // Skips to first character.
   while (isspace(*line))
      line++;

   char *key = (char*)malloc(9);
   size_t cur_size = 8;
   size_t index = 0;

   while (isgraph(*line))
   {
      if (index == cur_size)
      {
         cur_size *= 2;
         key = (char*)realloc(key, cur_size + 1);
      }

      key[index++] = *line++;
   }
   key[index] = '\0';
   list->key = key;

   while (isspace(*line))
      line++;

   // If we don't have an equal sign here, we've got an invalid string...
   if (*line != '=')
   {
      list->key = nullptr;
      free(key);
      return false;
   }
   line++;

   while (isspace(*line))
      line++;

   // We have a full string. Read until next ".
   if (*line == '"')
   {
      char *tok = strtok(line + 1, "\"");
      if (tok == nullptr)
      {
         list->key = nullptr;
         free(key);
         return false;
      }
      list->value = strdup(tok);
   }
   else // We don't have that... Read till next space.
   {
      char *tok = strtok(line, " \t\f");
      if (tok == nullptr)
      {
         list->key = nullptr;
         free(key);
         return false;
      }
      list->value = strdup(tok);
   }

   return true;
}

config_file_t *config_file_new(const char *path)
{

   struct config_file *conf = (struct config_file*)calloc(1, sizeof(*conf));
   if (conf == nullptr)
      return nullptr;

   if (path == nullptr)
      return conf;

   FILE *file = fopen(path, "r");
   if (!file)
   {
      free(conf);
      return nullptr;
   }

   struct entry_list *tail = conf->entries;

   while (!feof(file))
   {
      struct entry_list *list = (struct entry_list*)calloc(1, sizeof(*list));
      char *line = getaline(file);

      if (line)
      {
         if (parse_line(list, line))
         {
            if (conf->entries == nullptr)
            {
               conf->entries = list;
               tail = list;
            }
            else
            {
               tail->next = list;
               tail = list;
            }
         }
         free(line);
      }
   }
   fclose(file);

   return conf;
}

void config_file_free(config_file_t *conf)
{
   if (conf != nullptr)
   {
      struct entry_list *tmp = conf->entries;
      struct entry_list *old = tmp;
      while (tmp != nullptr)
      {
         free(tmp->key);
         free(tmp->value);
         old = tmp;
         tmp = tmp->next;
         free(old);
      }
      free(conf);
   }
}

bool config_get_double(config_file_t *conf, const char *key, double *in)
{
   struct entry_list *list = conf->entries;

   while (list != nullptr)
   {
      if (strcmp(key, list->key) == 0)
      {
         *in = strtod(list->value, nullptr);
         return true;
      }
      list = list->next;
   }
   return false;
}

bool config_get_int(config_file_t *conf, const char *key, int *in)
{
   struct entry_list *list = conf->entries;

   while (list != nullptr)
   {
      if (strcmp(key, list->key) == 0)
      {
         *in = strtol(list->value, nullptr, 0);
         return true;
      }
      list = list->next;
   }
   return false;
}

bool config_get_char(config_file_t *conf, const char *key, char *in)
{
   struct entry_list *list = conf->entries;

   while (list != nullptr)
   {
      if (strcmp(key, list->key) == 0)
      {
         if (strlen(list->value) > 1)
            return false;
         *in = *list->value;
         return true;
      }
      list = list->next;
   }
   return false;
}

bool config_get_string(config_file_t *conf, const char *key, char **str)
{
   struct entry_list *list = conf->entries;

   while (list != nullptr)
   {
      if (strcmp(key, list->key) == 0)
      {
         *str = strdup(list->value);
         return true;
      }
      list = list->next;
   }
   return false;
}

bool config_get_bool(config_file_t *conf, const char *key, bool *in)
{
   struct entry_list *list = conf->entries;

   while (list != nullptr)
   {
      if (strcmp(key, list->key) == 0)
      {
         if (strcasecmp(list->value, "true") == 0)
            *in = true;
         else if (strcasecmp(list->value, "1") == 0)
            *in = true;
         else if (strcasecmp(list->value, "false") == 0)
            *in = false;
         else if (strcasecmp(list->value, "0") == 0)
            *in = false;
         else
            return false;

         return true;
      }
      list = list->next;
   }
   return false;
}

void config_set_string(config_file_t *conf, const char *key, const char *val)
{
   struct entry_list *list = conf->entries;
   struct entry_list *last = list;
   while (list != nullptr)
   {
      if (strcmp(key, list->key) == 0)
      {
         free(list->value);
         list->value = strdup(val);
         return;
      }
      last = list;
      list = list->next;
   }

   struct entry_list *elem = (struct entry_list*)calloc(1, sizeof(*elem));
   elem->key = strdup(key);
   elem->value = strdup(val);

   if (last)
      last->next = elem;
   else
      conf->entries = elem;
}

void config_set_double(config_file_t *conf, const char *key, double val)
{
   char buf[128];
   snprintf(buf, sizeof(buf), "%f", (float)val);
   config_set_string(conf, key, buf);
}

void config_set_int(config_file_t *conf, const char *key, int val)
{
   char buf[128];
   snprintf(buf, sizeof(buf), "%d", val);
   config_set_string(conf, key, buf);
}

void config_set_char(config_file_t *conf, const char *key, char val)
{
   char buf[2];
   snprintf(buf, sizeof(buf), "%c", val);
   config_set_string(conf, key, buf);
}

void config_set_bool(config_file_t *conf, const char *key, bool val)
{
   config_set_string(conf, key, val ? "true" : "false");
}

bool config_file_write(config_file_t *conf, const char *path)
{
   FILE *file;

   if (path)
   {
      file = fopen(path, "w");
      if (!file)
         return false;
   }
   else
      file = stdout;

   config_file_dump(conf, file);

   if (path)
      fclose(file);

   return true;
}

void config_file_dump(config_file_t *conf, FILE *file)
{
   struct entry_list *list = conf->entries;

   while (list != nullptr)
   {
      fprintf(file, "%s = \"%s\"\n", list->key, list->value);
      list = list->next;
   }
}
