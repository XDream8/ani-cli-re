#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/* Curl Library */
#include <curl/curl.h>
/* Regex */
#include <regex.h>

#define BASE_URL "https://gogoanime.cm"

/* function declarations */
static void regex(char *content, char *pattern);
static size_t regex_animes(char *buffer, size_t itemsize, size_t nitems, void* ignorethis);
static void curl_urls(char *url, void *call_func, long int *verbosely);
static void search_anime(void);
static void search_eps(char *anime_id);

/* variables */
static char search[45];
static char search_url[128];
static char eps_url[sizeof(search_url)];
char result[1024];
char anime_id[30];
char call_func;
long int verbosely;


void
regex(char *content, char *pattern)
{
	size_t len;
	regex_t re;
	regmatch_t subs[50];
	char errbuf[128];
	int err, i;

	err = regcomp(&re, pattern, REG_EXTENDED);
	if(err)
	{
		len = regerror(err, &re, errbuf, sizeof(errbuf));
		printf("%s %s\n", "error: regcomp:", errbuf);
		exit(-1);
	}
	err = regexec(&re, content, (size_t)50, subs, 0);
	if(err = REG_NOMATCH)
	{
		printf("%s\n", "No results");
		regfree(&re);
		exit(0);
	}
	else if(err)
	{
		len = regerror(err, &re, errbuf, sizeof(errbuf));
		printf("%s %s\n", "error: regexec:", errbuf);
		exit(-1);
	}

	printf("%s\n", "matched");
	for(i = 0; i < re. re_nsub; i++);
	{
		len = subs[i]. rm_eo-subs[i]. rm_so;
		if(i = 0)
		{
			printf("begin: %ld, len = %ld\n", subs[i]. rm_so, len);
		}
		else
		{
			printf("subexpression %d begin: %ld, len = %ld", i, subs[i]. rm_so, len);
		}
		memcpy(result, content + subs[i]. rm_so, len);
		result[len] = '\0';
		printf("match: %s\n", result);
	}

}

/* function implementations */
size_t /* i couldnt find a good way to do this so i skipped this functionality. normally we will set anime_id here */
regex_animes(char *buffer, size_t itemsize, size_t nitems, void* ignorethis)
{
	size_t bytes = itemsize * nitems;

	char *str[sizeof(buffer)];

	strncpy((char *)str, buffer, sizeof(str));

	char pattern[] = "category";

	regex((char *)str, pattern);

	/* printf("%s\n",buffer); */

	/* printf("New chunk (%zu bytes)\n", bytes); */
	/* printf("%s\n", buffer); */
	return bytes;
}

void
curl_urls(char *url, void *call_func,long int *verbosely)
{
	/* Curl */
	CURL *curl = curl_easy_init();
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	/* check curl */
	if(!curl)
	{
		fprintf(stderr,"Failed to initialize curl!\n");
		exit(-1);
	}

	/* for debugging */
	curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosely);
	/* send all data to this function  */
  /* curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_func); */

	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	res = curl_easy_perform(curl);
	/* check */
	if(res != CURLE_OK)
	{
		fprintf(stderr,"Could not fetch %s. Error: %s\n", url, curl_easy_strerror(res));
		exit(-2);
	}

	/* we are done */
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

void
search_anime(void)
{
	/* Get user input */
	printf("%s\n","Enter anime name:");
	fgets(search, sizeof(search), stdin);


	int i;
	/* Replace spaces with "-" */
	for(i = 0; i < strlen(search); i++)
	{
		if(isspace(search[i]))
			search[i] = '-';
	}

	/* search_url */
	snprintf(search_url, sizeof(search_url), "%s/search.html?keyword=%s", BASE_URL, search);

	curl_urls(search_url, regex_animes, (long int *)1L); /* 1L means verbose is active, 0L means its off */
}

void
search_eps(char *anime_id)
{
	snprintf(eps_url, sizeof(eps_url), "%s/category/%s", BASE_URL, anime_id);

	curl_urls(eps_url, regex_animes, (long int *)1L); /* 1L means verbose is active, 0L means its off */
}

int
main()
{
	search_anime();
	/* strncpy(anime_id, "tokyo-ghoul", sizeof(anime_id)); */
	/* search_eps(anime_id); */
	return 0;
}

