/**
 * @author XiaKQ
 * @version 0.1
 * @brief ini operation
 */

//#include "system_port.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "dotini_parser.h"

typedef struct dotini_kvpair
{
	char key[64];
	char value[64];
	struct dotini_kvpair *next;
} dotini_kvpair_t;

typedef struct dotini_section
{
	char name[64];
	dotini_kvpair_t *kvpair;
	struct dotini_section *next;
} dotini_section_t;

dotini_section_t *g_section_head = NULL;

int  dotini_open(const char *file)
{
	int  lineno = 0;
	char *p = NULL;
	char *token = NULL;
	char buf[128];
	FILE *fp = NULL;
	dotini_section_t *current_section = NULL;
	dotini_kvpair_t *current_kvpair = NULL;

	if (file == NULL)
		return -1;

	fp = fopen(file, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "dotini_open()::fopen(\"%s\", ...) error: %s\n", file, strerror(errno));		
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		lineno++;
		p = buf;

		while (*p == ' ' || *p == '\t')
			p++;

		if (*p == '#' || *p == ';' || *p == '\r' || *p == '\n')
			continue;

		if (*p == '[')
		{
			token = strtok(p, " \t\n\r");
			if (token[strlen(token) - 1] != ']')
			{
				fprintf(stderr, "dotini_open() error: 缺少']', 文件%s, 第%d行\n", file, lineno);
				dotini_close();
				fclose(fp);
				return -1;
			}
			
			current_section = (dotini_section_t *)malloc(sizeof(dotini_section_t));
			if (current_section == NULL)
			{
				fprintf(stderr, "dotini_open()::malloc(%d) error: %s\n", sizeof(dotini_section_t), strerror(errno));
				dotini_close();
				fclose(fp);
				return -1;
			}

			memset(current_section, 0, sizeof(dotini_section_t));
			token[strlen(token) - 1] = '\0';
			token++;			
			strncpy(current_section->name, token, sizeof(current_section->name));

			current_section->next = g_section_head;
			g_section_head = current_section;

			token = strtok(NULL, " \t\n\r");
			if (token != NULL)
				fprintf(stderr, "dotini_open() warning: ']'后有多余的字符, 文件%s, 第%d行\n", file, lineno);
		}
		else
		{
			if (g_section_head == NULL)
			{
				fprintf(stderr, "dotini_open() error: 不合语法的开始, 缺少'[', 文件%s, 第%d行\n", file, lineno);
				dotini_close();
				fclose(fp);
				return -1;
			}			

			token = strchr(p, '=');
			if (token == NULL)
			{
				fprintf(stderr, "dotini_open() error: 此行缺少'=', 文件%s, 第%d行\n", file, lineno);
				dotini_close();
				fclose(fp);
				return -1;
			}

			if (*p == '=')
			{
				fprintf(stderr, "dotini_open() error: '='前缺少key字符串, 文件%s, 第%d行\n", file, lineno);
				dotini_close();
				fclose(fp);
				return -1;
			}

			token = strtok(p, " \t\r\n=");

			current_kvpair = (dotini_kvpair_t *)malloc(sizeof(dotini_kvpair_t));
			if (current_kvpair == NULL)
			{
				fprintf(stderr, "dotini_open()::malloc(%d) error: %s\n", sizeof(dotini_kvpair_t), strerror(errno));
				dotini_close();
				fclose(fp);
				return -1;
			}
			
			memset(current_kvpair, 0, sizeof(dotini_kvpair_t));
			strncpy(current_kvpair->key, token, sizeof(current_kvpair->key));

			current_kvpair->next = g_section_head->kvpair;
			g_section_head->kvpair = current_kvpair;

			//FIXME: value字符串中可能含有' '和'='
			token = strtok(NULL, " \t\r\n=");
			if (token == NULL)
				fprintf(stderr, "dotini_open() warning: '='后缺少value字符串, 文件%s, 第%d行\n", file, lineno);
			else
				strncpy(current_kvpair->value, token, sizeof(current_kvpair->key));
		}
	}

	fclose(fp);
	return 0;
}

void dotini_close()
{
	dotini_section_t *sc  = NULL;
	dotini_section_t *nsc = NULL;
	dotini_kvpair_t  *kv  = NULL;
	dotini_kvpair_t  *nkv = NULL;

	sc = g_section_head;
	while (sc != NULL)
	{
		kv = sc->kvpair;
		while (kv != NULL)
		{
			nkv = kv->next;
			free(kv);
			kv = nkv;
		}

		nsc = sc->next;
		free(sc);
		sc = nsc;
	}

	g_section_head = NULL;
	return;
}

char *dotini_get_string(const char *section, const char *key)
{
	dotini_section_t *sc = NULL;
	dotini_kvpair_t  *kv = NULL;

	if (section == NULL || key == NULL)
		return NULL;

	for (sc = g_section_head; sc != NULL; sc = sc->next)
	{
		if (strcmp(sc->name, section) == 0)
		{
			for (kv = sc->kvpair; kv != NULL; kv = kv->next)
			{
				if (strcmp(kv->key, key) == 0)
					return kv->value;
			}
		}		
	}

	return NULL;
}

int dotini_get_integer(const char *section, const char *key)
{
	char *s = dotini_get_string(section, key);

	if (s == NULL)
		return 0;

	return atoi(s);	
}

double dotini_get_double(const char *section, const char *key)
{
	char *s = dotini_get_string(section, key);

	if (s == NULL)
		return 0.0;

	return atof(s);	
}

