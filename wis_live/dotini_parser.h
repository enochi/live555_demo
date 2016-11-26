/**
 * @author XiaKQ
 * @version 0.1
 * @brief ini operation
 */

#ifndef __DOTINI_PARSER_H__
#define __DOTINI_PARSER_H__

int  dotini_open(const char *file);
void dotini_close();

char   *dotini_get_string(const char *section, const char *key);
int    dotini_get_integer(const char *setcion, const char *key);
double dotini_get_double(const char *section, const char *key);

#endif  /* __DOTINI_PARSER_H__ */

